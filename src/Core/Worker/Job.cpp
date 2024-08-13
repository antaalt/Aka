#include <Aka/Core/Worker/Job.h>

namespace aka {

TrackedJob::TrackedJob() :
	m_status(JobStatus::Waiting)
{
}

TrackedJob::~TrackedJob()
{
}
JobStatus TrackedJob::status() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_status;
}

void TrackedJob::reset()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_status != JobStatus::Finished)
		return;
	m_status = JobStatus::Waiting;
}

void TrackedJob::operator()()
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