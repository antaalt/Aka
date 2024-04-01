#pragma once

#include <mutex>
#include <functional>

#include <Aka/Core/Container/Vector.h>

namespace aka {

enum class JobStatus
{
	Waiting,
	Running,
	Finished
};

class Job
{
public:
	Job();
	virtual ~Job();
	// Check if the job is running
	JobStatus status() const;
	// Reset the job status only if finished
	void reset();
	// Execute the job and setup running flag
	void operator()();
protected:
	// Execute the job
	virtual void execute() = 0;
private:
	mutable std::mutex m_mutex; // Mutex for status
	JobStatus m_status;
};

class LambdaJob : public Job
{
public:
	LambdaJob(std::function<void(void)>&& lambda);
protected:
	void execute() override;
private:
	std::function<void(void)> m_job;
};


};