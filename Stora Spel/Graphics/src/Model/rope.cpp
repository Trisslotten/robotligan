#include "rope.hpp"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <lodepng.hpp>
#include <vector>

namespace glob {

void Rope::Init() {
  constexpr int length_res = 20;
  constexpr int cylinder_res = 50;
  constexpr int num_indices = (length_res - 1) * (cylinder_res + 1) * 6;

  static_assert(
      num_indices < (int)USHRT_MAX,
      "Rope mesh resolution too big. Change indices to unsigned int or "
      "make rope smaller resolution");

  std::vector<glm::vec2> vertices;
  std::vector<unsigned short> indices;

  for (int i = 0; i < length_res; i++) {
    float x = float(i) / length_res;
    for (int j = 0; j <= cylinder_res; j++) {
      float ratio = float(j) / cylinder_res;
      vertices.emplace_back(x, ratio);
    }
  }

  for (int i = 0; i < length_res - 1; i++) {
    for (int j = 0; j < cylinder_res; j++) {
      int bl = (j + 0) + (i + 0) * (cylinder_res + 1);
      int tl = (j + 0) + (i + 1) * (cylinder_res + 1);
      int br = (j + 1) + (i + 0) * (cylinder_res + 1);
      int tr = (j + 1) + (i + 1) * (cylinder_res + 1);
      indices.push_back(tl);
      indices.push_back(bl);
      indices.push_back(br);

      indices.push_back(br);
      indices.push_back(tr);
      indices.push_back(tl);
    }
  }
  num_indices_ = indices.size();

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ebo_);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short),
               indices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2),
                        (GLvoid*)0);
  glBindVertexArray(0);

  shader_.add("rope.vert");
  shader_.add("rope.frag");
  shader_.compile();

  std::string filename = "assets/materials/Rope02_col.png";
  std::vector<unsigned char> image;
  unsigned width, height;
  unsigned error = lodepng::decode(image, width, height, filename, LCT_RGB);
  if (error != 0) {
    std::cout << "ERROR: Could not load rope texture: " << filename << "\n";
    return;
  }

  glGenTextures(1, &texture_diffuse_);
  glBindTexture(GL_TEXTURE_2D, texture_diffuse_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, image.data());
  glGenerateMipmap(GL_TEXTURE_2D);
}

void Rope::TestDraw(Camera camera) {
  static float time = 0;
  time += 1.f/60.f;

  glm::vec3 start_pos = glm::vec3(0);
  glm::vec3 end_pos = glm::vec3(3*glm::sin(time), 5, 3*glm::cos(time));

  glBindVertexArray(vao_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_diffuse_);
  shader_.use();
  shader_.uniform("cam_transform", camera.GetViewPerspectiveMatrix());
  shader_.uniform("texture_diffuse", 0);
  shader_.uniform("start_pos", start_pos);
  shader_.uniform("end_pos", end_pos);
  glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_SHORT, 0);
}

}  // namespace glob