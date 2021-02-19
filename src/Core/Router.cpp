#include <Aka/Core/Router.h>

namespace aka {

Router::Router() :
	m_current((ViewID)0)
{
}

void Router::detach(ViewID id) {
	auto it = m_views.find(id);
	if (it == m_views.end())
		return;
	it->second->onDestroy();
	m_views.erase(it);
}

void Router::set(ViewID id) {
	m_current = id;
}

View& Router::get() {
	return get(m_current);
}

View& Router::get(ViewID id) {
	auto it = m_views.find(id);
	if (it == m_views.end())
		throw std::runtime_error("Failed to attach view");
	return *it->second;
}

ViewID Router::current() const
{
	return m_current;
}

};