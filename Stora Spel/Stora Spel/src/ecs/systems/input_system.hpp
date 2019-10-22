#ifndef INPUT_SYSTEM_HPP
#define INPUT_SYSTEM_HPP
#include <entt.hpp>
#include <glm/glm.hpp>
#include <glob/window.hpp>
#include <GLFW/glfw3.h>
#include "ecs/components/input_component.hpp"
#include "util/input.hpp"
void Update(entt::registry& registry) {
  auto view_inputs = registry.view<InputComponent>();
  auto window_size = glob::window::GetWindowDimensions();
  std::string last_active = "";
  bool active = false;
  glm::vec2 mouse_pos = Input::MousePos();
  for (auto entity : view_inputs) {
    InputComponent& input_c = registry.get<InputComponent>(entity);
    if (Input::IsButtonPressed(0)) {
      if (input_c.IsClicked(mouse_pos, window_size)) {
        last_active == input_c.input_name;
        active = true;
      } else {
        active = false;
      }
    }
    if (input_c.input_name == last_active) {
      if (input_c.text.length() < input_c.max_length) {
        input_c.text += Input::GetCharacters();
      }
      if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE) && input_c.text.size() > 0) {
        input_c.text.pop_back();
      }
    }
    //glob::Submit(input_c.font_hndl, input_c.pos, 0.5f, 150.0f);
    glob::Submit(input_c.font_hndl, input_c.pos, 20, input_c.text,
                 glm::vec4(0.0f, 0.0f, 0.0f, 1));
  }
  if (!active) {
    last_active = "";
  }
}

#endif
