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
  Model *model = nullptr;
  glm::mat4 transform;
  float emission_strength;

  int material_index = 0;
  bool cast_shadow = true;
};

struct GUIItem {
  Elements2D *gui;
  glm::vec2 pos;
  float scale;
  float scale_x;
  float opacity;
  float rot;
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
  float emission_strength;

  int material_index = 0;
  bool cast_shadow = true;
};

struct CombinedRenderItem {
  bool is_animated = false;
  void* item = nullptr;
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
  glm::float32 sphere_radius;
};

struct PlaneLightItem {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec3 color;
  glm::vec2 sizes;
};

struct TrailItem {
  std::vector<glm::vec3> position_history;
  float width;
  glm::vec4 color;
};

}  // namespace glob

#endif  // GLOB_RENDER_ITEMS_HPP_