#ifndef RENDER_SYSTEM_HPP_
#define RENDER_SYSTEM_HPP_

#include <entity/registry.hpp>
#include <glm/glm.hpp>

#include "boundingboxes.hpp"
#include "collision.hpp"
#include "ecs/components.hpp"
#include "shared/camera_component.hpp"
#include "shared/transform_component.hpp"
#include "util/global_settings.hpp"

void RenderSystem(entt::registry& registry) {
  auto view_cam = registry.view<CameraComponent, TransformComponent>();
  for (auto& cam_entity : view_cam) {
    auto& cam_c = view_cam.get<CameraComponent>(cam_entity);
    auto& trans_c = view_cam.get<TransformComponent>(cam_entity);

    Camera result = glob::GetCamera();
    glm::vec3 position =
        trans_c.position + glm::quat(trans_c.rotation) * cam_c.offset;
    result.SetOrientation(cam_c.orientation);
    result.SetPosition(position);
    glob::SetCamera(result);
  }

  // auto view_model = registry.group<ModelComponent, TransformComponent>(
  //    entt::exclude<AnimationComponent>);
  auto view_model = registry.view<ModelComponent, TransformComponent>();
  for (auto& model : view_model) {
    if (registry.has<AnimationComponent>(model) == false) {
      auto& t = view_model.get<TransformComponent>(model);
      auto& m = view_model.get<ModelComponent>(model);

      if (!m.invisible) {
        glob::Submit(m.handles,
          glm::translate(t.position) * glm::toMat4(t.rotation) *
          glm::translate(-m.offset) * glm::scale(t.scale), m.diffuse_index);
      }
    }
  }

  auto animated_models =
      registry.view<ModelComponent, TransformComponent, AnimationComponent>();
  for (auto& model : animated_models) {
    auto& t = animated_models.get<TransformComponent>(model);
    auto& m = animated_models.get<ModelComponent>(model);
    auto& a = animated_models.get<AnimationComponent>(model);

    if (!m.invisible) {
      glob::SubmitBAM(m.handles,
        glm::translate(t.position) *
        glm::toMat4(t.rotation + m.rot_offset) *
        glm::translate(-m.offset) * glm::scale(t.scale),
        a.bone_transforms, m.diffuse_index);
    }
  }

  // submit particles
  auto view_particles = registry.view<ParticleComponent>();
  for (auto entity : view_particles) {
    auto& particle_c = view_particles.get(entity);

    for (int i = 0; i < particle_c.handles.size(); ++i) {
      glob::SubmitParticles(particle_c.handles[i]);
    }
  }

  // submit lights
  auto lights = registry.view<LightComponent, TransformComponent>();
  for (auto& l : lights) {
    auto& transform = lights.get<TransformComponent>(l);
    auto& light = lights.get<LightComponent>(l);

    // glm::mat4 mat = glm::translate(transform.position) *
    // glm::scale(transform.scale) *
    // glm::mat4_cast(glm::quat(transform.rotation));
    glm::vec3 pos = transform.position;
    glm::vec3 dir = glm::quat(transform.rotation) * glm::vec3(1.f, 0.f, 0.f);
    if (!light.blackout) {
      glob::SubmitLightSource(pos, light.color, light.radius, light.ambient);
    }
  }

  // Render wireframes
  auto view_wireframe_obb = registry.view<physics::OBB, TransformComponent>();
  auto view_wireframe_sphere = registry.view<physics::Sphere>();
  auto view_wireframe_arena = registry.view<physics::Arena>();
  auto view_wireframe_mesh =
      registry.view<physics::MeshHitbox, ModelComponent>();
  if (GlobalSettings::Access()->ValueOf("RENDER_WIREFRAME") == 1.0f) {
    for (auto& w : view_wireframe_obb) {
      auto& obb = view_wireframe_obb.get<physics::OBB>(w);
      auto& transform = view_wireframe_obb.get<TransformComponent>(w);
      glob::SubmitCube(glm::translate(obb.center) *
                       glm::toMat4(transform.rotation) *
                       glm::scale(glm::vec3(obb.extents[0], obb.extents[1],
                                            obb.extents[2])));
    }
    for (auto& w : view_wireframe_sphere) {
      auto& sphere = view_wireframe_sphere.get(w);
      glob::SubmitCube(glm::translate(sphere.center) *
                       glm::scale(glm::vec3(sphere.radius)));
    }
    for (auto& w : view_wireframe_arena) {
      auto& arena = view_wireframe_arena.get(w);
      glob::SubmitCube(
          glm::scale(glm::vec3(arena.xmax - arena.xmin, arena.ymax - arena.ymin,
                               arena.zmax - arena.zmin) *
                     0.5f));
    }
  }

  auto view_buttons = registry.view<ButtonComponent, TransformComponent>();

  for (auto& button : view_buttons) {
    ButtonComponent& button_c = view_buttons.get<ButtonComponent>(button);
    TransformComponent& trans_c = view_buttons.get<TransformComponent>(button);

    glm::vec2 button_pos = glm::vec2(trans_c.position.x, trans_c.position.y);

    if (button_c.visible) {
      glob::Submit(button_c.f_handle, button_pos + glm::vec2(1, -1),
                   button_c.font_size, button_c.text, glm::vec4(0, 0, 0, 1));
      glob::Submit(button_c.f_handle, button_pos, button_c.font_size,
                   button_c.text, button_c.text_current_color);
      
      if (button_c.gui_handle_current) {
        glob::Submit(button_c.gui_handle_current, button_pos, 1.f);

        if (button_c.gui_handle_icon) {
          glob::Submit(button_c.gui_handle_icon, button_pos, 1.f);
        }
      }
      if (button_c.has_hovered) {
        glm::vec2 tooltip_pos = Input::MousePos();
        tooltip_pos.y =
            glob::window::GetWindowDimensions().y - tooltip_pos.y - 15;
        tooltip_pos.x += 20;
		glob::Submit(button_c.f_handle, tooltip_pos - glm::vec2(1,1), 32, button_c.hover_text, glm::vec4(0,0,0,0.7));
        glob::Submit(button_c.f_handle, tooltip_pos, 32, button_c.hover_text);
	  }
    }
  }

  auto view_sliders = registry.view<SliderComponent>();

  for (auto slider : view_sliders) {
    auto& slider_c = registry.get<SliderComponent>(slider);
    float indent_text = 0;
    //-4 * slider_c.text.length() / 2;
    glob::Submit(slider_c.font_handle,
                 slider_c.position + glm::vec2(indent_text, 60), 22,
                 slider_c.text);

    if (slider_c.back_tex) {
      glob::Submit(slider_c.back_tex, slider_c.position, 1.f);
    }
    if (slider_c.front_tex) {
      glm::vec2 pin_pos = slider_c.position - glm::vec2(10, 0);
      float range = slider_c.max_val - slider_c.min_val;
      float norm = slider_c.value - slider_c.min_val;
      float perc = norm / range;
      pin_pos.x += perc * slider_c.dimensions.x;
      glob::Submit(slider_c.front_tex, pin_pos, 1.f);
    }
    glob::Submit(slider_c.font_handle, slider_c.position + glm::vec2(40, -10),
                 22, std::to_string(slider_c.value));
  }

  auto view_trails = registry.view<TrailComponent>();
  for (auto entity : view_trails) {
    auto& trail_c = view_trails.get(entity);
    glob::SubmitTrail(trail_c.position_history, trail_c.width, trail_c.color);
  }
}
#endif  // RENDER_SYSTEM_HPP_