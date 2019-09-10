#include "mesh.h"

void Mesh::SetupMesh() {
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

  glEnableVertexAttribArray(3);  // Layout 2 for normals
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid*)offsetof(Vertex, normals));
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices,
           std::vector<Texture> textures) {
  vertices_ = vertices;
  indices_ = indices;
  textures_ = textures;

  SetupMesh();
}

Mesh::~Mesh() {}

void Mesh::Draw(GLuint shader) {
  for (unsigned int i = 0; i < textures_.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);  // Activate proper texture unit
    std::string name = textures_[i].type;
    glUniform1i(glGetUniformLocation(shader, (name).c_str()), i + 2);
  }
  glActiveTexture(GL_TEXTURE0);

  // Draw mesh
  glBindVertexArray(vertex_array_object_);
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
