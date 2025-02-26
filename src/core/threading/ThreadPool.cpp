#include "ThreadPool.h"

void ThreadPool::loop() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            // Wait for a job arriving or terminate flag beeing set
            m_cvar.wait(lock, [this] {
                return !m_jobs.empty() || m_terminateFlag;
                });
            if (m_terminateFlag) {
                return; // Exit if ThreadPool is terminating
            }
            // Get first job in queue
            job = m_jobs.front();
            m_jobs.pop();
        }
        // Execute job + Release lock
        job();
    }
}

ThreadPool::ThreadPool(size_t numThreads) : m_terminateFlag(false) {
    const unsigned int maxThreads = std::thread::hardware_concurrency();
    for (unsigned int i = 0; i < numThreads && i < maxThreads; i++) {
        m_threads.push_back(std::thread(&ThreadPool::loop, this));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_terminateFlag = true;
    }
    // Wake up all threads if after setting terminate flag
    m_cvar.notify_all();
    // Wait for all threads to finish and exit
    for (std::thread& activeThread : m_threads) {
        activeThread.join();
    }
    m_threads.clear();
}

void ThreadPool::pushJob(std::function<void()> job) {
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_jobs.push(job);
    }
    m_cvar.notify_one();
}
