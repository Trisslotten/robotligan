#ifndef EVENT_HPP
#define EVENT_HPP
#include <entt.hpp>

extern entt::dispatcher dispatcher;

enum class Event {
	DESTROY_ENTITY,
	CREATE_CANNONBALL,
};

struct EventInfo {
  Event event;
  int entity_id;
};

#endif  // !EVENT_HPP