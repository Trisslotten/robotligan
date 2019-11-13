#include "glob/graphics.hpp"

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <msdfgen/msdfgen-ext.h>
#include <msdfgen/msdfgen.h>
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <lodepng.hpp>
#include <map>
#include <sstream>
#include <unordered_map>

#include <textureslots.hpp>
#include "2D/elements2D.hpp"
#include "Font/Font2D.hpp"
#include "Model/model.hpp"
#include "Particles/particle_settings.hpp"
#include "glob/camera.hpp"
#include "glob/window.hpp"
#include "material\material.hpp"
#include "particles/particle_system.hpp"
#include "postprocess/blur.hpp"
#include "postprocess/postprocess.hpp"
#include "postprocess/ssao.hpp"
#include "renderitems.hpp"
#include "shader.hpp"
#include "shadows/shadows.hpp"

namespace glob {

bool kModelUseGL = true;

namespace {

ShaderProgram fullscreen_shader;
ShaderProgram model_shader;
ShaderProgram particle_shader;
// ShaderProgram compute_shader;
ShaderProgram animated_model_shader;
ShaderProgram text_shader;
ShaderProgram text3D_shader;
ShaderProgram wireframe_shader;
ShaderProgram gui_shader;
ShaderProgram e2D_shader;
ShaderProgram ssao_shader;
ShaderProgram sky_shader;

std::vector<ShaderProgram *> mesh_render_group;

ShaderProgram trail_shader;
int num_trail_quads = 0;
GLuint triangle_vbo, triangle_vao;
GLuint cube_vbo, cube_vao;
GLuint quad_vbo, quad_vao;
GLuint trail_vao, trail_vbo;

GLuint black_texture;
GLuint default_normal_texture;

GLuint sky_texture = 0;

PostProcess post_process;
Blur blur;
Shadows shadows;

GLint is_invisible = 0;
float num_frames = 0;

Camera camera;
Ssao ssao;

bool use_ao = true;

/*
TextureHandle current_texture_guid = 1;
std::unordered_map<std::string, TextureHandle> texture_handles;
std::unordered_map<TextureHandle, Texture> textures;
*/

ModelHandle current_model_guid = 1;
ParticleSystemHandle current_particle_guid = 1;
Font2DHandle current_font_guid = 1;
GUIHandle current_gui_guid = 1;
E2DHandle current_e2D_guid = 1;
std::unordered_map<std::string, ModelHandle> model_handles;
std::unordered_map<ModelHandle, Model> models;
std::unordered_map<ParticleSystemHandle, int> particle_systems;
std::unordered_map<std::string, GLuint> textures;
std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> compute_shaders;

struct ParticleSystemInfo {
  ParticleSystem system;
  bool in_use;

  ParticleSystemInfo(ShaderProgram *ptr, GLuint tex, bool use)
      : system(ptr, tex), in_use(use) {}
};
std::vector<ParticleSystemInfo> buffer_particle_systems;
std::unordered_map<std::string, Font2DHandle> font_2D_handles;
std::unordered_map<Font2DHandle, Font2D> fonts;
std::unordered_map<std::string, GUIHandle> gui_handles;
std::unordered_map<GUIHandle, Elements2D> gui_elements;
std::unordered_map<std::string, E2DHandle> e2D_handles;
std::unordered_map<E2DHandle, Elements2D> e2D_elements;

struct GLuintBuffers {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  GLuint size;
};

std::unordered_map<ModelHandle, GLuintBuffers> wireframe_buffers;

std::vector<RenderItem> items_to_render;
std::vector<LightItem> lights_to_render;
std::vector<int> particles_to_render;
std::vector<BoneAnimatedRenderItem> bone_animated_items_to_render;
std::vector<glm::mat4> cubes;
std::vector<ModelHandle> wireframe_meshes;
std::vector<TextItem> text_to_render;
std::vector<Text3DItem> text3D_to_render;
std::vector<GUIItem> gui_items_to_render;
std::vector<E2DItem> e2D_items_to_render;
std::vector<TrailItem> trails_to_render;

void SetDefaultMaterials(ShaderProgram &shader) {
  glActiveTexture(GL_TEXTURE0 + TEXTURE_SLOT_EMISSIVE);
  glBindTexture(GL_TEXTURE_2D, black_texture);
  shader.uniform("texture_emissive", TEXTURE_SLOT_EMISSIVE);

  glActiveTexture(GL_TEXTURE0 + TEXTURE_SLOT_NORMAL);
  glBindTexture(GL_TEXTURE_2D, default_normal_texture);
  shader.uniform("texture_normal", TEXTURE_SLOT_NORMAL);

  glActiveTexture(GL_TEXTURE0 + TEXTURE_SLOT_METALLIC);
  glBindTexture(GL_TEXTURE_2D, black_texture);
  shader.uniform("texture_metallic", TEXTURE_SLOT_METALLIC);

  glActiveTexture(GL_TEXTURE0 + TEXTURE_SLOT_ROUGHNESS);
  glBindTexture(GL_TEXTURE_2D, black_texture);
  shader.uniform("texture_roughness", TEXTURE_SLOT_ROUGHNESS);
}

void DrawFullscreenQuad() {
  glBindVertexArray(triangle_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
}

}  // namespace
void DrawCube(glm::mat4 t) {
  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  wireframe_shader.use();
  wireframe_shader.uniform("cam_transform", cam_transform);
  wireframe_shader.uniform("model_transform", t);
  glBindVertexArray(cube_vao);
  glDisable(GL_DEPTH_TEST);
  // glLineWidth(2);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDisable(GL_CULL_FACE);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glEnable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  glBindVertexArray(0);
}

void DrawWireFrameMeshes(ModelHandle model_h) {
  auto find_res = wireframe_buffers.find(model_h);
  if (find_res == wireframe_buffers.end()) {
    return;
  }

  auto b = wireframe_buffers[model_h];
  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  wireframe_shader.use();
  wireframe_shader.uniform("cam_transform", cam_transform);
  wireframe_shader.uniform("model_transform", glm::identity<glm::mat4>());
  glBindVertexArray(b.vao);
  glDisable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDisable(GL_CULL_FACE);
  glDrawElements(GL_TRIANGLES, b.size, GL_UNSIGNED_INT, 0);
  glEnable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  glBindVertexArray(0);
}

void CreateDefaultParticleTexture() {
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glm::vec2 middle(63.5f, 63.5f);
  std::vector<float> data(128 * 128 * 4);
  for (size_t y = 0; y < 128; ++y) {
    for (size_t x = 0; x < 128; ++x) {
      glm::vec2 result = middle - glm::vec2(x, y);
      float value = 1.0f;
      if (glm::length(result) > 63.5f) {
        value = 0.0f;
      } else {
        value = 1.0f - (glm::length(result) / 63.5f);
      }

      for (int i = 0; i < 3; ++i) data[y * 128 * 4 + x * 4 + i] = 1.0f;

      data[y * 128 * 4 + x * 4 + 3] = value;
    }
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_FLOAT,
               data.data());

  textures["default"] = texture;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  data.clear();
  data.resize(2 * 2 * 4, 1.0f);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_FLOAT,
               data.data());

