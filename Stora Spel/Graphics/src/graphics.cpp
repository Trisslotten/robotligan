#include "glob/graphics.h"

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "camera/camera.hpp"
#include "shader.h"
#include "Model/model.h"

namespace glob {
namespace {

struct RenderItem {
  // TODO: change when model class works
  GLuint item_vao;
  int num_indices;

  glm::mat4 transform;
};

ShaderProgram model_shader;

ShaderProgram test_shader;
GLuint triangle_vbo, triangle_vao;
GLuint debug_sphere_vbo, debug_sphere_vao;
int debug_sphere_num_indices;
Camera camera{glm::vec3(-3, 0, 0), glm::vec3(0), 90, 16.f / 9.f, 0.1f, 100.f};

Model model;

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
      {-1, -1, -1},
      {3, -1, 1},
      {-1, 3, 1},
  };

  glGenBuffers(1, &triangle_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);

  int stride = sizeof(glm::vec3);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);

  
  model.LoadModelFromFile("assets/Mech/Ball.obj");
}

void Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  cam_transform =
      glm::perspective(glm::radians(90.f), 16.f / 9.f, 0.1f, 100.f) *
      glm::lookAt(glm::vec3(3), glm::vec3(0), glm::vec3(0, 1, 0));

  glBindVertexArray(triangle_vao);
  test_shader.use();
  test_shader.uniform("cam_transform", cam_transform);
  //glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);

  /*
  for (auto& render_item : items_to_render) {
    glBindVertexArray(render_item.item_vao);
    test_shader.uniform("transform", render_item.transform);
    glDrawArrays(GL_TRIANGLES, 0, render_item.num_indices);
    // glBindVertexArray(0);
  }
  */
  model_shader.use();
  model_shader.uniform("cam_transform", cam_transform);
  model.Draw(model_shader.getId());

}

void DebugSubmitSphere(glm::vec3 pos, float radius) {
  //RenderItem to_render;
  //items_to_render.push_back();
}

void DebugSubmitCube(glm::vec3 pos, glm::vec3 side_lengths,
                     glm::quat orientation) {}

void DebugSubmitPlane(glm::vec3 pos, glm::vec2 side_lengths, glm::vec3 up) {}

}  // namespace glob