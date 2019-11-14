#ifndef GLOB_RENDER_ITEMS_HPP_
#define GLOB_RENDER_ITEMS_HPP_


#include "Model/model.hpp"
#include "Particles/particle_settings.hpp"
#include "glob/camera.hpp"
#include "glob/window.hpp"
#include "particles/particle_system.hpp"
#include "2D/elements2D.hpp"
#include "Font/Font2D.hpp"

namespace glob {


struct RenderItem {
  Model *model;
  glm::mat4 transform;

  int material_index;
};

struct GUIItem {
  Elements2D *gui;
  glm::vec2 pos;
  float scale;
  float scale_x;
  float opacity;
};

struct E2DItem {
  Elements2D *e2D;
  glm::vec3 pos;
  float scale;
  glm::mat4 rot;
};

struct BoneAnimatedRenderItem {
  Model *model;
  glm::mat4 transform;
  std::vector<glm::mat4>
      bone_transforms;  // may be a performance bottleneck, pointer instead?
  int numBones;

  int material_index = 0;
};

struct TextItem {
  Font2D *font = nullptr;
  glm::vec2 pos{0};
  unsigned int size = 0;
  std::string text;
  glm::vec4 color;
  bool visible;
  bool equal_spacing;
  float spacing;
};

struct Text3DItem {
  Font2D *font = nullptr;
  glm::vec3 pos{0};
  float size = 0.f;
  std::string text;
  glm::vec4 color = glm::vec4(1.f);
  glm::mat4 rotation = glm::mat4(1.f);
};

struct LightItem {
  glm::vec3 pos;
  glm::vec3 color;
  glm::float32 radius;
  glm::float32 ambient;
};

struct TrailItem {
  std::vector<glm::vec3> position_history;
  float width;
  glm::vec4 color;
};

}  // namespace glob

#endif  // GLOB_RENDER_ITEMS_HPP_