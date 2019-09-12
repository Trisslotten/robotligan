#ifndef MESH_H_
#define MESH_H_

#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace glob {

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
  GLuint vertex_array_object_, vertex_buffer_object_, element_buffer_object_;

  std::vector<Vertex> vertices_;
  std::vector<GLuint> indices_;
  std::vector<Texture> textures_;

  void SetupMesh();

 public:
  Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices,
       const std::vector<Texture>& textures);
  ~Mesh();

  void Draw(GLuint shader);
};

}  // namespace glob

#endif