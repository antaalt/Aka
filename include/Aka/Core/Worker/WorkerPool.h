#pragma once

#include <queue>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <list>
#include <functional>

#include <Aka/Core/Container/Vector.h>

namespace aka {

class Job;

class WorkerPool
{
public:
	WorkerPool();
	WorkerPool(size_t size);
	WorkerPool(const WorkerPool&) = delete;
	WorkerPool& operator=(const WorkerPool&) = delete;
	~WorkerPool();

	// Start all the workers
	void start();
	// Signal all the workers to stop
	void stop();
	// Wait for all the workers to stop
	void wait();
	// Kill the workers
	void kill();
	// Reset the queue
	void reset();
	// Is the worker active
	bool isActive() const;
	// Queue size
	size_t size() const;

	// Add a job to a worker
	void add(Job* job);

private:
	// Loop and execute a thread
	void loop();

private:
	mutable std::mutex m_mutex;
	std::condition_variable m_condition;
	std::list<Job*> m_jobs;
	std::vector<std::thread> m_workers;
	bool m_running;
};

};