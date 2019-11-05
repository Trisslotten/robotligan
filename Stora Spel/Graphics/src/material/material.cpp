#include "material.hpp"

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "lodepng.hpp"

namespace glob {

void Material::SetNormalMap(const std::string& path) {
  std::vector<unsigned char> image;

  unsigned width, height;
  unsigned error = lodepng::decode(image, width, height, path, LCT_RGBA);
  if (error != 0) {
    std::cout << "ERROR: material.cpp: Could not load normal map: " << path
              << "\n";
    return;
  }

  std::vector<unsigned char> to_upload;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int index = 4 * (x + y * width);
      int r = image[index+0];
      int g = image[index+1];
      int b = image[index+2];
      glm::vec3 normal = glm::vec3(r, g, b) / 255.f;
      normal = normalize(2.f*normal - 1.f);
      normal = 0.5f*normal + 0.5f;
      to_upload.push_back((unsigned char)(255.f*normal.x));
      to_upload.push_back((unsigned char)(255.f*normal.y));
    }
  }

  glGenTextures(1, &normal_map_);
  glBindTexture(GL_TEXTURE_2D, normal_map_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height, 0, GL_RG,
               GL_UNSIGNED_BYTE, to_upload.data());

  glGenerateMipmap(GL_TEXTURE_2D);
}

void Material::BindNormalMap(int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, normal_map_);
}

}  // namespace glob