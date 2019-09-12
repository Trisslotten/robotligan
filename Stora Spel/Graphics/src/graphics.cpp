#include "glob/graphics.h"

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <unordered_map>

#include "Model/model.h"
#include "camera/camera.hpp"
#include "shader.h"

namespace glob {
namespace {

struct RenderItem {
  Model* model;
  glm::mat4 transform;
};

ShaderProgram model_shader;
ShaderProgram test_shader;

GLuint triangle_vbo, triangle_vao;

Camera camera{glm::vec3(-3, 0, 0), glm::vec3(0), 90, 16.f / 9.f, 0.1f, 100.f};

ModelHandle current_guid = 1;

std::unordered_map<std::string, ModelHandle> model_handles;
std::unordered_map<ModelHandle, Model> models;
std::vector<RenderItem> items_to_render;

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
                        (GLvoid*)0);
}

ModelHandle GetModel(const std::string& filepath) {
  ModelHandle result = 0;

  auto item = model_handles.find(filepath);
  if (item == model_handles.end()) {
    std::cout << "DEBUG graphics.cpp: Loading model '" << filepath << "'\n";
    Model& model = models[current_guid];
    model.LoadModelFromFile(filepath.c_str());
    if (model.IsLoaded()) {
      model_handles[filepath] = current_guid;
      result = current_guid;
      current_guid++;
    } else {
      // remove the model since it could not load
      models.erase(current_guid);
    }
  } else {
    // if model is loaded
    std::cout << "DEBUG graphics.cpp: Model '" << filepath << "' already loaded\n";
    result = item->second;
  }

  return result;
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

  RenderItem to_render;
  to_render.model = &find_res->second;
  to_render.transform = transform;
  items_to_render.push_back(to_render);
}

void Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  /*
  glBindVertexArray(triangle_vao);
  test_shader.use();
  test_shader.uniform("cam_transform", cam_transform);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  */

  model_shader.use();
  model_shader.uniform("cam_transform", cam_transform);
  for (auto& render_item : items_to_render) {
    model_shader.uniform("model_transform", render_item.transform);
    render_item.model->Draw(model_shader.getId());
  }
  items_to_render.clear();

}

void DebugSubmitSphere(glm::vec3 pos, float radius) {
  // RenderItem to_render;
  // items_to_render.push_back();
}

void DebugSubmitCube(glm::vec3 pos, glm::vec3 side_lengths,
                     glm::quat orientation) {}

void DebugSubmitPlane(glm::vec3 pos, glm::vec2 side_lengths, glm::vec3 up) {}

}  // namespace glob