#pragma once

#include <queue>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <list>

#include <Aka/Core/Container/Vector.h>

namespace aka {

class Job;

class Worker
{
public:
	Worker();
	~Worker();

	// Start the worker
	void start();
	// Signal the worker to stop
	void stop();
	// Wait for the worker to stop
	void wait();
	// Kill the worker
	void kill();
	// Reset the queue
	void reset();
	// Is the worker active
	bool isActive() const;
	// Queue size
	size_t size() const;

	// Add a job to the worker
	void add(Job* job);

private:
	// Loop and execute the thread
	void loop();

private:
	mutable std::mutex m_mutex;
	std::condition_variable m_condition;
	std::list<Job*> m_jobs;
	std::thread m_thread;
	bool m_running;
};

};