  textures["quad"] = texture;
}

GLint TextureFromFile(std::string filename) {
  const char *path = "Assets/Texture";
  std::string directory = std::string(path);
  filename = directory + '/' + filename;

  // Generate texture id
  GLuint texture_id;
  glGenTextures(1, &texture_id);

  // Load texture
  std::vector<unsigned char> image;
  unsigned width, height;

  unsigned error = lodepng::decode(image, width, height, filename);
  if (error != 0) {
    std::cout << "ERROR: Could not load texture: " << filename << "\n";
    return false;
  }

  // Generate texture data
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image.data());
  // glGenerateMipmap(GL_TEXTURE_2D);

  // Set some parameters for the texture

  glBindTexture(GL_TEXTURE_2D, 0);  // Unbind the texture

  return texture_id;
}

void Init() {
  camera = Camera(glm::vec3(25, 5, 0), glm::vec3(0, 3, 0), 90, 16.f / 9.f, 0.1f,
                  200.f);

  // std::cout << "Max uniform size: " << MAX_VERTEX_UNIFORM_COMPONENTS_ARB <<
  // "\n";
  fullscreen_shader.add("fullscreenquad.vert");
  fullscreen_shader.add("fullscreenquad.frag");
  fullscreen_shader.compile();

  ssao_shader.add("fullscreenquad.vert");
  ssao_shader.add("ssao.frag");
  ssao_shader.compile();

  particle_shader.add("particle.vert");
  particle_shader.add("particle.geom");
  particle_shader.add("particle.frag");
  particle_shader.compile();

  compute_shaders["default"] = std::make_unique<ShaderProgram>();
  compute_shaders["default"]->add(
      "Particle compute shaders/compute_shader.comp");
  compute_shaders["default"]->compile();

  compute_shaders["confetti"] = std::make_unique<ShaderProgram>();
  compute_shaders["confetti"]->add("Particle compute shaders/confetti.comp");
  compute_shaders["confetti"]->compile();

  compute_shaders["dust"] = std::make_unique<ShaderProgram>();
  compute_shaders["dust"]->add("Particle compute shaders/dust.comp");
  compute_shaders["dust"]->compile();

  CreateDefaultParticleTexture();
  textures["smoke"] = TextureFromFile("smoke.png");
  textures["confetti"] = TextureFromFile("confetti.png");
  textures["dust"] = TextureFromFile("dust.png");

  model_shader.add("modelshader.vert");
  model_shader.add("modelshader.frag");
  model_shader.add("shading.vert");
  model_shader.add("shading.frag");
  model_shader.compile();

  animated_model_shader.add("animatedmodelshader.vert");
  animated_model_shader.add("modelshader.frag");
  animated_model_shader.add("shading.vert");
  animated_model_shader.add("shading.frag");
  animated_model_shader.compile();

  mesh_render_group.push_back(&animated_model_shader);
  mesh_render_group.push_back(&model_shader);

  wireframe_shader.add("modelshader.vert");
  wireframe_shader.add("shading.vert");
  wireframe_shader.add("wireframe.frag");
  wireframe_shader.compile();

  text_shader.add("text2Dshader.vert");
  text_shader.add("text2Dshader.frag");
  text_shader.compile();

  text3D_shader.add("text3Dshader.vert");
  text3D_shader.add("text3Dshader.frag");
  text3D_shader.compile();

  gui_shader.add("guishader.vert");
  gui_shader.add("guishader.frag");
  gui_shader.compile();

  e2D_shader.add("e2Dshader.vert");
  e2D_shader.add("e2Dshader.frag");
  e2D_shader.compile();

  sky_shader.add("sky.vert");
  sky_shader.add("sky.frag");
  sky_shader.compile();

  glGenVertexArrays(1, &triangle_vao);
  glBindVertexArray(triangle_vao);
  std::vector<glm::vec3> vertices{
      {-1, -1, 0},
      {3, -1, 0},
      {-1, 3, 0},
  };
  glGenBuffers(1, &triangle_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                        (GLvoid *)0);

  glGenVertexArrays(1, &cube_vao);
  glBindVertexArray(cube_vao);
  std::vector<glm::vec3> vertices_cube{
      {1, 1, 1},    {1, -1, 1},   {-1, 1, 1},  // z+ face
      {-1, -1, 1},  {1, -1, 1},   {-1, 1, 1},

      {1, 1, -1},   {1, -1, -1},  {1, -1, 1},  // x+ face
      {1, 1, -1},   {1, 1, 1},    {1, -1, 1},

      {-1, 1, -1},  {1, 1, -1},   {1, 1, 1},  // y+ face
      {-1, 1, -1},  {1, 1, 1},    {-1, 1, 1},

      {-1, -1, -1}, {1, 1, -1},   {-1, 1, -1},  // z- face
      {1, 1, -1},   {-1, -1, -1}, {1, -1, -1},

      {-1, -1, -1}, {1, -1, -1},  {1, -1, 1},  // y- face
      {-1, -1, 1},  {1, -1, 1},   {1, -1, -1},

      {-1, -1, -1}, {-1, -1, 1},  {-1, 1, -1},  // x- face
      {-1, -1, 1},  {-1, 1, -1},  {-1, 1, 1}};
  glGenBuffers(1, &cube_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices_cube.size(),
               vertices_cube.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                        (GLvoid *)0);

  std::vector<glm::vec3> quad_vertices{
      {-1, -1, 0}, {1, -1, 0}, {-1, 1, 0}, {1, 1, 0}};
  glGenVertexArrays(1, &quad_vao);
  glBindVertexArray(quad_vao);
  glGenBuffers(1, &quad_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * quad_vertices.size(),
               quad_vertices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                        (GLvoid *)0);
  glBindVertexArray(0);

  trail_shader.add("trail.vert");
  trail_shader.add("trail.frag");
  trail_shader.compile();
  num_trail_quads = 200;
  std::vector<glm::vec3> trail_verts;
  for (int i = 0; i < num_trail_quads; i++) {
    float left = float(i) / (num_trail_quads);
    float right = float(i + 1) / (num_trail_quads);
    glm::vec3 tl{left, 0, -0.5f};
    glm::vec3 tr{right, 0, -0.5f};
    glm::vec3 bl{left, 0, 0.5f};
    glm::vec3 br{right, 0, 0.5f};
    trail_verts.push_back(tl);
    trail_verts.push_back(bl);
    trail_verts.push_back(tr);

    trail_verts.push_back(tr);
    trail_verts.push_back(bl);
    trail_verts.push_back(br);
  }
  glGenVertexArrays(1, &trail_vao);
  glBindVertexArray(trail_vao);
  glGenBuffers(1, &trail_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, trail_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * trail_verts.size(),
               trail_verts.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                        (GLvoid *)0);
  glBindVertexArray(0);

  glGenTextures(1, &black_texture);
  glBindTexture(GL_TEXTURE_2D, black_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  unsigned char black_data[4] = {0, 0, 0, 0};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               &black_data);

  glGenTextures(1, &default_normal_texture);
  glBindTexture(GL_TEXTURE_2D, default_normal_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  unsigned char default_normal_data[4] = {127, 127, 255, 0};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               &default_normal_data);

  materials::Init();
  blur.Init();
  post_process.Init(blur);
  shadows.Init(blur);
  ssao.Init(blur);

  buffer_particle_systems.reserve(10);

  SetSky("assets/texture/nightsky.png");
}

