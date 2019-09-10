#ifndef MESH_H_
#define MESH_H_

#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normals;
  glm::vec2 texture;
};

struct Texture {
  GLuint id_texture;
  std::string type;
  aiString path;
};

class Mesh {
 private:
 public:
};

#endif