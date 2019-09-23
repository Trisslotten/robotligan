#include "gui.hpp"

#include <iostream>

#include <lodepng.hpp>

glob::GUI::GUI() {}

glob::GUI::~GUI() {}

GLint glob::GUI::LoadTextureFromFile(const char* path, std::string directory) {
  std::string filename = std::string(path);
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

void glob::GUI::Draw(ShaderProgram& shader, glm::vec2 pos, GLuint tex_id,
                     unsigned int size) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  //shader.uniform("")
}
