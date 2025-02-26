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
	bool m_terminateFlag;
	std::mutex m_mtx;
	std::condition_variable m_cvar;
	std::vector<std::thread> m_threads;
	std::queue<std::function<void()>> m_jobs;

	void loop();

public:
	ThreadPool(size_t numThreads);
	~ThreadPool();

	inline size_t threadCount() const { return m_threads.size(); };

	void pushJob(std::function<void()> job);
};

#endif