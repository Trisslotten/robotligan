#include "material.hpp"

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "lodepng.hpp"

#include <textureslots.hpp>
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
  GLuint GetNormalMap(const std::string& path);
  GLuint GetMetallicMap(const std::string& path);

  std::unordered_map<std::string, GLuint> textures_;
};
}  // namespace

void Materials::Init() {}

Material Materials::GetMaterial(
    std::unordered_map<materials::Type, std::string> wanted_textures) {
  Material result;
  for (auto& [type, path] : wanted_textures) {
    switch (type) {
      case materials::NORMAL:
        result.AddTexture(type, GetNormalMap(path), TEXTURE_SLOT_NORMAL,
                          "texture_normal");
        break;
      case materials::METALLIC:
        result.AddTexture(type, GetMetallicMap(path), TEXTURE_SLOT_METALLIC,
                          "texture_metallic");
        break;
    }
  }
  return result;
}

GLuint Materials::GetNormalMap(const std::string& path) {
  GLuint result = 0;
  auto iter = textures_.find(path);
  if (iter != textures_.end()) {
    result = iter->second;
  } else {
    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, path, LCT_RGBA);
    if (error != 0) {
      std::cout << "ERROR: material.cpp: Could not load normal map: " << path
                << "\n";
      return result;
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

    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height, 0, GL_RG,
                 GL_UNSIGNED_BYTE, to_upload.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    textures_[path] = result;
  }
  return result;
}
GLuint Materials::GetMetallicMap(const std::string& path) {
  GLuint result = 0;
  auto iter = textures_.find(path);
  if (iter != textures_.end()) {
    result = iter->second;
  } else {
    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, path, LCT_GREY);
    if (error != 0) {
      std::cout << "ERROR: material.cpp: Could not load metallic map: " << path
                << "\n";
      return result;
    }
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED,
                 GL_UNSIGNED_BYTE, image.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    textures_[path] = result;
  }
  return result;
}

void materials::Init() { Materials::getInstance().Init(); }

Material glob::materials::Get(
    std::unordered_map<Type, std::string> wanted_textures) {
  return Materials::getInstance().GetMaterial(wanted_textures);
}

void Material::AddTexture(materials::Type type, GLuint id, int slot,
                          const std::string& uniform_name) {
  textures_[type] = {id, slot, uniform_name};
}

void Material::Bind(ShaderProgram& shader) {
  for (auto& [type, texture] : textures_) {
    if (texture.id != 0) {
      glActiveTexture(GL_TEXTURE0 + texture.slot);
      glBindTexture(GL_TEXTURE_2D, texture.id);
      shader.uniform(texture.uniform_name, texture.slot);
    }
  }
}

}  // namespace glob
