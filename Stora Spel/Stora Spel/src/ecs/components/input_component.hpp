#ifndef INPUT_COMPONENT_HPP_
#define INPUT_COMPONENT_HPP_
#include <glm/glm.hpp>
#include <glob/graphics.hpp>
#include <util/input.hpp>
#include "util/transform_helper.hpp"
struct InputComponent {
  glm::vec2 pos = glm::vec2();
  glob::Font2DHandle font_hndl = 0;
  glob::GUIHandle gui_hndl = 0;
  glob::GUIHandle blinker_gui_hndl = 0;
  unsigned short font_size = 0;
  std::string text = "";
  std::string input_name = "";
  unsigned max_length = 16;
  bool is_active = false;
  int input_pos = 0;

  std::string* linked_value;

  bool IsClicked(glm::vec2 mouse_pos, glm::vec2 window_size) {
    if (Input::IsKeyPressed(0)) {
      if ((mouse_pos.x > pos.x && mouse_pos.x < pos.x + 100) &&
          (mouse_pos.y > window_size.y - pos.y - 10 &&
           mouse_pos.y < window_size.y - pos.y)) {
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
  InputComponent() {
    font_hndl = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
    gui_hndl = glob::GetGUIItem("Assets/GUI_elements/input_field.png");
    blinker_gui_hndl =
        glob::GetGUIItem("Assets/GUI_elements/input_blinker.png");
  }
};
#endif  // !INPUT_COMPONENT_HPP_
