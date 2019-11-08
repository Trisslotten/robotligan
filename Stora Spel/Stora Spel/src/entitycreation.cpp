#include "entitycreation.hpp"

#include "ecs/components.hpp"
#include <boundingboxes.hpp>
#include <glob/graphics.hpp>
#include <glob\camera.hpp>
#include "shared/transform_component.hpp"
#include "engine.hpp"

ButtonComponent* GenerateButtonEntity(entt::registry& reg, std::string text,
                                      glm::vec2 pos,
                                      glob::Font2DHandle f_handle, bool visible,
                                      unsigned int font_size,
                                      glm::vec4 normal_color,
                                      glm::vec4 hover_color) {
  auto button = reg.create();
  reg.assign<ButtonComponent>(button);
  reg.assign<TransformComponent>(button, glm::vec3(pos.x, pos.y, 0));
  ButtonComponent& b_c = reg.get<ButtonComponent>(button);
  b_c.text = text;
  b_c.text_current_color = b_c.text_normal_color = normal_color;
  b_c.text_hover_color = hover_color;
  b_c.font_size = font_size;  // menu_settings::font_size;
  b_c.bounds = glm::vec2(b_c.font_size * b_c.text.size() / 2, b_c.font_size);
  b_c.f_handle = f_handle;
  b_c.visible = visible;
  b_c.click_offset = glm::vec2(20, 10);
  return &b_c;
}

