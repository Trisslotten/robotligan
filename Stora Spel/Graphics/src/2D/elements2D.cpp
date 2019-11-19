#include "elements2D.hpp"

#include <iostream>

#include <lodepng.hpp>
#include "glob/window.hpp"

void glob::Elements2D::CreateTexture() {
  // Generate texture id
  GLuint texture_id;
  glGenTextures(1, &texture_id);

  // Generate texture data
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width_, texture_height_, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, gui_element_texture_.data());
  glGenerateMipmap(GL_TEXTURE_2D);

  // Set some parameters for the texture

  glBindTexture(GL_TEXTURE_2D, 0);  // Unbind the texture

  tex_id_ = texture_id;
}

glob::Elements2D::Elements2D() {}

glob::Elements2D::~Elements2D() {}

bool glob::Elements2D::LoadFromFile(const std::string& path) {
  std::string filename = std::string(path);
  // directory_ = path.substr(0, path.find_last_of('/'));
  // filename = directory_ + '/' + filename;

  // Load texture
  unsigned error = lodepng::decode(gui_element_texture_, texture_width_,
                                   texture_height_, filename);
  if (error != 0) {
    std::cout << "ERROR: Could not load texture: " << filename << "\n";
    return false;
  }

  CreateTexture();
  is_loaded_ = true;
  return true;
}

void glob::Elements2D::DrawOnScreen(ShaderProgram& shader, glm::vec2 pos,
                                    float scale, float scale_x, float opacity, float rot) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id_);

  shader.uniform("t_pos", pos);
  shader.uniform("t_scale", scale);
  shader.uniform("t_scale_x", scale_x);
  shader.uniform("opacity", opacity);
  shader.uniform("t_rot", rot);
  shader.uniform("screen_dims", window::GetWindowDimensions());
  shader.uniform("texture_dims", glm::vec2(texture_width_, texture_height_));
  shader.uniform("gui_element_texture", 0);

  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_ALWAYS);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDepthFunc(GL_LESS);
  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void glob::Elements2D::DrawInWorld(ShaderProgram& shader, glm::vec3 pos,
                                   float scale, glm::mat4 rot) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id_);

  shader.uniform("t_pos", pos);
  shader.uniform("t_scale", scale);
  shader.uniform("t_rot", rot);
  shader.uniform("texture_dims", glm::vec2(texture_width_, texture_height_));
  shader.uniform("gui_element_texture", 0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisable(GL_BLEND);

  glBindTexture(GL_TEXTURE_2D, 0);
}
