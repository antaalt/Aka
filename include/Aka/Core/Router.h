#pragma once 

#include <Aka/Core/View.h>

#include <stdexcept>
#include <map>

namespace aka {

// Handle scenes.
class Router
{
public:
	Router();

	// Attach a view to the router
	template <typename T, typename... Args>
	View& attach(ViewID id, Args&&... args);
	// Remove the specified view manually.
	void detach(ViewID id);

	// Set the next view
	void set(ViewID id);

	// Get the current view
	View& get();
	// Get the specified view
	View& get(ViewID id);
	// Get the current id
	ViewID current() const;
private:
	ViewID m_current;
	std::map<ViewID, View::Ptr> m_views;
};


template <typename T, typename... Args>
View& Router::attach(ViewID id, Args&&... args) {
	static_assert(std::is_base_of<View, T>::value, "Type is not a view");
	auto it = m_views.insert(std::make_pair(id, std::make_unique<T>(std::forward<Args>(args)...)));
	if (!it.second)
		throw std::runtime_error("Failed to attach view");
	return *it.first->second;
}

};