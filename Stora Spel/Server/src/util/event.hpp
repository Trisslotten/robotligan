#ifndef EVENT_HPP
#define EVENT_HPP
#include <entt.hpp>
#include <shared/shared.hpp>

extern entt::dispatcher dispatcher;

enum class Event {
  DESTROY_ENTITY,
  CREATE_CANNONBALL,
  CREATE_TELEPORT_PROJECTILE,
  CREATE_FORCE_PUSH,
  CREATE_MISSILE,
  CREATE_MINE,
  CREATE_HOOK,
  CHANGED_TARGET,
  CREATE_FAKE_BALL,
  BUILD_WALL,
  SPAWNER_SPAWNED_PICKUP,
  NUM_OF_EVENTS,
};

struct EventInfo {
  Event event;
  entt::entity entity;
  int e_id;
  EntityID owner_id;
};

#endif  // !EVENT_HPP