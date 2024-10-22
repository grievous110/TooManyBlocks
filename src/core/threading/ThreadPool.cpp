#include "ThreadPool.h"

void ThreadPool::loop() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cond_var.wait(lock, [this] {
                return !jobs.empty() || terminate;
                });
            if (terminate) {
                return;
            }
            job = jobs.front();
            jobs.pop();
        }
        job();
    }
}

ThreadPool::ThreadPool(size_t numThreads) : terminate(false) {
    const unsigned int maxThreads = std::thread::hardware_concurrency();
    for (unsigned int i = 0; i < numThreads && i < maxThreads; i++) {
        threads.push_back(std::thread(&ThreadPool::loop, this));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        terminate = true;
    }
    cond_var.notify_all();
    for (std::thread& activeThread : threads) {
        activeThread.join();
    }
    threads.clear();
}

size_t ThreadPool::threadCount() const {
    return threads.size();
}

void ThreadPool::pushJob(std::function<void()> job) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        jobs.push(job);
    }
    cond_var.notify_one();
}
