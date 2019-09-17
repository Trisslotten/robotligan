#ifndef RENDER_SYSTEM_HPP_
#define RENDER_SYSTEM_HPP_

#include <entity/registry.hpp>
#include <glm/glm.hpp>

#include "ball_component.hpp"
#include "boundingboxes.hpp"
#include "collision.hpp"
#include "model_component.hpp"
#include "transform_component.hpp"
#include "wireframe_component.hpp"

// temp variable
bool render_wireframe = true;

void Render(entt::registry& registry) {
  auto view_model = registry.view<ModelComponent, TransformComponent>();

  for (auto& model : view_model) {
    auto& t = view_model.get<TransformComponent>(model);
    auto& m = view_model.get<ModelComponent>(model);
    glob::Submit(m.handle,
                 glm::translate(t.position) *
                     glm::rotate(-t.rotation.y, glm::vec3(0.f, 1.f, 0.f)) *
                     glm::translate(-m.offset) * glm::scale(t.scale));
  }


  // Render wireframes
  auto view_wireframe = registry.view<WireframeComponent, physics::OBB>();
  auto view_wireframe_s = registry.view<WireframeComponent, physics::Sphere, TransformComponent>();
  auto view_wireframe_a =
      registry.view<WireframeComponent, physics::Arena, TransformComponent>();
  if (render_wireframe) {
    for (auto& w : view_wireframe) {
      auto& wc = view_wireframe.get<WireframeComponent>(w);
      auto& o = view_wireframe.get<physics::OBB>(w);
      glob::SubmitCube(glm::translate(o.center) * glm::scale(wc.scale));
    }

    for (auto& w : view_wireframe_s) {
      auto& wc = view_wireframe_s.get<WireframeComponent>(w);
      auto& o = view_wireframe_s.get<TransformComponent>(w);
      glob::SubmitCube(glm::translate(o.position) * glm::scale(wc.scale));
    }
    for (auto& w : view_wireframe_a) {
      auto& wc = view_wireframe_a.get<WireframeComponent>(w);
      auto& o = view_wireframe_a.get<TransformComponent>(w);
      glob::SubmitCube(glm::translate(o.position) * glm::scale(wc.scale));
    }
  }
}
#endif  // RENDER_SYSTEM_HPP_