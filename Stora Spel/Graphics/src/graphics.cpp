#include "glob/graphics.hpp"

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <lodepng.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "Model/model.hpp"
#include "glob/camera.hpp"
#include "Particles/particle_settings.hpp"
#include "particles/particle_system.hpp"
#include "shader.hpp"

#include "2D/elements2D.hpp"
#include "Font/Font2D.hpp"

#include <msdfgen/msdfgen-ext.h>
#include <msdfgen/msdfgen.h>

namespace glob {

bool kModelUseGL = true;

namespace {

struct RenderItem {
  Model *model;
  glm::mat4 transform;
};

struct GUIItem {
  Elements2D *gui;
  glm::vec2 pos;
  float scale;
  float scale_x;
};

struct E2DItem {
  Elements2D *e2D;
  glm::vec3 pos;
  float scale;
  glm::mat4 rot;
};

struct TextItem {
  Font2D *font;
  glm::vec2 pos;
  unsigned int size;
  std::string text;
  glm::vec4 color;
};

struct LightItem {
  glm::vec3 pos;
  glm::vec3 color;
  glm::float32 radius;
  glm::float32 ambient;
};

ShaderProgram test_shader;
ShaderProgram model_shader;
ShaderProgram particle_shader;
ShaderProgram compute_shader;
ShaderProgram text_shader;
ShaderProgram wireframe_shader;
ShaderProgram gui_shader;
ShaderProgram e2D_shader;

GLuint triangle_vbo, triangle_vao;
GLuint cube_vbo, cube_vao;
GLuint quad_vbo, quad_vao;

float num_frames = 0;

Camera camera{
    glm::vec3(25, 5, 0), glm::vec3(0, 3, 0), 90, 16.f / 9.f, 0.1f, 100.f};

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

struct ParticleSystemInfo {
  ParticleSystem system;
  bool in_use;

  ParticleSystemInfo(ShaderProgram *ptr, GLuint tex, bool use) : system(ptr, tex), in_use(use) {}
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
std::vector<glm::mat4> cubes;
std::vector<ModelHandle> wireframe_meshes;
std::vector<TextItem> text_to_render;
std::vector<GUIItem> gui_items_to_render;
std::vector<E2DItem> e2D_items_to_render;

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
}

GLint TextureFromFile(std::string filename) {
  const char *path = "Assets"; 
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
  test_shader.add("testshader.frag");
  test_shader.add("testshader.vert");
  test_shader.compile();

  model_shader.add("modelshader.vert");
  model_shader.add("modelshader.frag");
  model_shader.compile();

  particle_shader.add("particle.vert");
  particle_shader.add("particle.geom");
  particle_shader.add("particle.frag");
  particle_shader.compile();

  compute_shader.add("compute_shader.comp");
  compute_shader.compile();

  CreateDefaultParticleTexture();
  textures["smoke"] = TextureFromFile("smoke.png");

  wireframe_shader.add("modelshader.vert");
  wireframe_shader.add("wireframe.frag");
  wireframe_shader.compile();

  text_shader.add("text2Dshader.vert");
  text_shader.add("text2Dshader.frag");
  text_shader.compile();

  gui_shader.add("guishader.vert");
  gui_shader.add("guishader.frag");
  gui_shader.compile();

  e2D_shader.add("e2Dshader.vert");
  e2D_shader.add("e2Dshader.frag");
  e2D_shader.compile();

  glGenVertexArrays(1, &triangle_vao);
  glBindVertexArray(triangle_vao);
  std::vector<glm::vec3> vertices{
      {0, 1, 1},
      {0, -1, 1},
      {0, -1, -1},
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

  //buffer_particle_systems.reserve(1);
  //for (int i = 0; i < 1; ++i) {
  //  buffer_particle_systems.emplace_back();
  //  buffer_particle_systems[i].second = false;
  //}
}

// H=Handle, A=Asset
template <class H, class A>
H GetAsset(std::unordered_map<std::string, H> &handles,
           std::unordered_map<H, A> &assets, H &guid,
           const std::string filepath) {
  H result = 0;

  auto item = handles.find(filepath);
  if (item == handles.end()) {
    std::cout << "DEBUG graphics.cpp: Loading asset '" << filepath << "'\n";
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
    std::cout << "DEBUG graphics.cpp: Asset '" << filepath
              << "' already loaded\n";
    result = item->second;
  }

  return result;
}

ModelHandle GetModel(const std::string &filepath) {
  return GetAsset<ModelHandle, Model>(model_handles, models, current_model_guid,
                                      filepath);
}

std::pair<ParticleSettings, std::string> ReadParticleFile(std::string filename) {
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
    }
  }

  settings_file.close();

