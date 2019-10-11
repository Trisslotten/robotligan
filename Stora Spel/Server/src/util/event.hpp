#ifndef EVENT_HPP
#define EVENT_HPP
#include <entt.hpp>

extern entt::dispatcher dispatcher;

enum class Event {
	DESTROY_ENTITY,
	CREATE_CANNONBALL,
	CREATE_FORCE_PUSH,
	CREATE_MISSILE,
	CHANGED_TARGET,
	NUM_OF_EVENTS,
};

struct EventInfo {
  Event event;
  entt::entity entity;
  int e_id;
};

#endif  // !EVENT_HPP