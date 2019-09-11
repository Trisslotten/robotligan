#include "glob/graphics.h"

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "camera/camera.hpp"
#include "shader.h"

namespace glob {
namespace {

struct RenderItem {
  // TODO: change when model class works
  GLuint item_vao;
  int num_indices;

  glm::mat4 transform;
};

ShaderProgram test_shader;
GLuint triangle_vbo, triangle_vao;
GLuint debug_sphere_vbo, debug_sphere_vao;
int debug_sphere_num_indices;
Camera camera{glm::vec3(-3, 0, 0), glm::vec3(0), 90, 16.f / 9.f, 0.1f, 100.f};

std::vector<RenderItem> items_to_render;

void GenerateDebugSphereVerts() {
  std::vector<glm::vec3> vertices;
  float resolution = 20;
  for (int i = 0; i < resolution; i++) {
    for (int j = 0; j < resolution; j++) {
      float pitch = i * glm::pi<float>() / resolution;
      float pitch2 = (i + 1) * glm::pi<float>() / resolution;

      float yaw = j * glm::pi<float>() / resolution;
      float yaw2 = (j + 1) * glm::pi<float>() / resolution;

      glm::vec3 v1{cos(pitch), sin(pitch), 0};
      glm::vec3 v2{cos(pitch2), sin(pitch2), 0};
      glm::vec3 p1 = glm::rotateY(v1, yaw);
      glm::vec3 p2 = glm::rotateY(v2, yaw);
      glm::vec3 p3 = glm::rotateY(v1, yaw2);
      glm::vec3 p4 = glm::rotateY(v2, yaw2);
      vertices.push_back(p1);
      vertices.push_back(p2);
      vertices.push_back(p3);

      vertices.push_back(p3);
      vertices.push_back(p2);
      vertices.push_back(p4);
    }
  }
  glGenVertexArrays(1, &debug_sphere_vao);
  glBindVertexArray(debug_sphere_vao);

  glGenBuffers(1, &debug_sphere_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, debug_sphere_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);

  int stride = sizeof(glm::vec3);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);
  glBindVertexArray(0);

  debug_sphere_num_indices = vertices.size();
}

}  // namespace

void Init() {
  test_shader.add("testshader.frag");
  test_shader.add("testshader.vert");
  test_shader.compile();

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

  int stride = sizeof(glm::vec3);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);
}

void Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();

  //glBindVertexArray(quad_vao);
  glBindVertexArray(triangle_vao);
  test_shader.use();
  test_shader.uniform("cam_transform", cam_transform);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);

  for (auto& render_item : items_to_render) {
    glBindVertexArray(render_item.item_vao);
    test_shader.uniform("transform", render_item.transform);
    glDrawArrays(GL_TRIANGLES, 0, render_item.num_indices);
    // glBindVertexArray(0);
  }
}

void DebugSubmitSphere(glm::vec3 pos, float radius) {
  //RenderItem to_render;
  //items_to_render.push_back();
}

void DebugSubmitCube(glm::vec3 pos, glm::vec3 side_lengths,
                     glm::quat orientation) {}

void DebugSubmitPlane(glm::vec3 pos, glm::vec2 side_lengths, glm::vec3 up) {}

}  // namespace glob