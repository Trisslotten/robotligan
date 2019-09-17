#ifndef RENDER_SYSTEM_HPP_
#define RENDER_SYSTEM_HPP_

#include <entity/registry.hpp>
#include <glm/glm.hpp>

#include "ball_component.hpp"
#include "boundingboxes.hpp"
#include "collision.hpp"
#include <glob/graphics.hpp>
#include "transform_component.hpp"
#include "velocity_component.hpp"
#include "wireframe_component.hpp"

// temp variable
bool render_wireframe = false;

void Render(entt::registry& registry) {
  auto view_model = registry.view<glob::ModelHandle, TransformComponent>();

  for (auto& model : view_model) {
    auto& t = view_model.get<TransformComponent>(model);
    glob::Submit(view_model.get<glob::ModelHandle>(model),
                 glm::translate(t.position) * glm::scale(t.scale));
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