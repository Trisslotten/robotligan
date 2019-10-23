#ifndef INPUT_SYSTEM_HPP
#define INPUT_SYSTEM_HPP
#include <GLFW/glfw3.h>
#include <algorithm>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include "ecs/components/input_component.hpp"
#include "util/input.hpp"

namespace input_system {
bool active = false;
std::string last_active = "";
Timer back_space_timer;
float time_between_bs = 0.1f;
Timer blinker_timer;
float time_between_blinks = 0.3f;
bool blinker_shown = false;

int GetClosestCharPos(glm::vec2 pos, glm::vec2 click_pos, std::string content,
                      glob::Font2DHandle f_hndl, int size) {
  int char_pos = 0;
  float pixel_offset = click_pos.x - pos.x;

  float smallest_diff = 999999999.0f;
  for (int i = 0; i <= content.length(); i++) {
    std::string cur_sub = content.substr(0, i);
    double distance = glob::GetWidthOfText(f_hndl, cur_sub, size);
    float diff = abs(pixel_offset - distance);
    if (diff < smallest_diff) {
      char_pos = i;
      smallest_diff = diff;
    }
  }
  return std::max(0, char_pos);
}

void Update(entt::registry& registry) {
  auto view_inputs = registry.view<InputComponent>();
  auto window_size = glob::window::GetWindowDimensions();
  glm::vec2 mouse_pos = Input::MousePos();
  mouse_pos.y = glob::window::GetWindowDimensions().y - mouse_pos.y;

  bool found_a_box = false;
  for (auto entity : view_inputs) {
    int width_of_char = 14;
    InputComponent& input_c = registry.get<InputComponent>(entity);
    if (Input::IsButtonPressed(GLFW_MOUSE_BUTTON_1)) {
      if (transform_helper::InsideBounds2D(mouse_pos, input_c.pos,
                                           glm::vec2(200, 40))) {
        float pixel_offset = (mouse_pos.x - input_c.pos.x);
        input_c.input_pos = GetClosestCharPos(
            input_c.pos, mouse_pos, input_c.text, input_c.font_hndl,
            input_c.font_size);  //(int)pixel_offset / width_of_char;

        if (input_c.input_pos > input_c.text.length() && !active) {
          input_c.input_pos = input_c.text.length();
        }
        active = true;
        last_active = input_c.input_name;

        found_a_box = true;
      } else {
        if (!found_a_box)
			active = false;
      }
    }

    if (input_c.input_name == last_active) {
      if (Input::IsKeyDown(GLFW_KEY_LEFT) && input_c.input_pos > 0 &&
          back_space_timer.Elapsed() >= time_between_bs) {
        input_c.input_pos -= 1;
        blinker_shown = true;
        blinker_timer.Restart();
        back_space_timer.Restart();
      }
      if (Input::IsKeyDown(GLFW_KEY_RIGHT) &&
          input_c.input_pos < input_c.text.length() &&
          back_space_timer.Elapsed() >= time_between_bs) {
        input_c.input_pos += 1;
        blinker_shown = true;
        blinker_timer.Restart();
        back_space_timer.Restart();
      }
      if (input_c.text.length() < input_c.max_length) {
        std::string new_chars = Input::GetCharacters();
        input_c.text.insert(input_c.input_pos, new_chars.c_str());
        input_c.input_pos += new_chars.length();
        if (new_chars.length() > 0) {
          blinker_shown = true;
          blinker_timer.Restart();
          *input_c.linked_value = input_c.text;
        }
      }
      if (Input::IsKeyDown(GLFW_KEY_BACKSPACE) && input_c.text.size() > 0 &&
          back_space_timer.Elapsed() > time_between_bs &&
          input_c.input_pos > 0) {
        input_c.text.replace(input_c.input_pos - 1, 1, "");
        input_c.input_pos -= 1;
        back_space_timer.Restart();
        blinker_shown = true;
        blinker_timer.Restart();
      }
    }

    float opacity = input_c.input_name == last_active ? 1.0f : 0.7f;
    std::string sub_to_mark = input_c.text.substr(0, input_c.input_pos);
    double width_of_sub =
        glob::GetWidthOfText(input_c.font_hndl, sub_to_mark, input_c.font_size);

    // draw back
    glob::Submit(input_c.gui_hndl, input_c.pos, 1.0);

    // draw text in field
    glob::Submit(
        input_c.font_hndl, input_c.pos + glm::vec2(14, input_c.font_size / 2),
        input_c.font_size, input_c.text, glm::vec4(0.0f, 0.0f, 0.0f, opacity));

    // draw name
    float name_right =
        glob::GetWidthOfText(input_c.font_hndl, input_c.input_name, 20) / 2;

    glob::Submit(input_c.font_hndl,
                 input_c.pos + glm::vec2(100 - name_right, 40), 20,
                 input_c.input_name, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // blinker alternating logic
    if (blinker_timer.Elapsed() >= time_between_blinks &&
        !blinker_shown) {  // toggle blinker shown
      blinker_shown = true;
      blinker_timer.Restart();
    }

    if (input_c.input_name == last_active &&
        blinker_shown) {  // if shown, draw the blinker
      glob::Submit(input_c.blinker_gui_hndl,
                   input_c.pos + glm::vec2(input_c.input_pos + width_of_sub, 1),
                   1.0);

      if (blinker_timer.Elapsed() >=
          time_between_blinks) {  // switch between turning it on and off with
                                  // same interval using same timer
        blinker_shown = false;
        blinker_timer.Restart();
      }
    }
  }
  if (!active) {
    last_active = "";
  }
}
}  // namespace input_system

#endif