  ParticleSettings ps = {};
  std::string texture = "default";
  for (auto it : settings_map) {
    if (it.first == "color") {
      std::stringstream ss(it.second);
      glm::vec4 col;
      ss >> col.x;
      ss >> col.y;
      ss >> col.z;
      ss >> col.w;

      ps.color = col;
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
    } else if (it.first == "texture") {
      std::stringstream ss(it.second);
      ss >> texture;
    }
  }

  return {ps, texture};
}

ParticleSystemHandle CreateParticleSystem() {
  auto handle = current_particle_guid;

  int index = -1;
  for (int i = 0; i < buffer_particle_systems.size(); ++i) {
    if (buffer_particle_systems[i].in_use == false) {
      index = i;
      buffer_particle_systems[i].in_use = true;
    }
  }
  
  if (index < 0) {
    index = buffer_particle_systems.size();
    buffer_particle_systems.emplace_back(&compute_shader, textures["smoke"], true);
    //buffer_particle_systems[index].in_use = true;
  }

  if (index == -1) return -1;
  particle_systems[handle] = index;
  current_particle_guid++;
  
  return handle;
}

void SetParticleSettings(ParticleSystemHandle handle, std::string filename) {
  auto find_res = particle_systems.find(handle);
  if (find_res == particle_systems.end()) {
    std::cout << "ERROR graphics.cpp: invalid handle\n";
    return;
  }

  auto settings = ReadParticleFile(filename);
  auto texture = textures[settings.second];

  int index = find_res->second;
  buffer_particle_systems[index].system.Settings(settings.first);
  buffer_particle_systems[index].system.SetTexture(texture);
}

GUIHandle GetGUIItem(const std::string &filepath) {
  return GetAsset<GUIHandle, Elements2D>(gui_handles, gui_elements,
                                         current_gui_guid, filepath);
}

Font2DHandle GetFont(const std::string &filepath) {
  return GetAsset<Font2DHandle, Font2D>(font_2D_handles, fonts,
                                        current_font_guid, filepath);
}

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

void Submit(ModelHandle model_h, glm::vec3 pos) {
  glm::mat4 transform = glm::translate(pos);
  Submit(model_h, transform);
}

void Submit(ModelHandle model_h, glm::mat4 transform) {
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

void Submit(Font2DHandle font_h, glm::vec2 pos, unsigned int size,
            std::string text, glm::vec4 color) {
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
  text_to_render.push_back(to_render);
}

void SetCamera(Camera cam) { camera = cam; }

void SetModelUseGL(bool use_gl) {
  std::cout << "Before " << kModelUseGL << "\n";
  kModelUseGL = use_gl;
  std::cout << "after " << kModelUseGL << "\n";
}

void Submit(GUIHandle gui_h, glm::vec2 pos, float scale, float scale_x) {
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
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  // render models and light
  model_shader.use();

  int lightNR = 0;
  for (auto &light_item : lights_to_render) {
    model_shader.uniform("light_pos[" + std::to_string(lightNR) + "]",
                         light_item.pos);
    model_shader.uniform("light_col[" + std::to_string(lightNR) + "]",
                         light_item.color);
    model_shader.uniform("light_radius[" + std::to_string(lightNR) + "]",
                         light_item.radius);
    model_shader.uniform("light_amb[" + std::to_string(lightNR) + "]",
                         light_item.ambient);
    lightNR++;
  }
  model_shader.uniform("NR_OF_LIGHTS", (int)lights_to_render.size());

  model_shader.uniform("cam_transform", cam_transform);
  // model_shader.uniform("num_frames", num_frames);
  for (auto &render_item : items_to_render) {
    model_shader.uniform("model_transform", render_item.transform);
    render_item.model->Draw(model_shader);
  }


  // render wireframe cubes
  for (auto &m : cubes) DrawCube(m);
  // render wireframe meshes
  for (auto &m : wireframe_meshes) DrawWireFrameMeshes(m);

  // render text and gui elements
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
    buffer_particle_systems[p].system.Draw(particle_shader, camera);
  }

  glBindVertexArray(quad_vao);
  gui_shader.use();
  for (auto &gui_item : gui_items_to_render) {
    gui_item.gui->DrawOnScreen(gui_shader, gui_item.pos, gui_item.scale,
                               gui_item.scale_x);
  }

  text_shader.use();
  for (auto &text_item : text_to_render) {
    text_item.font->Draw(text_shader, text_item.pos, text_item.size,
                         text_item.text, text_item.color);
  }

  lights_to_render.clear();
  items_to_render.clear();
  e2D_items_to_render.clear();
  gui_items_to_render.clear();
  text_to_render.clear();
  cubes.clear();
  wireframe_meshes.clear();
  particles_to_render.clear();

  num_frames++;
}

Camera GetCamera() { return camera; }

}  // namespace glob