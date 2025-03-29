#include "ThreadPool.h"
#include "Logger.h"
#include <stdexcept>

void ThreadPool::loop() {
    while (true) {
        Job job;
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            // Wait for a job arriving or terminate flag beeing set
            m_taskAvailableCvar.wait(lock, [this] { return !m_jobs.empty() || m_terminateFlag; });
            if (m_terminateFlag) return;

            // Get first job in queue
            job = std::move(m_jobs.front());
            m_jobs.pop_front();
        }
        // Execute job + Release lock
        
        try {
            job.task();
        } catch (const std::exception& e) {
            lgr::lout.error("Failed thread pool taks: " + std::string(e.what()));
        }
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (--m_ownerTaskCount[job.owner] == 0) {
                if (m_ownerWaitingThreads[job.owner] == 0) {
                    erasePerOwnerEntrys(job.owner);
                } else {
                    m_ownerWaitCvars[job.owner].notify_all(); // Notify any waiting threads for this owner
                }
            }

            if (--m_totalTaskCount == 0) {
                m_globalWaitCvar.notify_all();
            }
        }
    }
}

void ThreadPool::erasePerOwnerEntrys(const void* owner) {
    m_ownerTaskCount.erase(owner);
    m_ownerWaitCvars.erase(owner);
    m_ownerWaitingThreads.erase(owner);
}

ThreadPool::ThreadPool(size_t numThreads) : m_terminateFlag(false), m_totalTaskCount(0) {
    const unsigned int maxThreads = std::thread::hardware_concurrency();
    for (unsigned int i = 0; i < numThreads && i < maxThreads; i++) {
        m_threads.emplace_back(std::thread(&ThreadPool::loop, this));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_terminateFlag = true;
    }
    // Wake up all threads if after setting terminate flag
    m_taskAvailableCvar.notify_all();
    // Wait for all threads to finish and exit
    for (std::thread& activeThread : m_threads) {
        activeThread.join();
    }
    m_threads.clear();
}

void ThreadPool::waitForCompletion() {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_globalWaitCvar.wait(lock, [this] { return m_totalTaskCount == 0; });
}

void ThreadPool::waitForOwnerCompletion(const void* owner) {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_ownerWaitingThreads[owner]++;
    m_ownerWaitCvars[owner].wait(lock, [this, owner] { return m_ownerTaskCount[owner] == 0; });
    if (--m_ownerWaitingThreads[owner] == 0) {
        erasePerOwnerEntrys(owner); // Last waiting thread cleans up per owner entrys
    }
}

void ThreadPool::pushJob(const void* owner, std::function<void()> job) {
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_jobs.push_back({ owner, std::move(job) });
        m_ownerTaskCount[owner]++;
        m_totalTaskCount++;
    }
    // Notify one worker cause job is available
    m_taskAvailableCvar.notify_one();
}

void ThreadPool::cancelJobs(const void* owner) {
    std::lock_guard<std::mutex> lock(m_mtx);

    auto it = m_jobs.begin();
    while (it != m_jobs.end()) {
        if (it->owner == owner) {
            it = m_jobs.erase(it);
            m_totalTaskCount--;
            if (--m_ownerTaskCount[owner] == 0) {
                if (m_ownerWaitingThreads[owner] == 0) {
                    erasePerOwnerEntrys(owner);
                } else {
                    m_ownerWaitCvars[owner].notify_all();  // Notify any waiting threads for this owner
                }
            }
        } else {
            ++it;
        }
    }

    if (m_totalTaskCount == 0) {
        m_globalWaitCvar.notify_all();
    }
}

void ThreadPool::forceCancelAllJobs() {
    std::lock_guard<std::mutex> lock(m_mtx);
    for (const Job& job : m_jobs) {
        m_totalTaskCount--;
        if (--m_ownerTaskCount[job.owner] == 0) {
            if (m_ownerWaitingThreads[job.owner] == 0) {
                erasePerOwnerEntrys(job.owner);
            } else {
                m_ownerWaitCvars[job.owner].notify_all();  // Notify any waiting threads for this owner
            }
        }
    }

    m_jobs.clear();

    if (m_totalTaskCount == 0) {
        m_globalWaitCvar.notify_all();
    }
}