#include "material.hpp"

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "lodepng.hpp"

#include <unordered_map>

namespace glob {

namespace {

class Materials {
 public:
  Materials(Materials const&) = delete;
  void operator=(Materials const&) = delete;
  static Materials& getInstance() {
    static Materials instance;
    return instance;
  }

  Material GetMaterial(
      std::unordered_map<materials::Type, std::string> wanted_textures);

  void Init();

 private:
  Materials() {}
  Materials(Materials const&);

  void LoadNormalMap(const std::string& word, const std::string& path);

  std::unordered_map<std::string, GLuint> normal_maps_;
};

}  // namespace

void Materials::Init() {
  LoadNormalMap("METAL_WEAVY", "Assets/Texture/normal_metal_weavy.png");
}

Material Materials::GetMaterial(
    std::unordered_map<materials::Type, std::string> wanted_textures) {
  Material result;
  for (auto& [type, word] : wanted_textures) {
    switch (type) {
      case materials::NORMAL:
        auto iter = normal_maps_.find(word);
        if (iter != normal_maps_.end()) result.SetNormalMap(iter->second);
        break;
    }
  }
  return result;
}

void Materials::LoadNormalMap(const std::string& word,
                              const std::string& path) {
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
      int r = image[index + 0];
      int g = image[index + 1];
      int b = image[index + 2];
      glm::vec3 normal = glm::vec3(r, g, b) / 255.f;
      normal = normalize(2.f * normal - 1.f);
      normal = 0.5f * normal + 0.5f;
      to_upload.push_back((unsigned char)(255.f * normal.x));
      to_upload.push_back((unsigned char)(255.f * normal.y));
    }
  }

  glGenTextures(1, &normal_maps_[word]);
  glBindTexture(GL_TEXTURE_2D, normal_maps_[word]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height, 0, GL_RG,
               GL_UNSIGNED_BYTE, to_upload.data());
  glGenerateMipmap(GL_TEXTURE_2D);
}

void Material::BindNormalMap(int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, normal_map_);
}

Material glob::materials::Get(
    std::unordered_map<Type, std::string> wanted_textures) {
  return Materials::getInstance().GetMaterial(wanted_textures);
}

}  // namespace glob
