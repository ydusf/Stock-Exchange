#pragma once

#include <thread>
#include <mutex>
#include <future>
#include <memory>
#include <functional>
#include <queue>
#include <condition_variable>
#include <vector>
#include <atomic>

class ThreadPool
{
private:
	std::mutex m_lock;
	std::condition_variable m_cv;

	std::vector<std::thread> m_threads;
	std::atomic<bool> m_stopRequested = false;

	std::queue<std::move_only_function<void()>> m_tasks;

public:
	int m_busyThreads = 0;

private:
	class ThreadWorker
	{
	private:
		ThreadPool* m_threadPool;


	public:
		ThreadWorker(ThreadPool* pool) : m_threadPool(pool)
		{
		}

		void operator()()
		{
			std::unique_lock<std::mutex> lock(m_threadPool->m_lock);
			while (!m_threadPool->m_stopRequested || (m_threadPool->m_stopRequested && !m_threadPool->m_tasks.empty()))
			{
				m_threadPool->m_busyThreads--;
				m_threadPool->m_cv.wait(lock, [this] 
				{
					return this->m_threadPool->m_stopRequested || !this->m_threadPool->m_tasks.empty();
				});

				m_threadPool->m_busyThreads++;

				if (!this->m_threadPool->m_tasks.empty())
				{
					auto func = std::move(m_threadPool->m_tasks.front());
					m_threadPool->m_tasks.pop();

					lock.unlock();
					func();
					lock.lock();

				}
			};
		}

		~ThreadWorker()
		{}
	};

public:
	explicit ThreadPool(std::size_t threadCount = std::thread::hardware_concurrency())
	{
		for (std::size_t i = 0; i < threadCount; ++i)
		{
			m_threads.emplace_back([this]
			{
				while (true)
				{
					std::move_only_function<void()> task;
					{
						std::unique_lock<std::mutex> lock(m_lock);
						m_cv.wait(lock, [this]
						{
							return m_stopRequested || !m_tasks.empty();
						});

						if (m_stopRequested && m_tasks.empty())
							return;

						m_busyThreads++;
						task = std::move(m_tasks.front());
						m_tasks.pop();
					}

					task();
					m_busyThreads--;
				}
			});
		}
	}

	template <typename F, typename... Args>
	auto AddTask(F&& f, Args&&... args) -> std::future<decltype(f(std::forward<Args>(args)...))>
	{
		using ReturnType = decltype(f(std::forward<Args>(args)...));

		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			[func = std::forward<F>(f), ... largs = std::forward<Args>(args)]() mutable {
				return func(std::forward<Args>(largs)...);
			}
		);

		auto future = task->get_future();

		{
			std::lock_guard<std::mutex> lock(m_lock);
			m_tasks.emplace([task]() { (*task)(); });
			m_cv.notify_one();
		}

		return future;
	}

	std::size_t QueueSize()
	{
		std::unique_lock<std::mutex> lock(m_lock);
		return m_tasks.size();
	}

	~ThreadPool()
	{
		{
			std::lock_guard<std::mutex> lock(m_lock);
			m_stopRequested = true;
		}

		m_cv.notify_all();

		for (std::thread& t : m_threads)
		{
			if (t.joinable()) t.join();
		}
	}
};