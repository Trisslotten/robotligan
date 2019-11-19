#ifndef MESH_HPP_
#define MESH_HPP_

#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "../shader.hpp"
#include "glob/mesh_data.hpp"

namespace glob {

struct Vertex {
  glm::vec3 position;
  glm::vec3 normals;
  glm::vec2 texture;
};

struct Texture {
  GLuint id_texture;
  int slot = 0;
  std::string type;
  aiString path;
};

class Mesh {
 private:
  GLuint vertex_array_object_, vertex_buffer_object_, element_buffer_object_,
      weight_buffer_object_, bone_buffer_object_;

  std::vector<Vertex> vertices_;
  std::vector<GLuint> indices_;
  std::vector<Texture> textures_;

  std::vector<glm::vec4> weights_;
  std::vector<glm::ivec4> bone_index_;

  glm::mat4 transform_;

  bool weighted_ = false;

  void SetupMesh(bool weighted);

 public:
  Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices,
       const std::vector<Texture>& textures, glm::mat4 transform);
  Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices,
       const std::vector<Texture>& textures,
       const std::vector<glm::vec4> weights,
       const std::vector<glm::ivec4> boneIndex);
  ~Mesh();

  void Draw(ShaderProgram& shader);

  MeshData GetMeshData();
};

}  // namespace glob

#endif  // MESH_HPP_