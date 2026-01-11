#include "SystemMetrics.h"

#ifdef _WIN32
#include <psapi.h>
#include <windows.h>

MemoryInfo getSystemMemoryInfo() {
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);

    GlobalMemoryStatusEx(&status);

    MemoryInfo info;
    info.totalBytes = static_cast<uint64_t>(status.ullTotalPhys);
    info.bytesInUse = static_cast<uint64_t>(status.ullTotalPhys - status.ullAvailPhys);

    return info;
}

uint64_t getProcessUsedBytes() {
    PROCESS_MEMORY_COUNTERS pmc{};
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return 0;
    }

    return static_cast<uint64_t>(pmc.WorkingSetSize);
}

ProcessIO getProcessIO() {
    IO_COUNTERS counters{};
    GetProcessIoCounters(GetCurrentProcess(), &counters);

    ProcessIO io;
    io.bytesRead = counters.ReadTransferCount;
    io.bytesWritten = counters.WriteTransferCount;
    io.readOps = counters.ReadOperationCount;
    io.writeOps = counters.WriteOperationCount;

    return io;
}

CpuTimes getCpuTimes() {
    FILETIME create, exit, kernel, user;
    GetProcessTimes(GetCurrentProcess(), &create, &exit, &kernel, &user);

    const uint64_t user100ns = (uint64_t(user.dwHighDateTime) << 32) | user.dwLowDateTime;
    const uint64_t kernel100ns = (uint64_t(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime;

    CpuTimes times;
    times.userNs = kernel100ns * 100;
    times.kernelNs = kernel100ns * 100;
    return times;
}

uint32_t getLogicalCoreCount() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors > 0 ? info.dwNumberOfProcessors : 1;
}

#else
#include <unistd.h>

#include <fstream>
#include <string>
#include <unordered_map>

MemoryInfo getSystemMemoryInfo() {
    std::ifstream file("/proc/meminfo");
    std::unordered_map<std::string, uint64_t> mem;

    std::string key;
    uint64_t value;
    std::string unit;
    while (file >> key >> value >> unit) {
        mem[key] = value;
    }

    MemoryInfo info;
    info.totalBytes = mem["MemTotal:"] * 1000;
    info.bytesInUse = info.totalBytes - (mem["MemAvailable:"] * 1000);
    return info;
}

uint64_t getProcessUsedBytes() {
    std::ifstream file("/proc/self/statm");

    uint64_t pagesVirtual = 0;   // Theoretically available
    uint64_t pagesResident = 0;  // Actually used memory of process
    file >> pagesVirtual >> pagesResident;

    const long pageSize = sysconf(_SC_PAGESIZE);
    return pagesResident * pageSize;
}

ProcessIO getProcessIO() {
    std::ifstream file("/proc/self/io");

    ProcessIO io{};
    std::string key;
    uint64_t value;

    while (file >> key >> value) {
        if (key == "read_bytes:") {
            io.bytesRead = value;
        } else if (key == "write_bytes:") {
            io.bytesWritten = value;
        } else if (key == "syscr:") {
            io.readCalls = value;
        } else if (key == "syscw:") {
            io.writeCalls = value;
        }
    }

    return io;
}

CpuTimes getCpuTimes() {
    std::ifstream file("/proc/self/stat");
    std::string ignore;

    // skip first 13 fields
    for (int i = 0; i < 13; i++) file >> ignore;

    uint64_t userTicks = 0;
    uint64_t kernelTicks = 0;
    file >> userTicks >> kernelTicks;

    const long ticksPerSec = sysconf(_SC_CLK_TCK);

    CpuTimes times;
    times.userNs = (userTicks * 1'000'000'000ULL) / ticksPerSec;
    times.kernelNs = (kernelTicks * 1'000'000'000ULL) / ticksPerSec;
    return times;
}

uint32_t getLogicalCoreCount() {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return (n > 0) ? static_cast<uint32_t>(n) : 1;
}

#endif

float getCpuUsage(CpuTimes prev, CpuTimes curr, float deltaTimeSeconds) {
    if (deltaTimeSeconds <= 0.0f) return 0.0f;

    uint64_t cpuNs = (curr.userNs - prev.userNs) + (curr.kernelNs - prev.kernelNs);
    uint32_t coreCount = getLogicalCoreCount();

    float cpuTimeSeconds = cpuNs * 1e-9f;

    return cpuTimeSeconds / (deltaTimeSeconds * coreCount);
}