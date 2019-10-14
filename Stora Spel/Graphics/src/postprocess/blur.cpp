#include "blur.hpp"
#include <iostream>

namespace glob {

void Blur::Init() {
  kawase_blur_compute_.add("kawaseblur.comp");
  kawase_blur_compute_.compile();
}

void Blur::CreatePass(uint16_t width, uint16_t height,
                      int32_t internal_format) {
  PassInfo pass;
  pass.info.width = width;
  pass.info.height = height;
  pass.info.internal_format = internal_format;
  passes_.insert(pass.id);
}

void Blur::Finalize() {
  for (auto id : passes_) {
    PassInfo pass_info;
    pass_info.id = id;

    GLuint textures[2] = {0, 0};
    glGenTextures(2, textures);
    for (int i = 0; i < 2; i++) {
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, pass_info.info.internal_format,
                   pass_info.info.width, pass_info.info.height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, NULL);
    }
    //Pass pass {textures, 0};
  }
}

}  // namespace glob