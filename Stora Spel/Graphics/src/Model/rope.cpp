#include "rope.hpp"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <lodepng.hpp>
#include <vector>

namespace glob {

void Rope::Init() {
  constexpr float two_pi = 2.f * glm::pi<float>();

  constexpr int length_res = 20;
  constexpr int cylinder_res = 50;
  constexpr int num_indices = (length_res - 1) * cylinder_res * 6;

  static_assert(num_indices < (int)USHRT_MAX,
                "Rope resolution too big. Change indices to unsigned int or "
                "make rope smaller resolution");

  std::vector<glm::vec4> vertices;
  std::vector<unsigned short> indices;

  for (int i = 0; i < length_res; i++) {
    float x = float(i) / length_res;

    for (int j = 0; j < cylinder_res; j++) {
      float ratio = float(j) / cylinder_res;
      float a = two_pi * float(j) / cylinder_res;

      vertices.push_back(
          glm::vec4(x, 0.5f * glm::cos(a), 0.5f * glm::sin(a), ratio));
    }
  }

  for (int i = 0; i < length_res - 1; i++) {
    for (int j = 0; j < cylinder_res; j++) {
      int bl, br, tl, tr;
      bl = (j + 0) + (i + 0) * cylinder_res;
      tl = (j + 0) + (i + 1) * cylinder_res;
      if (j < cylinder_res - 1) {
        br = (j + 1) + (i + 0) * cylinder_res;
        tr = (j + 1) + (i + 1) * cylinder_res;
      } else {
        br = 0 + (i + 0) * cylinder_res;
        tr = 0 + (i + 1) * cylinder_res;
      }
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
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
               indices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4),
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, image.data());
}

void Rope::TestDraw(Camera camera) {
  glBindVertexArray(vao_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_diffuse_);
  shader_.use();
  shader_.uniform("cam_transform", camera.GetViewPerspectiveMatrix());
  shader_.uniform("texture_diffuse", 0);
  glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_SHORT, 0);
}

}  // namespace glob