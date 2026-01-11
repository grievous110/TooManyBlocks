#ifndef TOOMANYBLOCKS_SYSTEMMETRICS_H
#define TOOMANYBLOCKS_SYSTEMMETRICS_H

#include <cstdint>

struct MemoryInfo {
    uint64_t totalBytes;
    uint64_t bytesInUse;
};

struct ProcessIO {
    uint64_t bytesRead;
    uint64_t bytesWritten;
    uint64_t readCalls;
    uint64_t writeCalls;
};

struct CpuTimes {
    uint64_t userNs;
    uint64_t kernelNs;
};

MemoryInfo getSystemMemoryInfo();

uint64_t getProcessUsedBytes();

ProcessIO getProcessIO();

CpuTimes getCpuTimes();

uint32_t getLogicalCoreCount();

float getCpuUsage(CpuTimes prev, CpuTimes curr, float deltaTime);

#endif