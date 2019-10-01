#ifndef ANIMATION_SYSTEM_HPP_
#define ANIMATION_SYSTEM_HPP_

#include <entity/registry.hpp>

#include "ball_component.hpp"
#include "boundingboxes.hpp"
#include "collision.hpp"
#include "physics.hpp"
#include "physics_component.hpp"
#include "transform_component.hpp"

void UpdatePhysics(entt::registry& registry, float dt) {
	auto view_moveable = registry.view<TransformComponent, PhysicsComponent>();

	for (auto entity : view_moveable) {
		TransformComponent& trans_c = view_moveable.get<TransformComponent>(entity);
		PhysicsComponent& physics_c = view_moveable.get<PhysicsComponent>(entity);

		physics::PhysicsObject po;
		po.airborne = physics_c.is_airborne;
		po.friction = physics_c.friction;
		po.position = trans_c.position;
		po.velocity = physics_c.velocity;

		Update(&po, dt);

		trans_c.position = po.position;
		physics_c.velocity = po.velocity;

		// trans_c.position += physics_c.velocity;
	}
}

#endif  // PHYSICS_SYSTEM_HPP_
