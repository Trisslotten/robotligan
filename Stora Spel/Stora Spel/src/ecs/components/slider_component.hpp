#ifndef SLIDER_COMPONENT_HPP_
#define SLIDER_COMPONENT_HPP_

#include <string>
#include <glob/graphics.hpp>

struct SliderComponent {
  glm::vec2 position;
  float min_val = 0.f;
  float max_val = 100.f;
  float value = 50.f;
  float increment = .5f;
  glm::vec2 dimensions = glm::vec2(150.f, 50.f);
  std::string text = "slider";
  float* value_to_write = nullptr;
  glob::GUIHandle back_tex = 0;
  glob::GUIHandle front_tex = 0;
  glob::Font2DHandle font_handle = 0;
};

#endif  // SLIDER_COMPONENT_HPP