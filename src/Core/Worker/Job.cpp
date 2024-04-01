#include <Aka/Core/Worker/Job.h>

namespace aka {

Job::Job() :
	m_status(JobStatus::Waiting)
{
}

Job::~Job()
{
}
JobStatus Job::status() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_status;
}

void Job::reset()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_status != JobStatus::Finished)
		return;
	m_status = JobStatus::Waiting;
}

void Job::operator()()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_status = JobStatus::Running;
	lock.unlock();
	execute();
	lock.lock();
	m_status = JobStatus::Finished;
}

LambdaJob::LambdaJob(std::function<void(void)>&& lambda) :
	m_job(std::move(lambda))
{
}

void LambdaJob::execute()
{
	m_job();
}

};