#pragma once

#include <thread>
#include <future>
#include <queue>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>

using std::function;
using std::thread;
using std::vector;
using std::future;
using std::queue;
using std::mutex;
using std::condition_variable;
using std::unique_lock;
using std::make_shared;
using std::forward;
using std::bind;
using std::packaged_task;

class ThreadPool{
public:
	virtual ~ThreadPool() = default;
	ThreadPool(const int& thread_maxnums);

	template<typename Func, typename ...Args>
	std::future<std::invoke_result_t<Func, Args...>> join(Func&& func, Args&&... args);
private:
	bool m_exit_flag = false;
	vector<thread> m_workers;
	queue<function<void()>> m_tasks;
	mutex m_queue_mutex;
	condition_variable m_queue_condition;
};

template<typename Func, typename ...Args>
std::future<std::invoke_result_t<Func, Args...>> ThreadPool::join(Func&& func, Args&&... args)
{
	using result_type = std::invoke_result_t<Func, Args...>;
	auto task = make_shared<packaged_task<result_type()>>(bind(forward<Func>(func), forward<Args>(args)...));
	future<result_type> res = task->get_future();

	{
		unique_lock<mutex> lock(m_queue_mutex);

		if (m_exit_flag)
			throw std::runtime_error("can't join the task after the pool closed");

		m_tasks.emplace([task]() { (*task)(); });
	}

	m_queue_condition.notify_one();
	return res;
}

inline ThreadPool::ThreadPool(const int& thread_maxnums){
	for (int i = 0; i < thread_maxnums; i++)
	{
		m_workers.emplace_back([this]() {
			while(true){
				function<void()> task;
				{
					unique_lock<mutex> lock(this->m_queue_mutex);
					this->m_queue_condition.wait(
						lock, 
						[this]() { return this->m_exit_flag || !this->m_tasks.empty(); }
					);

					if (this->m_exit_flag || this->m_tasks.empty()) 
						return;

					task = std::move(this->m_tasks.front());
					m_tasks.pop();
				}
				task();
			}
		});
	}
}