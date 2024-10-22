#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
private:
	bool terminate;
	std::mutex mtx;
	std::condition_variable cond_var;
	std::vector<std::thread> threads;
	std::queue<std::function<void()>> jobs;

	void loop();

public:
	ThreadPool(size_t numThreads);
	~ThreadPool();

	size_t threadCount() const;

	void pushJob(std::function<void()> job);
};

#endif