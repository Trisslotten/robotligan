#include "glob/graphics.hpp"

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <unordered_map>

#include "Model/model.hpp"
#include "glob/camera.hpp"
#include "glob/window.hpp"
#include "shader.hpp"

#include "2D/elements2D.hpp"
#include "Font/Font2D.hpp"

#include <msdfgen/msdfgen-ext.h>
#include <msdfgen/msdfgen.h>
#include "postprocess/blur.hpp"
#include "postprocess/postprocess.hpp"

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
  Font2D *font = nullptr;
  glm::vec2 pos{0};
  unsigned int size = 0;
  std::string text;
  glm::vec4 color;
  bool visible;
};

struct LightItem {
  glm::vec3 pos;
  glm::vec3 color;
  glm::float32 radius;
  glm::float32 ambient;
};

ShaderProgram fullscreen_shader;
ShaderProgram model_emission_shader;
ShaderProgram model_shader;
ShaderProgram text_shader;
ShaderProgram wireframe_shader;
ShaderProgram gui_shader;
ShaderProgram e2D_shader;

GLuint triangle_vbo, triangle_vao;
GLuint cube_vbo, cube_vao;
GLuint quad_vbo, quad_vao;

const int num_shadow_maps = 4;
GLuint shadow_framebuffer;
GLuint shadow_renderbuffer;
GLuint shadow_texture;
GLuint shadow_blurred_textures[num_shadow_maps] = {0};
int shadow_size = 1024;
int shadow_blurred_level = 1;
int shadow_blurred_size = shadow_size / glm::pow(2, shadow_blurred_level);
ShaderProgram shadow_shader;
uint64_t shadow_blur_id = 0;

PostProcess post_process;
Blur blur;

float num_frames = 0;

Camera camera{
    glm::vec3(25, 5, 0), glm::vec3(0, 3, 0), 90, 16.f / 9.f, 0.1f, 100.f};

/*
TextureHandle current_texture_guid = 1;
std::unordered_map<std::string, TextureHandle> texture_handles;
std::unordered_map<TextureHandle, Texture> textures;
*/

ModelHandle current_model_guid = 1;
Font2DHandle current_font_guid = 1;
GUIHandle current_gui_guid = 1;
E2DHandle current_e2D_guid = 1;
std::unordered_map<std::string, ModelHandle> model_handles;
std::unordered_map<ModelHandle, Model> models;
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

void Init() {
  fullscreen_shader.add("fullscreenquad.vert");
  fullscreen_shader.add("fullscreenquad.frag");
  fullscreen_shader.compile();

  model_shader.add("modelshader.vert");
  model_shader.add("modelshader.frag");
  model_shader.add("shading.vert");
  model_shader.add("shading.frag");
  model_shader.compile();

  model_emission_shader.add("modelshader.vert");
  model_emission_shader.add("modelemissive.frag");
  model_emission_shader.add("shading.vert");
  model_emission_shader.add("shading.frag");
  model_emission_shader.compile();

  wireframe_shader.add("modelshader.vert");
  wireframe_shader.add("shading.vert");
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

  blur.Init();

  shadow_blur_id =
      blur.CreatePass(shadow_blurred_size, shadow_blurred_size, GL_RG32F);
  post_process.Init(blur);

  shadow_shader.add("modelshader.vert");
  shadow_shader.add("shading.vert");
  shadow_shader.add("shadow.frag");
  shadow_shader.compile();

  glGenFramebuffers(1, &shadow_framebuffer);
  glGenRenderbuffers(1, &shadow_renderbuffer);

  glGenTextures(1, &shadow_texture);
  glBindTexture(GL_TEXTURE_2D, shadow_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, shadow_size, shadow_size, 0, GL_RG,
               GL_UNSIGNED_BYTE, NULL);

  glGenTextures(num_shadow_maps, shadow_blurred_textures);
  for (int i = 0; i < num_shadow_maps; i++) {
    glBindTexture(GL_TEXTURE_2D, shadow_blurred_textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, shadow_blurred_size,
                 shadow_blurred_size, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer);

  glBindRenderbuffer(GL_RENDERBUFFER, shadow_renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, shadow_size,
                        shadow_size);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         shadow_texture, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, shadow_renderbuffer);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR: graphics.cpp: shadow_framebuffer is not complete!"
              << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    /*
    std::cout << "DEBUG graphics.cpp: Asset '" << filepath
              << "' already loaded\n";
    */
    result = item->second;
  }

  return result;
}

