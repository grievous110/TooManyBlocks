#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

class ThreadPool {
private:
    struct Job {
        const void* owner;
        std::function<void()> task;
    };

    bool m_terminateFlag;
    std::mutex m_mtx;
    std::condition_variable m_taskAvailableCvar;
    std::condition_variable m_globalWaitCvar;
    std::vector<std::thread> m_threads;
    std::deque<Job> m_jobs;
    std::unordered_map<const void*, size_t> m_ownerTaskCount;
    std::unordered_map<const void*, std::condition_variable> m_ownerWaitCvars;
    std::unordered_map<const void*, size_t> m_ownerWaitingThreads;
    size_t m_totalTaskCount;

    void loop();

    void erasePerOwnerEntrys(const void* owner);

public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    inline size_t threadCount() const { return m_threads.size(); };

    void waitForCompletion();

    void waitForOwnerCompletion(const void* owner);

    void pushJob(const void* owner, std::function<void()> job);

    void cancelJobs(const void* owner);

    void forceCancelAllJobs();
};

#endif