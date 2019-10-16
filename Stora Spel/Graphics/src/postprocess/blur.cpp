#include "blur.hpp"
#include <iostream>

namespace glob {

void Blur::Init() {
  kawase_blur_compute_.add("kawaseblur.comp");
  kawase_blur_compute_.compile();
}

uint64_t Blur::CreatePass(uint16_t width, uint16_t height,
                          int32_t internal_format) {
  PassInfo pass;
  pass.info.width = width;
  pass.info.height = height;
  pass.info.internal_format = internal_format;
  passes_[pass.id];

  GLuint textures[2] = {0, 0};
  glGenTextures(2, textures);
  for (int i = 0; i < 2; i++) {
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, pass.info.internal_format, pass.info.width,
                 pass.info.height, 0, GL_RG, GL_FLOAT, NULL);
  }
  // create PassTextures instance and copy textures
  memcpy(passes_[pass.id].textures, textures, 2 * sizeof(GLuint));

  return pass.id;
}

GLuint Blur::BlurTexture(uint64_t pass_id, int num_passes,
                         GLuint source_texture, int source_level,
                         GLuint result_texture) {
  auto iter = passes_.find(pass_id);
  if (iter == passes_.end()) {
    return 0;
  }

  PassInfo pass_info;
  pass_info.id = pass_id;

  Pass& pass = passes_[pass_id];

  float group_size = 16;
  auto tex_size = glm::vec2(pass_info.info.width, pass_info.info.height);
  glm::ivec2 num_groups = glm::ceil(tex_size / group_size);

  kawase_blur_compute_.use();
  kawase_blur_compute_.uniform("size", tex_size);
  // the kernels for each pass
  std::vector<int> kernels = {0, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  for (int i = 0; i < kernels.size() && i < num_passes; i++) {
    kawase_blur_compute_.uniform("kernel", kernels[i]);

    int read_level = 0;

    int curr_i = pass.index;
    int next_i = (pass.index + 1) % 2;
    glActiveTexture(GL_TEXTURE0);
    if (i == 0) {
      read_level = source_level;
      glBindTexture(GL_TEXTURE_2D, source_texture);
    } else {
      glBindTexture(GL_TEXTURE_2D, pass.textures[curr_i]);
    }
    kawase_blur_compute_.uniform("read_tex", 0);
    kawase_blur_compute_.uniform("read_level", read_level);

    if (result_texture != 0 && i == glm::min((int)kernels.size(), num_passes) - 1) {
      glBindImageTexture(0, result_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY,
                         pass_info.info.internal_format);
    } else {
      glBindImageTexture(0, pass.textures[next_i], 0, GL_FALSE, 0,
                         GL_WRITE_ONLY, pass_info.info.internal_format);
    }

    glDispatchCompute(num_groups.x, num_groups.y, 1);

    // glMemoryBarrier(GL_ALL_BARRIER_BITS);

    pass.index = next_i;
  }
  // glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

  if (result_texture == 0) result_texture = pass.textures[pass.index];

  return result_texture;
}

}  // namespace glob