ModelHandle GetModel(const std::string &filepath) {
  return GetAsset<ModelHandle, Model>(model_handles, models, current_model_guid,
                                      filepath);
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

void Submit(Font2DHandle font_h, glm::vec2 pos, unsigned int size,
            std::string text, glm::vec4 color, bool visible) {
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
  std::vector<RenderItem> emissive_items;
  for (auto &render_item : items_to_render) {
    if (render_item.model->IsEmissive()) {
      emissive_items.push_back(render_item);
    }
  }

  std::vector<std::pair<glm::vec3, glm::mat4>> shadow_casters;
  shadow_casters.push_back(std::make_pair(
      glm::vec3(10.6, 5.7, 7.1),
      glm::perspective(glm::radians(70.f), 1.f, 0.1f, 50.f) *
          glm::lookAt(glm::vec3(10.6, 5.7, 7.1), glm::vec3(0, -5.7, 0),
                      glm::vec3(0, 1, 0))));

  shadow_casters.push_back(std::make_pair(
      glm::vec3(-10.6, 5.7, -7.1),
      glm::perspective(glm::radians(70.f), 1.f, 0.1f, 50.f) *
          glm::lookAt(glm::vec3(-10.6, 5.7, -7.1), glm::vec3(0, -5.7, 0),
                      glm::vec3(0, 1, 0))));

  shadow_casters.push_back(std::make_pair(
      glm::vec3(10.6, 5.7, -7.1),
      glm::perspective(glm::radians(70.f), 1.f, 0.1f, 50.f) *
          glm::lookAt(glm::vec3(10.6, 5.7, -7.1), glm::vec3(0, -5.7, 0),
                      glm::vec3(0, 1, 0))));

  shadow_casters.push_back(std::make_pair(
      glm::vec3(-10.6, 5.7, 7.1),
      glm::perspective(glm::radians(70.f), 1.f, 0.1f, 50.f) *
          glm::lookAt(glm::vec3(-10.6, 5.7, 7.1), glm::vec3(0, -5.7, 0),
                      glm::vec3(0, 1, 0))));

  glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer);
  glViewport(0, 0, shadow_size, shadow_size);

  for (int i = 0; i < shadow_casters.size(); i++) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::vec3 shadow_light_pos = shadow_casters[i].first;
    glm::mat4 shadow_transform = shadow_casters[i].second;

    shadow_shader.use();
    shadow_shader.uniform("cam_transform", shadow_transform);
    shadow_shader.uniform("shadow_light_pos", shadow_light_pos);
    for (auto &render_item : items_to_render) {
      shadow_shader.uniform("model_transform", render_item.transform);
      render_item.model->Draw(shadow_shader);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow_texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    blur.BlurTexture(shadow_blur_id, shadow_texture, 1,
                     shadow_blurred_textures[i]);

    model_shader.use();
    model_shader.uniform("shadow_transforms[" + std::to_string(i) + "]",
                         shadow_transform);
    model_shader.uniform("shadow_light_positions[" + std::to_string(i) + "]",
                         shadow_light_pos);
    model_shader.uniform("shadow_maps[" + std::to_string(i) + "]", 3 + i);

    model_emission_shader.use();
    model_emission_shader.uniform(
        "shadow_transforms[" + std::to_string(i) + "]", shadow_transform);
    model_emission_shader.uniform(
        "shadow_light_positions[" + std::to_string(i) + "]", shadow_light_pos);
    model_emission_shader.uniform("shadow_maps[" + std::to_string(i) + "]",
                                  3 + i);
  }

  for (int i = 0; i < shadow_casters.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + 3 + i);
    glBindTexture(GL_TEXTURE_2D, shadow_blurred_textures[i]);
  }

  auto ws = glob::window::GetWindowDimensions();
  glViewport(0, 0, ws.x, ws.y);
  post_process.BeforeDraw();
  {
    model_shader.use();
    model_shader.uniform("num_shadows", (int)shadow_casters.size());
    model_shader.uniformv("light_pos", lights_to_render.size(),
                          light_positions.data());
    model_shader.uniformv("light_col", lights_to_render.size(),
                          light_colors.data());
    model_shader.uniformv("light_radius", lights_to_render.size(),
                          light_radii.data());
    model_shader.uniformv("light_amb", lights_to_render.size(),
                          light_ambients.data());
    model_shader.uniform("NR_OF_LIGHTS", (int)lights_to_render.size());
    model_shader.uniform("cam_transform", cam_transform);
    for (auto &render_item : items_to_render) {
      if (render_item.model->IsEmissive()) {
        continue;
      }
      model_shader.uniform("model_transform", render_item.transform);
      render_item.model->Draw(model_shader);
    }

    model_emission_shader.use();
    model_emission_shader.uniform("num_shadows", (int)shadow_casters.size());
    model_emission_shader.uniformv("light_pos", lights_to_render.size(),
                                   light_positions.data());
    model_emission_shader.uniformv("light_col", lights_to_render.size(),
                                   light_colors.data());
    model_emission_shader.uniformv("light_radius", lights_to_render.size(),
                                   light_radii.data());
    model_emission_shader.uniformv("light_amb", lights_to_render.size(),
                                   light_ambients.data());
    model_emission_shader.uniform("NR_OF_LIGHTS", (int)lights_to_render.size());
    model_emission_shader.uniform("cam_transform", cam_transform);
    for (auto &render_item : emissive_items) {
      model_emission_shader.uniform("model_transform", render_item.transform);
      render_item.model->Draw(model_emission_shader);
    }
  }
  post_process.AfterDraw(blur);

  fullscreen_shader.use();
  post_process.BindColorTex(0);
  fullscreen_shader.uniform("texture_color", 0);
  post_process.BindEmissionTex(1);
  fullscreen_shader.uniform("texture_emission", 1);
  DrawFullscreenQuad();

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

  gui_shader.use();
  for (auto &gui_item : gui_items_to_render) {
    gui_item.gui->DrawOnScreen(gui_shader, gui_item.pos, gui_item.scale,
                               gui_item.scale_x);
  }

  text_shader.use();
  for (auto &text_item : text_to_render) {
    text_item.font->Draw(text_shader, text_item.pos, text_item.size,
                         text_item.text, text_item.color, text_item.visible);
  }

  lights_to_render.clear();
  items_to_render.clear();
  e2D_items_to_render.clear();
  gui_items_to_render.clear();
  text_to_render.clear();
  cubes.clear();
  wireframe_meshes.clear();

  num_frames++;
}

Camera GetCamera() { return camera; }

}  // namespace glob