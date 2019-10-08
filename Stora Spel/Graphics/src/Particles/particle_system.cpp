#include "particle_system.hpp"

#include <iostream>

namespace glob {
  GLsync fence;

  ParticleSystem::ParticleSystem() {
    glGenVertexArrays(1, &vertex_array_object_);
    glBindVertexArray(vertex_array_object_);
    glGenBuffers(1, &vertex_buffer_object_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * SIZE, NULL,
                 GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (GLvoid*)0);

    glBindVertexArray(0);

    for (int i = 0; i < 10; ++i)
      particles_.push_back({glm::vec3(0.f), glm::vec3(i * 0.001f), 0.f});
  }

  ParticleSystem::~ParticleSystem() {
    //glDeleteBuffers(1, &vertex_buffer_object_);
  }

  void ParticleSystem::Update() {
    for (auto& p : particles_) {
      p.position += p.velocity;
    }
  }

  void ParticleSystem::Draw(ShaderProgram& shader) {
    Update();

    std::vector<Vertex> data;
    //Vertex vertex;
    //glm::vec3 up(0.f, 1.f, 0.f);
    //glm::vec3 right(1.f, 0.f, 0.f);
    //
    //vertex.pos = -right - up;
    //data.push_back(vertex);
    //
    //vertex.pos = right - up;
    //data.push_back(vertex);
    //
    //vertex.pos = -right + up;
    //data.push_back(vertex);
    //
    //vertex.pos = right + up;
    //data.push_back(vertex);
    for (auto& p : particles_) {
      Vertex vertex;

      vertex.pos = p.position;
      data.push_back(vertex);
      //glm::vec3 up(0.f, 1.f, 0.f);
      //up *= 2.f;
      //glm::vec3 right(1.f, 0.f, 0.f);
      //
      //vertex.pos = p.position + -right - up;
      //data.push_back(vertex);
      //
      //vertex.pos = p.position + right - up;
      //data.push_back(vertex);
      //
      //vertex.pos = p.position + -right + up;
      //data.push_back(vertex);
      //
      //vertex.pos = p.position + right + up;
      //data.push_back(vertex);
    }
    //
    //std::vector<glm::vec3> quad_vertices{
    //    {-1, -1, 0}, {1, -1, 0}, {-1, 1, 0}, {1, 1, 0}};
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
    void* old_data =
        glMapBufferRange(GL_ARRAY_BUFFER, 0, SIZE,
                         GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                             GL_MAP_UNSYNCHRONIZED_BIT);
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    
    memcpy(old_data, data.data(),
           data.size() * sizeof(glm::vec3));
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, SIZE);
    GLboolean succes = glUnmapBuffer(GL_ARRAY_BUFFER);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDepthFunc(GL_ALWAYS);
    //glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    glBindVertexArray(vertex_array_object_);
    glDrawArrays(GL_POINTS, 0, data.size());
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    //
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    //glDisable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glDisable(GL_CULL_FACE);
    //glDrawArrays(GL_TRIANGLES, 0, data.size());
    //glEnable(GL_CULL_FACE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glEnable(GL_DEPTH_TEST);
    //
    //fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }

}  // namespace glob