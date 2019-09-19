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
#include "shader.hpp"

#include "Font/Font2D.hpp"

#include <msdfgen/msdfgen-ext.h>
#include <msdfgen/msdfgen.h>

namespace glob {
namespace {

struct RenderItem {
  Model *model;
  glm::mat4 transform;
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
ShaderProgram text_shader;
ShaderProgram wireframe_shader;

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
Font2DHandle current_font_guid = 1;
std::unordered_map<std::string, ModelHandle> model_handles;
std::unordered_map<ModelHandle, Model> models;
std::unordered_map<std::string, Font2DHandle> font_2D_handles;
std::unordered_map<Font2DHandle, Font2D> fonts;

std::vector<RenderItem> items_to_render;
std::vector<LightItem> lights_to_render;
std::vector<glm::mat4> cubes;
std::vector<TextItem> text_to_render;

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
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  glBindVertexArray(0);
}

void Init() {
  test_shader.add("testshader.frag");
  test_shader.add("testshader.vert");
  test_shader.compile();

  model_shader.add("modelshader.vert");
  model_shader.add("modelshader.frag");
  model_shader.compile();

  wireframe_shader.add("modelshader.vert");
  wireframe_shader.add("wireframe.frag");
  wireframe_shader.compile();

  text_shader.add("text2Dshader.vert");
  text_shader.add("text2Dshader.frag");
  text_shader.compile();

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
  std::vector<glm::vec3> vertices_cube {
      {1, 1, 1}, {1, -1, 1}, {-1, 1, 1}, // z+ face
      {-1, -1, 1}, {1, -1, 1},  {-1, 1, 1},

      {1, 1, -1}, {1, -1, -1}, {1, -1, 1}, // x+ face
      {1, 1, -1},  {1, 1, 1},  {1, -1, 1},

      {-1, 1, -1}, {1, 1, -1},  {1, 1, 1}, // y+ face
      {-1, 1, -1},  {1, 1, 1},    {-1, 1, 1},

      {-1, -1, -1}, {1, 1, -1},  {-1, 1, -1}, // z- face
      {1, 1, -1},   {-1, -1, -1}, {1, -1, -1},

      {-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, // y- face
      {-1, -1, 1},  {1, -1, 1},  {1, -1, -1},

      {-1, -1, -1}, {-1, -1, 1}, {-1, 1, -1}, // x- face
      {-1, -1, 1},  {-1, 1, -1}, {-1, 1, 1}
  };
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
Font2DHandle GetFont(const std::string &filepath) {
  return GetAsset<Font2DHandle, Font2D>(font_2D_handles, fonts, current_font_guid, filepath);
}
/*
TextureHandle GetTexture(const std::string &filepath) {
  return GetAsset<TextureHandle, Texture>(texture_handles, textures,
                                          current_texture_guid, filepath);
}
*/

void SubmitLightSource(glm::vec3 pos, glm::vec3 color, glm::float32 radius, glm::float32 ambient) {
	LightItem  item;
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

void SubmitCube(glm::mat4 t) { cubes.push_back(t); }

void Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  model_shader.use();

  int lightNR = 0;
  for (auto& light_item : lights_to_render) {
	  model_shader.uniform("light_pos[" + std::to_string(lightNR) + "]", light_item.pos);
	  model_shader.uniform("light_col[" + std::to_string(lightNR) + "]", light_item.color);
	  model_shader.uniform("light_radius[" + std::to_string(lightNR) + "]", light_item.radius);
	  model_shader.uniform("light_amb[" + std::to_string(lightNR) + "]", light_item.ambient);
	  lightNR++;
  }
  model_shader.uniform("NR_OF_LIGHTS", (int)lights_to_render.size());

  model_shader.uniform("cam_transform", cam_transform);
  //model_shader.uniform("num_frames", num_frames);
  for (auto &render_item : items_to_render) {
    model_shader.uniform("model_transform", render_item.transform);
    render_item.model->Draw(model_shader);
  }

  // render wireframe cubes
  for (auto &m : cubes) DrawCube(m);

  glBindVertexArray(quad_vao);
  text_shader.use();
  for (auto &text_item : text_to_render) {
    text_item.font->Draw(text_shader, text_item.pos, text_item.size,
                        text_item.text, text_item.color);
  }
  lights_to_render.clear();
  items_to_render.clear();
  text_to_render.clear();
  cubes.clear();

  num_frames++;
}

void* GetCamera() { return (void*)&camera; }

}  // namespace glob