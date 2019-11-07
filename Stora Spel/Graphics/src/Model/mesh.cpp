#include "mesh.hpp"

#include <iostream>
#include "../usegl.hpp"

namespace glob {

void Mesh::SetupMesh(bool weighted) {
  /*---------------Generate needed buffers--------------*/
  glGenVertexArrays(1, &vertex_array_object_);
  glGenBuffers(1, &vertex_buffer_object_);
  glGenBuffers(1, &element_buffer_object_);

  /*---------------Binding vertex buffer---------------*/

  glBindVertexArray(vertex_array_object_);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
  glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex),
               &vertices_[0], GL_STATIC_DRAW);

  /*---------------Binding element buffer--------------*/
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint),
               &indices_[0], GL_STATIC_DRAW);

  /*---------------Enable arrays----------------------*/
  glEnableVertexAttribArray(0);  // Layout 0 for vertices
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

  glEnableVertexAttribArray(1);  // Layout 1 for textures
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid*)offsetof(Vertex, texture));

  glEnableVertexAttribArray(2);  // Layout 2 for normals
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid*)offsetof(Vertex, normals));

  if (weighted) {
    glGenBuffers(1, &bone_buffer_object_);
    glGenBuffers(1, &weight_buffer_object_);

    glBindBuffer(GL_ARRAY_BUFFER, bone_buffer_object_);  // bone indexes
    glBufferData(GL_ARRAY_BUFFER, bone_index_.size() * sizeof(glm::ivec4),
                 &bone_index_[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(glm::ivec4), (GLvoid*)0);

    glBindBuffer(GL_ARRAY_BUFFER, weight_buffer_object_);  // weights
    glBufferData(GL_ARRAY_BUFFER, weights_.size() * sizeof(glm::vec4),
                 &weights_[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4),
                          (GLvoid*)0);
  }
}

Mesh::Mesh(const std::vector<Vertex>& vertices,
           const std::vector<GLuint>& indices,
           const std::vector<Texture>& textures) {
  vertices_ = vertices;
  indices_ = indices;
  textures_ = textures;

  if (glob::kModelUseGL) {
    SetupMesh(weighted_);
  }
}

Mesh::Mesh(const std::vector<Vertex>& vertices,
           const std::vector<GLuint>& indices,
           const std::vector<Texture>& textures,
           const std::vector<glm::vec4> weights,
           const std::vector<glm::ivec4> boneIndex) {
  vertices_ = vertices;
  indices_ = indices;
  textures_ = textures;

  weights_ = weights;
  bone_index_ = boneIndex;

  weighted_ = true;
  if (glob::kModelUseGL) {
    SetupMesh(weighted_);
  }
}

Mesh::~Mesh() {}

void Mesh::Draw(ShaderProgram& shader) {
  for (auto& texture : textures_) {
    glActiveTexture(GL_TEXTURE0 + texture.slot);
    glBindTexture(GL_TEXTURE_2D, texture.id_texture);
    shader.uniform(texture.type, texture.slot);
  }

  // Draw mesh
  glBindVertexArray(vertex_array_object_);
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

MeshData Mesh::GetMeshData() {
  MeshData mesh_data;
  mesh_data.pos.reserve(vertices_.size());
  mesh_data.indices.reserve(indices_.size());

  for (auto& v : vertices_) {
    mesh_data.pos.push_back(v.position);
  }

  for (auto& i : indices_) {
    mesh_data.indices.push_back(i);
  }

  return mesh_data;
}

}  // namespace glob