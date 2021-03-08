#pragma once

#include <Aka/Core/String.h>

#include <vector>
#include <deque>

namespace aka {

template <typename T>
struct EventListener
{
	EventListener() {}
	virtual ~EventListener() {}
	virtual void onSubscribe() {};
	virtual void onUnsubscribe() {};
	virtual void onReceive(const T& event) = 0;
};

template <typename T>
class EventDispatcher
{
public:
	// Emit an event and queue it
	static void emit(T&& event);
	// Emit an event and dispatch it immediately
	static void trigger(T&& event);

	// Dispatch all events to all listeners
	static void dispatch();

	// Clear all events from queue
	static void clear();

	// Subscribe a listener
	static void subscribe(EventListener<T>* listener);
	// Unsubscribe a specific listener
	static void unsubscribe(EventListener<T>* listener);
	// Unsubscribe all listeners from event
	static void unsubscribe();
private:
	static std::deque<T> m_events;
	static std::vector<EventListener<T>*> m_listeners;
};

template<typename T>
std::deque<T> EventDispatcher<T>::m_events;
template<typename T>
std::vector<EventListener<T>*> EventDispatcher<T>::m_listeners;

template <typename T>
inline void EventDispatcher<T>::emit(T&& event)
{
	m_events.push_back(std::move(event));
}
template <typename T>
inline void EventDispatcher<T>::trigger(T&& event)
{
	for (EventListener<T>* listener : m_listeners)
		listener->onReceive(event);
}
template <typename T>
inline void EventDispatcher<T>::dispatch()
{
	for (T& event : m_events)
		for (EventListener<T>* listener : m_listeners)
			listener->onReceive(event);
	m_events.clear();
}
template <typename T>
inline void EventDispatcher<T>::clear()
{
	m_events.clear();
}
template <typename T>
inline void EventDispatcher<T>::subscribe(EventListener<T>* listener) 
{
	m_listeners.push_back(listener);
	listener->onSubscribe();
}
template <typename T>
inline void EventDispatcher<T>::unsubscribe(EventListener<T>* listener)
{
	for (auto it = m_listeners.begin(); it != m_listeners.end(); it++)
	{
		if ((*it) == listener)
		{
			listener->onUnsubscribe();
			m_listeners.erase(it);
			break;
		}
	}
}
template <typename T>
inline void EventDispatcher<T>::unsubscribe()
{
	for (EventListener<T>* listener : m_listeners)
		listener->onUnsubscribe();
	m_listeners.clear();
}

};