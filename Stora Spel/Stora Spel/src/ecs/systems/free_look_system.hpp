#ifndef FREE_LOOK_SYSTEM_HPP_
#define FREE_LOOK_SYSTEM_HPP_

#include <ecs/components.hpp>
#include <entt.hpp>
#include <util/input.hpp>

namespace free_look_system {
bool active = false;
glm::quat old_ori;
glm::vec3 old_off = glm::vec3(-0.2f, 0.4f, 0.f);
float pitch = 0.f;
float yaw = 0.f;

void SetActive(entt::registry& registry, bool val) {
  active = val;

  auto view_camera = registry.view<CameraComponent,TransformComponent>();
  for (auto camera : view_camera) {
    CameraComponent& c_c = registry.get<CameraComponent>(camera);
    TransformComponent& t_c = registry.get<TransformComponent>(camera);
    t_c.rotation = glm::quat();
    if (val) {
      old_ori = c_c.orientation;
      old_off = c_c.offset;
    } else {
      c_c.orientation = old_ori;
      c_c.offset = old_off;
    }
  }
}

void Update(entt::registry& registry, float dt) {
  if (active) {
    auto view_camera = registry.view<CameraComponent>();
    for (auto camera : view_camera) {
      CameraComponent& c_c = registry.get<CameraComponent>(camera);

      float speed = 10.f;
      

      glm::vec2 mouse_off =
          Input::MouseMov() * 0.003f *
          GlobalSettings::Access()->ValueOf("INPUT_MOUSE_SENS");
      yaw -= mouse_off.x;
      pitch -= mouse_off.y;
      constexpr float pi = glm::pi<float>();
      pitch = glm::clamp(pitch, -0.49f * pi, 0.49f * pi);

      glm::quat new_orientation = glm::quat(glm::vec3(0, yaw, 0)) * glm::quat(glm::vec3(0, 0,	pitch));
      new_orientation = glm::normalize(new_orientation);
      c_c.orientation = new_orientation;

	  glm::vec3 right = glm::cross(c_c.GetLookDir(), glm::vec3(0, 1, 0));
      glm::vec3 dir(0);
      if (Input::IsKeyDown(GLFW_KEY_W)) {
        dir += c_c.GetLookDir();
      }
      if (Input::IsKeyDown(GLFW_KEY_S)) {
        dir -= c_c.GetLookDir();
      }
      if (Input::IsKeyDown(GLFW_KEY_D)) {
        dir += right;
      }
      if (Input::IsKeyDown(GLFW_KEY_A)) {
        dir -= right;
      }
      if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
        speed = 30.f;
	  }
      if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
          speed = 3.f;
      }
      if (glm::length(dir) > 0)
		 dir = glm::normalize(dir);

      c_c.offset += dir * speed * dt;
    }
  }
}

}  // namespace free_look_system

#endif  // FREE_LOOK_SYSTEM_HPP_