// H=Handle, A=Asset
template <class H, class A>
H GetAsset(std::unordered_map<std::string, H> &handles,
           std::unordered_map<H, A> &assets, H &guid,
           const std::string filepath) {
  H result = 0;

  std::string borg = filepath;
  std::transform(borg.begin(), borg.end(), borg.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  auto item = handles.find(filepath);
  if (item == handles.end()) {
    // std::cout << "DEBUG graphics.cpp: Loading asset '" << filepath << "'\n";
    A &asset = assets[guid];
    asset.LoadFromFile(filepath);
    if (asset.IsLoaded()) {
      handles[filepath] = guid;
      result = guid;
      guid++;
    } else {
      // remove the asset since it could not load
      assets.erase(guid);
    }
  } else {
    // if asset is loaded
    /*
    std::cout << "DEBUG graphics.cpp: Asset '" << filepath
              << "' already loaded\n";
    */
    result = item->second;
  }

  return result;
}

ModelHandle GetModel(const std::string &filepath) {
  return GetAsset(model_handles, models, current_model_guid, filepath);
}

ModelHandle GetTransparentModel(const std::string &filepath) {
  auto result = GetAsset(model_handles, models, current_model_guid, filepath);
  models[result].SetTransparent(true);
  return result;
}

ParticleSettings ProccessMap(
    ParticleSettings ps,
    const std::unordered_map<std::string, std::string> &map,
    const std::vector<std::string> &colors) {
  bool color_delta = false;
  glm::vec4 end_color = glm::vec4(1.f);
  bool vel_delta = false;
  float end_vel = 0.0f;
  bool size_delta = false;
  float end_size;

  for (auto it : map) {
    if (it.first == "color") {
      std::stringstream ss(it.second);
      glm::vec4 col;
      ss >> col.x;
      ss >> col.y;
      ss >> col.z;
      ss >> col.w;

      ps.colors[0] = col;
    } else if (it.first == "end_color") {
      color_delta = true;
      std::stringstream ss(it.second);
      glm::vec4 col;
      ss >> col.x;
      ss >> col.y;
      ss >> col.z;
      ss >> col.w;

      end_color = col;
    } else if (it.first == "emit_pos") {
      std::stringstream ss(it.second);
      glm::vec3 pos;
      ss >> pos.x;
      ss >> pos.y;
      ss >> pos.z;

      ps.emit_pos = pos;
    } else if (it.first == "size") {
      std::stringstream ss(it.second);
      float size;
      ss >> size;

      ps.size = size;
    } else if (it.first == "end_size") {
      size_delta = true;
      std::stringstream ss(it.second);
      float size;
      ss >> size;

      end_size = size;
    } else if (it.first == "time") {
      std::stringstream ss(it.second);
      float time;
      ss >> time;

      ps.time = time;
    } else if (it.first == "spawn_rate") {
      std::stringstream ss(it.second);
      float rate;
      ss >> rate;

      ps.spawn_rate = rate;
    } else if (it.first == "velocity") {
      std::stringstream ss(it.second);
      float vel;
      ss >> vel;

      if (ps.velocity == ps.min_velocity) ps.min_velocity = vel;

      ps.velocity = vel;
    } else if (it.first == "min_velocity") {
      std::stringstream ss(it.second);
      float vel;
      ss >> vel;

      ps.min_velocity = vel;
    } else if (it.first == "end_velocity") {
      vel_delta = true;
      std::stringstream ss(it.second);
      float vel;
      ss >> vel;

      end_vel = vel;
    } else if (it.first == "direction") {
      std::stringstream ss(it.second);
      glm::vec3 dir;
      ss >> dir.x;
      ss >> dir.y;
      ss >> dir.z;

      ps.direction = dir;
    } else if (it.first == "dir_val") {
      std::stringstream ss(it.second);
      float val;
      ss >> val;

      ps.direction_strength = val;
    } else if (it.first == "radius") {
      std::stringstream ss(it.second);
      float val;
      ss >> val;

      ps.radius = val;
    } else if (it.first == "burst") {
      std::stringstream ss(it.second);
      bool burst;
      ss >> burst;

      ps.burst = burst;
    } else if (it.first == "burst_particles") {
      std::stringstream ss(it.second);
      float val;
      ss >> val;

      ps.burst_particles = val;
    } else if (it.first == "number_of_bursts") {
      std::stringstream ss(it.second);
      int val;
      ss >> val;

      ps.number_of_bursts = val;
    } else if (it.first == "texture") {
      std::string texture;
      std::stringstream ss(it.second);
      ss >> texture;

      auto it = textures.find(texture);
      if (it != textures.end()) {
        ps.texture = it->second;
      }
    } else if (it.first == "shader") {
      std::string shader;
      std::stringstream ss(it.second);
      ss >> shader;

      auto it = compute_shaders.find(shader);
      if (it != compute_shaders.end()) {
        ps.compute_shader = it->second.get();
      }
    }
  }

  if (colors.size() > 0) ps.colors.clear();

  for (auto color : colors) {
    std::stringstream ss(color);
    glm::vec4 col;
    ss >> col.x;
    ss >> col.y;
    ss >> col.z;
    ss >> col.w;

    ps.colors.push_back(col);
  }

  if (color_delta) ps.color_delta = (ps.colors[0] - end_color) / ps.time;
  if (vel_delta) ps.velocity_delta = (ps.velocity - end_vel) / ps.time;
  if (size_delta) ps.size_delta = (ps.size - end_size) / ps.time;

  return ps;
}

ParticleSettings ReadParticleFile(std::string filename) {
  const char *path = "Assets/Particle config";
  std::string directory = std::string(path);
  filename = directory + '/' + filename;
  // Create three strings to hold entire line, key and value
  std::string cur_str;
  std::string key_str;
  std::string val_str;

  // Clear the settings map
  std::unordered_map<std::string, std::string> settings_map;

  // Open a file stream
  std::ifstream settings_file(filename);

  // If the file isn't able to be opened, write error
  if (!settings_file.is_open()) {
    std::cout << "graphics.cpp ReadParticles()\n";
    return {};
  }

  std::vector<std::string> colors;
  // Read through the settings file
  while (settings_file) {
    // Read up to the end of the line. Save as the current line
    std::getline(settings_file, cur_str, '\n');

    // Check if the first char is '>'
    if (!(cur_str.size() == 0) && cur_str.at(0) == '>') {
      // If it is, read the string up the '=' delimiter. That is the key.
      key_str = cur_str.substr(0, cur_str.find('='));

      // Remove the '>' from the key
      key_str.erase(0, 1);

      // Remove the part holding the key from the strong, along with the
      // delimiter
      cur_str.erase(0, cur_str.find('=') + 1);

      // Save the remaining string as the value
      val_str = cur_str;

      // Save what has been read into the map
      settings_map[key_str] = val_str;

      if (key_str == "color") colors.push_back(val_str);
    }
  }

  settings_file.close();

  ParticleSettings ps = {};
  ps.texture = textures["default"];
  ps.compute_shader = compute_shaders["default"].get();

  ps = ProccessMap(ps, settings_map, colors);

  return ps;
}

ParticleSystemHandle CreateParticleSystem() {
  auto handle = current_particle_guid;

  int index = -1;
  for (int i = 0; i < buffer_particle_systems.size(); ++i) {
    if (buffer_particle_systems[i].in_use == false) {
      index = i;
      buffer_particle_systems[i].in_use = true;
      break;
    }
  }

  if (index < 0) {
    index = buffer_particle_systems.size();
    buffer_particle_systems.emplace_back(compute_shaders["default"].get(),
                                         textures["default"], true);
  }

  if (index == -1) return -1;
  particle_systems[handle] = index;
  current_particle_guid++;

  return handle;
}

void DestroyParticleSystem(ParticleSystemHandle handle) {
  auto find_res = particle_systems.find(handle);
  if (find_res == particle_systems.end()) {
    std::cout << "ERROR graphics.cpp: invalid handle\n";
    return;
  }
  int index = find_res->second;
  buffer_particle_systems[index].in_use = false;
  particle_systems.erase(handle);
}

void ResetParticles(ParticleSystemHandle handle) {
  auto find_res = particle_systems.find(handle);
  if (find_res == particle_systems.end()) {
    std::cout << "ERROR graphics.cpp: invalid handle\n";
    return;
  }

  int index = find_res->second;
  buffer_particle_systems[index].system.Reset();
}

void SetEmitPosition(ParticleSystemHandle handle, glm::vec3 pos) {
  auto find_res = particle_systems.find(handle);
  if (find_res == particle_systems.end()) {
    std::cout << "ERROR graphics.cpp: invalid handle\n";
    return;
  }

  int index = find_res->second;
  buffer_particle_systems[index].system.SetPosition(pos);
}

void SetParticleDirection(ParticleSystemHandle handle, glm::vec3 dir) {
  auto find_res = particle_systems.find(handle);
  if (find_res == particle_systems.end()) {
    std::cout << "ERROR graphics.cpp: invalid handle\n";
    return;
  }

  int index = find_res->second;
  buffer_particle_systems[index].system.SetDirection(dir);
}

void SetParticleSettings(ParticleSystemHandle handle,
                         std::unordered_map<std::string, std::string> map) {
  auto find_res = particle_systems.find(handle);
  if (find_res == particle_systems.end()) {
    std::cout << "ERROR graphics.cpp: invalid handle\n";
    return;
  }

  int index = find_res->second;
  ParticleSettings ps = buffer_particle_systems[index].system.GetSettings();
  auto settings = ProccessMap(ps, map, {});

  buffer_particle_systems[index].system.Settings(settings);
}

void SetParticleSettings(ParticleSystemHandle handle, std::string filename) {
  auto find_res = particle_systems.find(handle);
  if (find_res == particle_systems.end()) {
    std::cout << "ERROR graphics.cpp: invalid handle\n";
    return;
  }

  auto settings = ReadParticleFile(filename);

  int index = find_res->second;
  buffer_particle_systems[index].system.Settings(settings);
}

GUIHandle GetGUIItem(const std::string &filepath) {
  return GetAsset<GUIHandle, Elements2D>(gui_handles, gui_elements,
                                         current_gui_guid, filepath);
}

Font2DHandle GetFont(const std::string &filepath) {
  return GetAsset<Font2DHandle, Font2D>(font_2D_handles, fonts,
                                        current_font_guid, filepath);
}

animData GetAnimationData(ModelHandle handle) {
  auto res = models.find(handle);
  animData data;
  if (res == models.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted model\n";
    return data;
  }

  const glob::Model *model = &res->second;

  // copy armature
  for (auto source : model->bones_) {
    glob::Joint j;
    j.id = source->id;
    j.name = source->name;
    j.offset = source->offset;
    j.transform = source->transform;
    j.f_transform = source->f_transform;
    for (auto c : source->children) {
      // std::cout << c << "\n";
      j.children.push_back(c);
    }
    data.bones.push_back(j);
  }

  // copy animations
  for (auto source : model->animations_) {
    glob::Animation a;
    a.name_ = source->name_;
    a.duration_ = source->duration_;
    a.current_frame_time_ = source->current_frame_time_;
    a.tick_per_second_ = source->tick_per_second_;
    a.channels_ = source->channels_;
    a.armature_transform_ = source->armature_transform_;
    data.animations.push_back(a);
  }

  data.globalInverseTransform = model->globalInverseTransform_;

  for (auto bone : data.bones) {
    if (bone.name == "Hip") {
      data.humanoid = true;
      data.hip = bone.id;
      // std::cout << "Hip detected and set...\nHumanoid animation-set
      // loading...\n";
    }
  }

  if (data.humanoid) {
    for (int i = 0; i < data.bones.size(); i++) {
      Joint *bone = &data.bones.at(i);
      if (bone->name == "Spine") {
        data.makeGroup(i, &data.spine);
        // std::cout << "Upper body found!\n";
      } else if (bone->name == "Chest") {
        data.makeGroup(i, &data.upperBody);
        // std::cout << "Left leg found!\n";
      } else if (bone->name == "Leg upper L") {
        data.makeGroup(i, &data.leftLeg);
        // std::cout << "Left leg found!\n";
      } else if (bone->name == "Leg upper R") {
        data.makeGroup(i, &data.rightLeg);
        // std::cout << "Right leg found!\n";
      } else if (bone->name == "Shoulder L") {
        data.makeGroup(i, &data.leftArm);
        // std::cout << "Left arm found!\n";
      } else if (bone->name == "Shoulder R") {
        data.makeGroup(i, &data.rightArm);
        // std::cout << "Right arm found!\n";
      }
    }
    for (int i = 0; i < data.rightArm.size(); i++) {
      data.arms.push_back(data.rightArm.at(i));
    }
    for (int i = 0; i < data.leftArm.size(); i++) {
      data.arms.push_back(data.leftArm.at(i));
    }
    for (int i = 0; i < data.rightLeg.size(); i++) {
      data.legs.push_back(data.rightLeg.at(i));
    }
    for (int i = 0; i < data.leftLeg.size(); i++) {
      data.legs.push_back(data.leftLeg.at(i));
    }
  }

  int num = 0;
  for (auto b : model->bones_) {
    if (b->name == "Armature") {
      break;
    }
    num++;
  }

  data.armatureRoot = num;

  return data;
}
/*
TextureHandle GetTexture(const std::string &filepath) {
  return GetAsset<TextureHandle, Texture>(texture_handles, textures,
                                          current_texture_guid, filepath);
}
*/

MeshData GetMeshData(ModelHandle model_h) {
  auto item = models.find(model_h);

  if (item == models.end()) {
    std::cout
        << "DEBUG graphics.cpp: asset not found trying to get mesh hitbox\n";
    return {};
  }

  auto &model = models[model_h];
  return model.GetMeshData();
}
E2DHandle GetE2DItem(const std::string &filepath) {
  return GetAsset<E2DHandle, Elements2D>(e2D_handles, e2D_elements,
                                         current_e2D_guid, filepath);
}

void UpdateParticles(ParticleSystemHandle handle, float dt) {
  auto find_res = particle_systems.find(handle);
  if (find_res == particle_systems.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted particles\n";
    return;
  }

  int index = find_res->second;
  buffer_particle_systems[index].system.Update(dt);
}
/*
TextureHandle GetTexture(const std::string &filepath) {
return GetAsset<TextureHandle, Texture>(texture_handles, textures,
                                      current_texture_guid, filepath);
}
*/

void SubmitLightSource(glm::vec3 pos, glm::vec3 color, glm::float32 radius,
                       glm::float32 ambient) {
  LightItem item;
  item.pos = pos;
  item.color = color;
  item.radius = radius;
  item.ambient = ambient;
  lights_to_render.push_back(item);
}

void SubmitBAM(const std::vector<ModelHandle> &handles, glm::mat4 transform,
               std::vector<glm::mat4> bone_transforms,
               int material_index) {  // Submit Bone Animated Mesh
  for (auto handle : handles) {
    SubmitBAM(handle, transform, bone_transforms, material_index);
  }
}

void SubmitBAM(ModelHandle model_h, glm::mat4 transform,
               std::vector<glm::mat4> bone_transforms,
               int material_index) {  // Submit Bone Animated Mesh
  BoneAnimatedRenderItem BARI;

  auto find_res = models.find(model_h);
  if (find_res == models.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted model\n";
    return;
  }
  BARI.model = &find_res->second;
  BARI.bone_transforms = bone_transforms;

  const glm::mat4 pre_rotation =
      glm::rotate(glm::pi<float>() / 2.f, glm::vec3(0, 1, 0)) *
      glm::rotate(-glm::pi<float>() / 2.f, glm::vec3(1, 0, 0));

  BARI.transform = transform * pre_rotation;
  BARI.numBones = BARI.bone_transforms.size();

  BARI.material_index = material_index;

  bone_animated_items_to_render.push_back(BARI);
}

void Submit(ModelHandle model_h, glm::vec3 pos, int material_index) {
  glm::mat4 transform = glm::translate(pos);
  Submit(model_h, transform, material_index);
}

void Submit(const std::vector<ModelHandle> &handles, glm::mat4 transform,
            int material_index) {
  for (auto handle : handles) {
    Submit(handle, transform, material_index);
  }
}
void Submit(ModelHandle model_h, glm::mat4 transform, int material_index) {
  auto find_res = models.find(model_h);
  if (find_res == models.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted model\n";
    return;
  }

  const glm::mat4 pre_rotation =
      glm::rotate(glm::pi<float>() / 2.f, glm::vec3(0, 1, 0)) *
      glm::rotate(-glm::pi<float>() / 2.f, glm::vec3(1, 0, 0));

  RenderItem to_render;
  to_render.model = &find_res->second;
  to_render.transform = transform * pre_rotation;
  to_render.material_index = material_index;

  items_to_render.push_back(to_render);
}

void SubmitParticles(ParticleSystemHandle handle) {
  auto find_res = particle_systems.find(handle);
  if (find_res == particle_systems.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted particles\n";
    return;
  }

  particles_to_render.push_back(find_res->second);
}

double GetWidthOfText(Font2DHandle font_handle, std::string text, float size) {
  const char *chars = text.c_str();
  int len = text.length();

  //////////////////////////////////
  // for backwards compatibility
  size *= 16. / 28.;
  //////////////////////////////////

  double offset_accum = 0;
  for (int i = 0; i < len; i++) {
    unsigned char cur = *(unsigned char *)(chars + i);

    offset_accum += fonts[font_handle].GetAdvance(cur, size);

    // std::cout << offset_accum << "\n";
  }
  return (offset_accum - 0.7 * len + 4.) * 93. / 97.;
}

double GetWidthOfChatText(Font2DHandle font_handle, std::string text,
                          float size) {
  const char *chars = text.c_str();
  int len = text.length();

  //////////////////////////////////
  // for backwards compatibility
  size *= 16. / 28.;
  //////////////////////////////////

  double offset_accum = 0;
  for (int i = 0; i < len; i++) {
    unsigned char cur = *(unsigned char *)(chars + i);

    offset_accum += fonts[font_handle].GetAdvance(cur, size);

    // std::cout << offset_accum << "\n";
  }
  return offset_accum;
}

void Submit(Font2DHandle font_h, glm::vec2 pos, unsigned int size,
            std::string text, glm::vec4 color, bool visible, bool equal_spacing,
            float spacing) {
  auto find_res = fonts.find(font_h);
  if (find_res == fonts.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted font! \n";
    return;
  }

  TextItem to_render;
  to_render.font = &find_res->second;
  to_render.pos = pos;
  to_render.size = size;
  to_render.text = text;
  to_render.color = color;
  to_render.visible = visible;
  to_render.equal_spacing = equal_spacing;
  to_render.spacing = spacing;
  text_to_render.push_back(to_render);
}

void Submit(Font2DHandle font_h, glm::vec3 pos, float size, std::string text,
            glm::vec4 color, glm::mat4 rot) {
  auto find_res = fonts.find(font_h);
  if (find_res == fonts.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted font! \n";
    return;
  }

  Text3DItem to_render;
  to_render.font = &find_res->second;
  to_render.pos = pos;
  to_render.size = size;
  to_render.text = text;
  to_render.color = color;
  to_render.rotation = rot;
  text3D_to_render.push_back(to_render);
}

void SetCamera(Camera cam) { camera = cam; }

void SetModelUseGL(bool use_gl) { kModelUseGL = use_gl; }

void SetSSAO(bool val) { use_ao = val; }

void SetInvisibleEffect(bool in_bool) { is_invisible = (GLint)in_bool; }

void SetBlackout(bool blackout) {
  if (blackout) {
    shadows.SetNumUsed(0);
  } else {
    shadows.SetNumUsed(4);
  }
}

void SetSky(const std::string &file) {
  if (sky_texture != 0) {
    glDeleteTextures(1, &sky_texture);
    sky_texture = 0;
  }
  std::vector<unsigned char> image;
  unsigned width, height;
  unsigned error = lodepng::decode(image, width, height, file, LCT_RGB);
  if (error != 0) {
    std::cout << "ERROR: Could not load sky texture: " << file << "\n";
    return;
  }
  glGenTextures(1, &sky_texture);
  // Set some parameters for the texture
  glBindTexture(GL_TEXTURE_2D, sky_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, image.data());
}

void ReloadShaders() {
  fullscreen_shader.reload();
  model_shader.reload();
  particle_shader.reload();
  animated_model_shader.reload();
  text_shader.reload();
  text3D_shader.reload();
  wireframe_shader.reload();
  gui_shader.reload();
  e2D_shader.reload();
}

void Submit(GUIHandle gui_h, glm::vec2 pos, float scale, float scale_x,
            float opacity) {
  auto find_res = gui_elements.find(gui_h);
  if (find_res == gui_elements.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted gui element\n";
    return;
  }

  GUIItem to_render;
  to_render.gui = &find_res->second;
  to_render.pos = pos;
  to_render.scale = scale;
  to_render.scale_x = scale_x;
  to_render.opacity = opacity;
  gui_items_to_render.push_back(to_render);
}

void Submit(E2DHandle e2D_h, glm::vec3 pos, float scale, float rotDegrees,
            glm::vec3 rotAxis) {
  auto find_res = e2D_elements.find(e2D_h);
  if (find_res == e2D_elements.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted e2D item\n";
    return;
  }

  E2DItem to_render;
  to_render.e2D = &find_res->second;
  to_render.pos = pos;
  to_render.scale = scale;
  to_render.rot = glm::rotate(glm::radians(rotDegrees), rotAxis);
  e2D_items_to_render.push_back(to_render);
}

void Submit(E2DHandle e2D_h, glm::vec3 pos, glm::mat4 matrix) {
  auto find_res = e2D_elements.find(e2D_h);
  if (find_res == e2D_elements.end()) {
    std::cout << "ERROR graphics.cpp: could not find submitted e2D item\n";
    return;
  }

  E2DItem to_render;
  to_render.e2D = &find_res->second;
  to_render.pos = pos;
  to_render.scale = 1.f;
  to_render.rot = matrix;
  e2D_items_to_render.push_back(to_render);
}

void SubmitTrail(const std::vector<glm::vec3> &pos_history, float width,
                 glm::vec4 color) {
  trails_to_render.push_back({pos_history, width, color});
}

void SubmitCube(glm::mat4 t) { cubes.push_back(t); }

void SubmitWireframeMesh(ModelHandle model_h) {
  wireframe_meshes.push_back(model_h);
}

void LoadWireframeMesh(ModelHandle model_h,
                       const std::vector<glm::vec3> &vertices,
                       const std::vector<unsigned int> &indices) {
  auto find_res = wireframe_buffers.find(model_h);
  if (find_res == wireframe_buffers.end()) {
    GLuintBuffers b;
    b.size = indices.size();
    /*---------------Generate needed buffers--------------*/
    glGenVertexArrays(1, &b.vao);
    glGenBuffers(1, &b.vbo);
    glGenBuffers(1, &b.ebo);

    /*---------------Binding vertex buffer---------------*/
    glBindVertexArray(b.vao);
    glBindBuffer(GL_ARRAY_BUFFER, b.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
                 &vertices[0], GL_STATIC_DRAW);

    /*---------------Binding element buffer--------------*/
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                 &indices[0], GL_STATIC_DRAW);

    /*---------------Enable arrays----------------------*/
    glEnableVertexAttribArray(0);  // Layout 0 for vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (GLvoid *)0);

    wireframe_buffers[model_h] = b;
  }
}

