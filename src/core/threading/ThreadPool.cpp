#include "ThreadPool.h"

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
        job.task();  // This will never throw since errors are captured by the internal future object

        {
            std::lock_guard<std::mutex> lock(m_mtx);
            JobTrackingState& state = m_ownerTackingState[job.owner];
            if (--state.taskCount == 0) {
                if (state.waitingThreadCount == 0) {
                    eraseOwnerJobTrackingEntry(job.owner);
                } else {
                    state.waitCvar.notify_all();  // Notify any waiting threads for this owner
                }
            }

            if (--m_totalTaskCount == 0) {
                m_globalWaitCvar.notify_all();
            }
        }
    }
}

void ThreadPool::eraseOwnerJobTrackingEntry(const void* owner) { m_ownerTackingState.erase(owner); }

ThreadPool::ThreadPool(size_t numThreads) : m_terminateFlag(false), m_totalTaskCount(0) {
    size_t maxThreads = std::min<size_t>(std::thread::hardware_concurrency(), numThreads);
    m_threads.reserve(maxThreads);
    for (size_t i = 0; i < maxThreads; i++) {
        m_threads.emplace_back(&ThreadPool::loop, this);
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
}

void ThreadPool::waitForCompletion() {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_globalWaitCvar.wait(lock, [this] { return m_totalTaskCount == 0; });
}

void ThreadPool::waitForOwnerCompletion(const void* owner) {
    std::unique_lock<std::mutex> lock(m_mtx);
    JobTrackingState& state = m_ownerTackingState[owner];
    state.waitingThreadCount++;
    state.waitCvar.wait(lock, [this, &state] { return state.taskCount == 0; });
    if (--state.waitingThreadCount == 0) {
        eraseOwnerJobTrackingEntry(owner);  // Last waiting thread cleans up per owner entrys
    }
}

void ThreadPool::cancelJobs(const void* owner) {
    std::lock_guard<std::mutex> lock(m_mtx);

    auto it = m_jobs.begin();
    while (it != m_jobs.end()) {
        if (it->owner == owner) {
            JobTrackingState& state = m_ownerTackingState[it->owner];
            it = m_jobs.erase(it);
            m_totalTaskCount--;
            if (--state.taskCount == 0) {
                if (state.waitingThreadCount == 0) {
                    eraseOwnerJobTrackingEntry(owner);
                } else {
                    state.waitCvar.notify_all();  // Notify any waiting threads for this owner
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
        JobTrackingState& state = m_ownerTackingState[job.owner];
        if (--state.taskCount == 0) {
            if (state.waitingThreadCount == 0) {
                eraseOwnerJobTrackingEntry(job.owner);
            } else {
                state.waitCvar.notify_all();  // Notify any waiting threads for this owner
            }
        }
    }

    m_jobs.clear();

    if (m_totalTaskCount == 0) {
        m_globalWaitCvar.notify_all();
    }
}