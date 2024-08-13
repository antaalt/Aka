#include <Aka/Core/Worker/WorkerPool.h>

#include <Aka/Core/Worker/Job.h>

#include <thread>
#include <functional>

namespace aka {

WorkerPool::WorkerPool() :
	WorkerPool(std::thread::hardware_concurrency())
{
}
WorkerPool::WorkerPool(size_t size) :
	m_running(false)
{
	m_workers.resize(size);
	start();
}

WorkerPool::~WorkerPool()
{
	stop();
	wait();
}

void WorkerPool::start()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_running)
		return;
	m_running = true;
	lock.unlock();
	for (auto& worker : m_workers)
		worker = std::thread(&WorkerPool::loop, this);
}

void WorkerPool::stop()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (!m_running)
		return;
	m_running = false;
	lock.unlock();
	m_jobs.clear();
	m_condition.notify_all();
}

void WorkerPool::wait()
{
	for (auto& worker : m_workers)
		if (worker.joinable())
			worker.join();
}

void WorkerPool::kill()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (!m_running)
		return;
	m_running = false;
	for (auto& worker : m_workers)
		worker.~thread();
}


void WorkerPool::reset()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_jobs.clear();
}

bool WorkerPool::isActive() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_running;
}

size_t WorkerPool::size() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_jobs.size();
}

void WorkerPool::add(Job* job)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_jobs.push_back(job);
	lock.unlock();
	m_condition.notify_all();
}

void WorkerPool::loop()
{
	while (true)
	{
		std::list<Job*> localQueue;
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			while (m_jobs.empty() && m_running)
				m_condition.wait(lock);

			if (!m_running)
			{
				// Stop if not running
				for (auto& job : m_jobs)
				{
					(*job)();
					mem::akaDelete(job);
				}
				m_jobs.clear();
				return;
			}
			localQueue = std::move(m_jobs);
			m_jobs.clear();
		}
		// Free the queue.
		for (auto& job : localQueue)
		{
			(*job)();
			mem::akaDelete(job);
		}
	}
}

};