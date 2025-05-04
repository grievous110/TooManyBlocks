#ifndef TOOMANYBLOCKS_THREADPOOL_H
#define TOOMANYBLOCKS_THREADPOOL_H

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

    /**
     * @brief Main worker loop for the thread pool.
     *
     * Continuously waits for new jobs to be available or for termination.
     * Each job is executed in a try-catch block to prevent thread crashes on exceptions.
     * This function runs in each worker thread and exits when the termination flag is set.
     */
    void loop();

    /**
     * @brief Removes all tracking data associated with a given job owner.
     *
     * @param owner Pointer used to identify the owner of submitted jobs.
     */
    void erasePerOwnerEntrys(const void* owner);

public:
    /**
     * @brief Constructs the thread pool and launches worker threads.
     *
     * Initializes internal state and starts up to `numThreads` worker threads,
     * capped by the system's hardware concurrency limit.
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

    /**
     * @brief This function blocks the calling thread until all tasks in the pool have been completed.
     */
    void waitForCompletion();

    /**
     * @brief This function blocks the calling thread until all tasks submitted by the given owner have finished
     * processing.
     *
     * @param owner Pointer identifying the owner whose tasks to wait for.
     */
    void waitForOwnerCompletion(const void* owner);

    /**
     * @brief Adds a job to the thread pool for execution.
     *
     * @param owner Pointer identifying the owner of the job (nullptr is also a valid owner).
     * @param job The task to be executed by a worker thread.
     */
    void pushJob(const void* owner, std::function<void()> job);

    /**
     * @brief Cancels all jobs associated with a specific owner.
     *
     * @param owner Pointer identifying the owner of the jobs to cancel.
     */
    void cancelJobs(const void* owner);

    /**
     * @brief Forces cancellation of all jobs in the thread pool.
     */
    void forceCancelAllJobs();
};

#endif