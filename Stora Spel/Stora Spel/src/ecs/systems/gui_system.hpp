#ifndef GUI_SYSTEM_HPP_
#define GUI_SYSTEM_HPP_

#include <entt.hpp>

#include "ecs/components/button_component.hpp"
#include "shared/transform_component.hpp"
#include "util/input.hpp"
#include "util/transform_helper.hpp"

#include <GLFW/glfw3.h>
#include <glob/window.hpp>

namespace gui_system {

void Update(entt::registry& registry) {
  // ----- BUTTON LOGIC ------
  auto view_buttons = registry.view<ButtonComponent, TransformComponent>();
  bool a_button_is_selected = false;
  for (auto entity : view_buttons) {
    glm::vec2 mouse_pos = Input::MousePos();
    ButtonComponent& button_c = registry.get<ButtonComponent>(entity);
    TransformComponent& trans_c = registry.get<TransformComponent>(entity);

    glm::vec2 button_pos = glm::vec2(trans_c.position.x, trans_c.position.y);
    glm::vec2 click_pos = button_pos + button_c.click_offset;

    mouse_pos.y = glob::window::GetWindowDimensions().y - mouse_pos.y +
                  button_c.font_size / 2;
    mouse_pos.x += button_c.font_size / 2;
    
	if (button_c.text.size() > 0) {
      if (button_c.font_size > 0) button_c.bounds.y = button_c.font_size/2;

      double width_of_text = glob::GetWidthOfText(
          button_c.f_handle, button_c.text, button_c.font_size);

      if (width_of_text > 0.0) button_c.bounds.x = width_of_text;
	}

    if (button_c.visible) {
          if (transform_helper::InsideBounds2D(mouse_pos, click_pos,
                                           button_c.bounds) &&
          !a_button_is_selected) {
        if (!button_c.has_hovered) {
          // Trigger mouse hover event
          MenuEvent hover_event;
          hover_event.type = MenuEvent::HOVER;
          menu_dispatcher.trigger(hover_event);
          button_c.has_hovered = true; 
        }
        button_c.text_current_color = button_c.text_hover_color;
        if (button_c.gui_handle_hover) {
          button_c.gui_handle_current = button_c.gui_handle_hover;
        }
        if (Input::IsButtonPressed(GLFW_MOUSE_BUTTON_1)) {
          button_c.button_func();
          // Trigger mouse click event
          MenuEvent click_event;
          click_event.type = MenuEvent::CLICK;
          menu_dispatcher.trigger(click_event);
        }
        a_button_is_selected = true;
      } else {
        button_c.has_hovered = false;
        button_c.text_current_color = button_c.text_normal_color;
        if (button_c.gui_handle_current) {
          button_c.gui_handle_current = button_c.gui_handle_normal;
        }
      }
    }
  }

  // ----- SLIDER LOGIC ------
  auto view_sliders = registry.view<SliderComponent>();

  for (auto slider : view_sliders) {
    auto& slider_c = registry.get<SliderComponent>(slider);
    glm::vec2 mouse_pos = Input::MousePos();
    mouse_pos.y = glob::window::GetWindowDimensions().y - mouse_pos.y + 22 / 2;

    if (transform_helper::InsideBounds2D(mouse_pos, slider_c.position,
                                         slider_c.dimensions)) {
      float val_diff =
          (mouse_pos.x - slider_c.position.x) / slider_c.dimensions.x;
      //val_diff /= slider_c.increment;
      // float rest = val_diff % slider_c.increment;

      float range = slider_c.max_val - slider_c.min_val;
      if (Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_1)) {
        float newval = slider_c.min_val + val_diff * range;
        if (slider_c.increment != 0.0f) {
          newval = glm::round(newval / slider_c.increment) * slider_c.increment;
         // int top = (int)(newval + slider_c.increment);
//          newval = (float)top - slider_c.increment;

          slider_c.value = glm::max(slider_c.min_val,glm::min(newval,slider_c.max_val));
          *slider_c.value_to_write = slider_c.value;
        }
      }
    }
  }
}

}  // namespace gui_system

#endif  // !GUI_SYSTEM_HPP_
