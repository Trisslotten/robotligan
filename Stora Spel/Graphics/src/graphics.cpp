#include "glob/graphics.hpp"

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <unordered_map>

#include "Model/model.hpp"
#include "camera/camera.hpp"
#include "shader.hpp"

#include "Font/Font2D.hpp"

#include <msdfgen/msdfgen.h>
#include <msdfgen/msdfgen-ext.h>

namespace glob {
namespace {

struct RenderItem {
  Model *model;
  glm::mat4 transform;
};

ShaderProgram test_shader;
ShaderProgram model_shader;

GLuint triangle_vbo, triangle_vao;

Camera camera{
    glm::vec3(25, 5, 0), glm::vec3(0, 3, 0), 90, 16.f / 9.f, 0.1f, 100.f};

/*
TextureHandle current_texture_guid = 1;
std::unordered_map<std::string, TextureHandle> texture_handles;
std::unordered_map<TextureHandle, Texture> textures;
*/

ModelHandle current_model_guid = 1;
ModelHandle current_font_guid = 1;
std::unordered_map<std::string, ModelHandle> model_handles;
std::unordered_map<ModelHandle, Model> models;
std::unordered_map<std::string, Font2DHandle> font_2D_handles;
std::unordered_map<Font2DHandle, Font2D> fonts;

std::vector<RenderItem> items_to_render;

void DrawFullscreenQuad() {
  glBindVertexArray(triangle_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
}

}  // namespace

void Init() {
  test_shader.add("testshader.frag");
  test_shader.add("testshader.vert");
  test_shader.compile();

  model_shader.add("modelshader.vert");
  model_shader.add("modelshader.frag");
  model_shader.compile();

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
    A &asset = assets[current_model_guid];
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
  return GetAsset(font_2D_handles, fonts, current_font_guid,
                                      filepath);
	return Font2DHandle();
}
/*
TextureHandle GetTexture(const std::string &filepath) {
  return GetAsset<TextureHandle, Texture>(texture_handles, textures,
                                          current_texture_guid, filepath);
}
*/

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

void Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  model_shader.use();
  model_shader.uniform("cam_transform", cam_transform);
  for (auto &render_item : items_to_render) {
    model_shader.uniform("model_transform", render_item.transform);
    render_item.model->Draw(model_shader);
  }
  items_to_render.clear();
}

}  // namespace glob