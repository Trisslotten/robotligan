#ifndef BUTTON_COMPONENT_HPP_
#define BUTTON_COMPONENT_HPP_

#include <string>
#include <glm/glm.hpp>
#include <glob/graphics.hpp>
#include <functional>

struct ButtonComponent {
  glob::Font2DHandle f_handle;
  std::string text = "Button";
  unsigned int font_size = 72;
  glm::vec4 text_normal_color = glm::vec4(1,1,1,1);
  glm::vec4 text_hover_color = glm::vec4(1,0,1,1);
  glm::vec4 text_current_color = glm::vec4(1, 1, 1, 1);
  std::string back_texture;
  glm::vec2 bounds = glm::vec2(200, 50);
  bool visible = true;
  std::function<void()> button_func;
  glob::GUIHandle gui_handle_icon;
  glob::GUIHandle gui_handle_normal;
  glob::GUIHandle gui_handle_hover;
  glob::GUIHandle gui_handle_current;

  std::string find_name = "";
};

#endif  // !BUTTON_COMPONENT_HPP_
