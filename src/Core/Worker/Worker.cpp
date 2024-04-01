#include <Aka/Core/Worker/Worker.h>

#include <Aka/Core/Worker/Job.h>

namespace aka {

Worker::Worker() :
	m_running(false)
{
	start();
}

Worker::~Worker()
{
	stop();
	wait();
}

void Worker::start()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_running)
		return;
	m_running = true;
	lock.unlock();
	m_thread = std::thread(&Worker::loop, this);
}

void Worker::stop()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (!m_running)
		return;
	m_running = false;
	m_jobs.clear();
	lock.unlock();
	m_condition.notify_one();
}

void Worker::wait()
{
	if (m_thread.joinable())
		m_thread.join();
}

void Worker::kill()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (!m_running)
		return;
	m_running = false;
	m_thread.~thread();
}

void Worker::reset()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_jobs.clear();
}

bool Worker::isActive() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_running;
}

size_t Worker::size() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_jobs.size();
}

void Worker::add(Job* job)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_jobs.push_back(job);
	lock.unlock();
	m_condition.notify_all();
}

void Worker::loop()
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
		}
	}
}

};