#pragma once

#include <Aka/Core/Container/String.h>

#include <vector>
#include <set>

namespace aka {

template <typename T>
struct EventListener
{
	EventListener();
	virtual ~EventListener();
	virtual void onReceive(const T& event) = 0;
};

template <typename T>
class EventDispatcher
{
public:
	// Emit an event and queue it
	template <typename... Args>
	static void emit(Args&&... args);
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
	static Vector<T> m_events;
	static std::set<EventListener<T>*> m_listeners;
};

template<typename T>
Vector<T> EventDispatcher<T>::m_events;
template<typename T>
std::set<EventListener<T>*> EventDispatcher<T>::m_listeners;

template<typename T>
inline EventListener<T>::EventListener()
{
	EventDispatcher<T>::subscribe(this);
}
template<typename T>
inline EventListener<T>::~EventListener()
{
	EventDispatcher<T>::unsubscribe(this);
}
template <typename T>
template <typename... Args>
inline void EventDispatcher<T>::emit(Args&&... args)
{
	m_events.append(std::forward<Args>(args)...);
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
	m_listeners.insert(listener);
}
template <typename T>
inline void EventDispatcher<T>::unsubscribe(EventListener<T>* listener)
{
	m_listeners.erase(listener);
}
template <typename T>
inline void EventDispatcher<T>::unsubscribe()
{
	m_listeners.clear();
}

};