#ifndef BUTTON_SYSTEM_HPP_
#define BUTTON_SYSTEM_HPP_

#include <entt.hpp>

#include "ecs/components/button_component.hpp"
#include "shared/transform_component.hpp"
#include "util/input.hpp"
#include "util/transform_helper.hpp"

#include <glob/window.hpp>

namespace button_system {

void Update(entt::registry& registry) {
  auto view_buttons = registry.view<ButtonComponent, TransformComponent>();

  bool a_button_is_selected = false;
  for (auto entity : view_buttons) {
    glm::vec2 mouse_pos = Input::MousePos();
    ButtonComponent& button_c = registry.get<ButtonComponent>(entity);
    TransformComponent& trans_c = registry.get<TransformComponent>(entity);

    glm::vec2 button_pos = glm::vec2(trans_c.position.x, trans_c.position.y);

    mouse_pos.y = glob::window::GetWindowDimensions().y - mouse_pos.y +
                  button_c.font_size / 2;
    mouse_pos.x += button_c.font_size / 2;

    if (button_c.visible) {
      if (transform_helper::InsideBounds2D(mouse_pos, button_pos,
                                           button_c.bounds) &&
          !a_button_is_selected) {
        button_c.text_current_color = button_c.text_hover_color;
        if (button_c.gui_handle_hover) {
          button_c.gui_handle_current = button_c.gui_handle_hover;
			
		}
        if (Input::IsButtonPressed(GLFW_MOUSE_BUTTON_1)) {
          button_c.button_func();
        }
        a_button_is_selected = true;
      } else {
        button_c.text_current_color = button_c.text_normal_color;
        if (button_c.gui_handle_current) {
          button_c.gui_handle_current = button_c.gui_handle_normal;
		}
      }
	}
  }
}

}  // namespace button_system

#endif  // !BUTTON_SYSTEM_HPP_
