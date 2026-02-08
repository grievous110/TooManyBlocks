#ifndef TOOMANYBLOCKS_THREADPOOL_H
#define TOOMANYBLOCKS_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#include "Future.h"

class ThreadPool {
private:
    struct ExecutionSlotStatus {
        uint64_t completionCount;
        bool isRunning;
    };

    std::atomic<bool> m_terminateFlag;
    std::vector<std::thread> m_threads;

    std::mutex m_schedulingMtx;
    uint64_t m_taskContextGen;
    std::unordered_set<uint64_t> m_activeContextSet;
    std::vector<ExecutionSlotStatus> m_executionPointStatuses;

    std::condition_variable m_watingForActiveTaskCVar;

    std::mutex m_workerJobsMtx;
    std::deque<std::unique_ptr<FutureBase>> m_workerJobs;
    std::condition_variable m_workerTaskAvailableCVar;

    std::mutex m_mainThreadJobsMtx;
    std::deque<std::unique_ptr<FutureBase>> m_mainThreadJobs;

    // Keep futures alive and only clear them on the main thread
    // to be save to not clear render api ressources on the wrong thread
    std::mutex m_finishedJobsMtx;
    std::vector<std::unique_ptr<FutureBase>> m_finishedJobs;

    /**
     * @brief Main worker loop for the thread pool.
     *
     * Continuously waits for new jobs to be available or for termination.
     * Each job is executed in a try-catch block to prevent thread crashes on exceptions.
     * This function runs in each worker thread and exits when the termination flag is set.
     */
    void loop(unsigned int workerIndex);

    bool isContextActive(uint64_t taskContext) const;

    bool hasExecutionPassed(const std::vector<ExecutionSlotStatus>& a, const std::vector<ExecutionSlotStatus>& b) const;

public:
    /**
     * @brief Constructs the thread pool and launches worker threads.
     *
     * @param numThreads The desired number of worker threads to spawn.
     */
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    /**
     * @brief Returns the number of worker threads in the pool.
     *
     * @return The number of threads currently managed by the thread pool.
     */
    inline size_t threadCount() const { return m_threads.size(); };

    uint64_t getNewTaskContext();

    void destroyTaskContext(uint64_t taskContext);

    void waitForCurrentActiveTasks();

    /**
     * @brief Adds a job to the thread pool for execution.
     *
     * @param owner Pointer identifying the owner of the job (nullptr is also a valid owner).
     * @param future The task to be executed by a worker thread.
     */
    void pushJob(std::unique_ptr<FutureBase> future, Executor executor);

    void processMainThreadJobs();

    void cleanupFinishedJobs();
};

#endif
