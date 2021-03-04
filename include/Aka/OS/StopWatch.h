#pragma once

#include <chrono>

namespace aka {

template <
	typename TimeT = std::chrono::milliseconds,
	typename ClockT = std::chrono::high_resolution_clock
>
class StopWatch
{
public:
	StopWatch();

	void start();

	typename TimeT::rep elapsed() const;
private:
	std::chrono::time_point<ClockT> m_start;
};

template <typename TimeT, typename ClockT>
inline StopWatch<TimeT, ClockT>::StopWatch() : m_start(ClockT::now())
{
}
template <typename TimeT, typename ClockT>
inline void StopWatch<TimeT, ClockT>::start()
{
	m_start = ClockT::now();
}
template <typename TimeT, typename ClockT>
inline typename TimeT::rep StopWatch<TimeT, ClockT>::elapsed() const
{
	using namespace std::chrono;
	TimeT delta = duration_cast<TimeT>(ClockT::now() - m_start);
	return delta.count();
}

}