void Render() {
  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  // render models and light
  std::vector<glm::vec3> light_positions;
  std::vector<glm::vec3> light_colors;
  std::vector<float> light_radii;
  std::vector<float> light_ambients;
  for (auto &light_item : lights_to_render) {
    light_positions.push_back(light_item.pos);
    light_colors.push_back(light_item.color);
    light_radii.push_back(light_item.radius);
    light_ambients.push_back(light_item.ambient);
  }

  std::vector<RenderItem> normal_items;
  std::map<float, std::vector<RenderItem>> transparent_items;
  for (auto &render_item : items_to_render) {
    if (render_item.model->IsTransparent()) {
      float max_dist = render_item.model->MaxDistance(render_item.transform,
                                                      camera.GetPosition());
      transparent_items[-max_dist].push_back(render_item);
    } else {
      normal_items.push_back(render_item);
    }
  }

  auto draw_function = [&](ShaderProgram &shader) {
    for (auto &render_item : items_to_render) {
      shader.uniform("model_transform", render_item.transform);
      render_item.model->Draw(shader);
    }
  };
  auto anim_draw_function = [&](ShaderProgram &shader) {
    for (auto &BARI : bone_animated_items_to_render) {
      shader.uniform("model_transform", BARI.transform);
      int numBones = 0;
      for (auto &bone : BARI.bone_transforms) {
        shader.uniform("bone_transform[" + std::to_string(numBones) + "]",
                       bone);
        numBones++;
      }
      // animated_model_shader.uniform("NR_OF_BONES",
      // (int)BARI.bone_transforms.size());
      BARI.model->Draw(animated_model_shader);
    }
  };
  shadows.RenderToMaps(draw_function, anim_draw_function, blur);
  shadows.BindMaps(TEXTURE_SLOT_SHADOWS);

  for (auto &shader : mesh_render_group) {
    shader->use();
    for (auto &light_item : lights_to_render) {
      shader->uniformv("light_pos", lights_to_render.size(),
                       light_positions.data());
      shader->uniformv("light_col", lights_to_render.size(),
                       light_colors.data());
      shader->uniformv("light_radius", lights_to_render.size(),
                       light_radii.data());
      shader->uniformv("light_amb", lights_to_render.size(),
                       light_ambients.data());
    }
    shader->uniform("NR_OF_LIGHTS", (int)lights_to_render.size());
    shader->uniform("cam_transform", cam_transform);
    shader->uniform("cam_position", camera.GetPosition());
    shadows.SetUniforms(*shader);
  }

  auto ws = glob::window::GetWindowDimensions();
  glViewport(0, 0, ws.x, ws.y);
  post_process.BeforeDraw();
  {
    model_shader.use();
    for (auto &render_item : normal_items) {
      SetDefaultMaterials(model_shader);
      model_shader.uniform("model_transform", render_item.transform);
      render_item.model->Draw(model_shader);
    }

    // render bone animated items
    animated_model_shader.use();
    for (auto &BARI : bone_animated_items_to_render) {
      animated_model_shader.uniform("diffuse_index", BARI.material_index);
      animated_model_shader.uniform("model_transform", BARI.transform);
      int numBones = 0;
      for (auto &bone : BARI.bone_transforms) {
        animated_model_shader.uniform(
            "bone_transform[" + std::to_string(numBones) + "]", bone);
        numBones++;
      }
      SetDefaultMaterials(animated_model_shader);
      BARI.model->Draw(animated_model_shader);
    }

    // render wireframe cubes
    for (auto &m : cubes) DrawCube(m);
    // render wireframe meshes
    for (auto &m : wireframe_meshes) DrawWireFrameMeshes(m);

    // render gui elements
    glBindVertexArray(quad_vao);
    // render 2D elements
    e2D_shader.use();
    e2D_shader.uniform("cam_transform", cam_transform);
    for (auto &e2D_item : e2D_items_to_render) {
      e2D_item.e2D->DrawInWorld(e2D_shader, e2D_item.pos, e2D_item.scale,
                                e2D_item.rot);
    }

    // render particles
    particle_shader.use();
    particle_shader.uniform("cam_transform", cam_transform);
    particle_shader.uniform("cam_pos", camera.GetPosition());
    particle_shader.uniform("cam_up", camera.GetUpVector());
    for (auto p : particles_to_render) {
      buffer_particle_systems[p].system.Draw(particle_shader);
    }
    // draw sky
    sky_shader.use();
    glm::mat4 view = glm::mat3(camera.GetViewMatrix());
    sky_shader.uniform("view", view);
    sky_shader.uniform("projection", camera.GetProjectionMatrix());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sky_texture);
    sky_shader.uniform("texture_sky", 0);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDepthFunc(GL_LESS);

    // TODO: Sort all transparent triangles
    // maybe sort internally in modell and then and externally
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    model_shader.use();
    for (auto &[dist, render_items] : transparent_items) {
      for (auto &render_item : render_items) {
        SetDefaultMaterials(model_shader);
        model_shader.uniform("model_transform", render_item.transform);
        render_item.model->Draw(model_shader);
      }
    }
    glDisable(GL_BLEND);

    // render text
    glBindVertexArray(quad_vao);
    text3D_shader.use();
    text3D_shader.uniform("cam_transform", cam_transform);
    for (auto &text3D : text3D_to_render) {
      text3D.font->Draw3D(text3D_shader, text3D.pos, text3D.size, text3D.text,
                          text3D.color, text3D.rotation);
    }


    trail_shader.use();
    trail_shader.uniform("cam_transform", cam_transform);
    trail_shader.uniform("cam_pos", camera.GetPosition());
    glBindVertexArray(trail_vao);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (auto &trail_item : trails_to_render) {
      trail_shader.uniform("width", trail_item.width);
      trail_shader.uniform("color", trail_item.color);
      int max_positions = 100;
      auto &ph = trail_item.position_history;
      int history_size = glm::min((int)ph.size(), max_positions);
      trail_shader.uniformv("position_history", history_size, ph.data());
      trail_shader.uniform("history_size", history_size);
      glDrawArrays(GL_TRIANGLES, 0, num_trail_quads * 6);
    }
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
  }
  post_process.AfterDraw(blur);

  if (use_ao) {
    ssao.BindFrameBuffer();
    ssao_shader.use();
    post_process.BindDepthTex(0);
    ssao_shader.uniform("texture_depth", 0);
    post_process.BindNormalTex(1);
    ssao_shader.uniform("texture_normals", 1);
    ssao.BindNoiseTexture(2);
    ssao_shader.uniform("texture_noise", 2);
    post_process.BindPositionTex(3);
    ssao_shader.uniform("texture_position", 3);
    ssao_shader.uniformv("samples", (GLuint)ssao.GetKernel().size(),
                         ssao.GetKernel().data());
    ssao_shader.uniform("projection", cam_transform);
    ssao_shader.uniform("inv_projection", inverse(cam_transform));
    ssao_shader.uniform("screen_dims", window::GetWindowDimensions());

    DrawFullscreenQuad();  // do ssao pass same way we do final color pass
    ssao.Finish(blur);
  }

  fullscreen_shader.use();
  fullscreen_shader.uniform("is_invisible", is_invisible);
  post_process.BindColorTex(0);
  fullscreen_shader.uniform("texture_color", 0);
  post_process.BindEmissionTex(1);
  fullscreen_shader.uniform("texture_emission", 1);
  ssao.BindSsaoTexture(2);
  fullscreen_shader.uniform("texture_ssao", 2);
  fullscreen_shader.uniform("use_ao", use_ao);
  DrawFullscreenQuad();

  glBindVertexArray(quad_vao);
  gui_shader.use();
  for (auto &gui_item : gui_items_to_render) {
    gui_item.gui->DrawOnScreen(gui_shader, gui_item.pos, gui_item.scale,
                               gui_item.scale_x, gui_item.opacity);
  }

  text_shader.use();
  for (auto &text_item : text_to_render) {
    text_item.font->Draw(text_shader, text_item.pos, text_item.size,
                         text_item.text, text_item.color, text_item.visible,
                         text_item.equal_spacing, text_item.spacing);
  }

  trails_to_render.clear();
  lights_to_render.clear();
  items_to_render.clear();
  bone_animated_items_to_render.clear();
  e2D_items_to_render.clear();
  text3D_to_render.clear();
  gui_items_to_render.clear();
  text_to_render.clear();
  cubes.clear();
  wireframe_meshes.clear();
  particles_to_render.clear();
}

Camera &GetCamera() { return camera; }

}  // namespace glob