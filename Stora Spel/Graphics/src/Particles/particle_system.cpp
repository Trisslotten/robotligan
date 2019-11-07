#include "particle_system.hpp"

#include <iostream>

#define CPUSPAWN 0

namespace glob {
  GLsync fence;

  ParticleSystem::ParticleSystem(ShaderProgram* ptr, GLuint tex) {
    settings_.texture = tex;
    settings_.compute_shader = ptr;

    glGenVertexArrays(1, &vertex_array_object_);

    glGenBuffers(1, &position_vbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, position_vbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * SIZE, NULL,
                 GL_DYNAMIC_DRAW);


    glGenBuffers(1, &color_vbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, color_vbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * SIZE, NULL,
                 GL_DYNAMIC_DRAW);


    glGenBuffers(1, &size_vbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, size_vbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * SIZE, NULL,
                 GL_DYNAMIC_DRAW);



    glGenBuffers(1, &time_buffer_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, time_buffer_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * SIZE, NULL, GL_DYNAMIC_DRAW);
    
    glGenBuffers(1, &velocity_buffer_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocity_buffer_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * SIZE, NULL,
                 GL_DYNAMIC_DRAW);

    
    glBindVertexArray(vertex_array_object_);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo_);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4),
                          (GLvoid*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, color_vbo_);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4),
                          (GLvoid*)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, size_vbo_);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float),
                          (GLvoid*)0);

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, time_buffer_);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (GLvoid*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Reset();
  }

  ParticleSystem::ParticleSystem(ParticleSystem&& other) noexcept {
    settings_ = other.settings_;
    color_vbo_ = other.color_vbo_;
    created_bursts_ = other.created_bursts_;
    current_index_ = other.current_index_;
    gen_ = other.gen_;
    position_vbo_ = other.position_vbo_;
    size_vbo_ = other.size_vbo_;
    spawns_ = other.spawns_;
    time_buffer_ = other.time_buffer_;
    velocity_buffer_ = other.velocity_buffer_;
    vertex_array_object_ = other.vertex_array_object_;


    // Reset other so buffers doesnt get deleted
    other.color_vbo_ = 0;
    other.position_vbo_ = 0;
    other.size_vbo_ = 0;
    other.time_buffer_ = 0;
    other.velocity_buffer_ = 0;
    other.vertex_array_object_ = 0;
  }

  ParticleSystem::~ParticleSystem() {
    glDeleteBuffers(1, &position_vbo_);
    glDeleteBuffers(1, &color_vbo_);
    glDeleteBuffers(1, &size_vbo_);
    glDeleteBuffers(1, &velocity_buffer_);
    glDeleteBuffers(1, &time_buffer_);
    glDeleteVertexArrays(1, &vertex_array_object_);
  }

  void ParticleSystem::Spawn(int num) {
    std::uniform_real_distribution<float> dist(-1.0, 1.0);

    std::vector<Particle> particles;
    particles.reserve(num);

    int new_index = current_index_ + num;
    if (new_index > SIZE) {
      new_index = SIZE;
      num = new_index - current_index_;
    }

    if (settings_.direction_strength > 1.f) settings_.direction_strength = 1.f;
    if (settings_.direction != glm::vec3(0.f))
      settings_.direction = glm::normalize(settings_.direction);

    std::uniform_real_distribution<float> nor_dist(settings_.direction_strength, 1.0);
    std::uniform_real_distribution<float> vel_dist(settings_.min_velocity, settings_.velocity);
    for (int i = 0; i < num; ++i) {
      float normal_factor = nor_dist(gen_);
      glm::vec3 dir = glm::vec3(dist(gen_), dist(gen_), dist(gen_));

      if (dir != glm::vec3(0.f))
        dir = glm::normalize(dir);

      dir -= dir * glm::dot(dir, settings_.direction);
      if (dir != glm::vec3(0.f))
        dir = glm::normalize(dir);
      float dir_factor = sqrtf(1.0f - normal_factor * normal_factor);

      dir = settings_.direction * normal_factor + dir * dir_factor;

      dir = glm::normalize(dir);

      glm::vec3 vel = dir;

     

      vel *= vel_dist(gen_);

      glm::vec4 color = settings_.colors[i % settings_.colors.size()];

      particles.push_back({glm::vec4(dir * settings_.radius + settings_.emit_pos, 0.f),
                           glm::vec4(vel, 0.f), color,
                           settings_.size, settings_.time});
    }

    std::vector<glm::vec4> pos_data;
    pos_data.reserve(num);
    std::vector<glm::vec4> vel_data;
    vel_data.reserve(num);
    std::vector<glm::vec4> color_data;
    color_data.reserve(num);
    std::vector<float> size_data;
    size_data.reserve(num);
    std::vector<float> time_data;
    time_data.reserve(num);
    
    for (auto& p : particles) {
      pos_data.push_back(p.position);
      vel_data.push_back(p.velocity);
      color_data.push_back(p.color);
      size_data.push_back(p.size);

      time_data.push_back(p.time);
    }

    glBindBuffer(GL_ARRAY_BUFFER, position_vbo_);
    void* old_data =
        glMapBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(glm::vec4), num * sizeof(glm::vec4),
                         GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                             GL_MAP_UNSYNCHRONIZED_BIT);
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);

    memcpy(old_data, pos_data.data(), pos_data.size() * sizeof(glm::vec4));
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(glm::vec4), num * sizeof(glm::vec4));
    GLboolean succes = glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, color_vbo_);
    old_data =
        glMapBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(glm::vec4),
                         num * sizeof(glm::vec4),
                                GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                                    GL_MAP_UNSYNCHRONIZED_BIT);
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    
    memcpy(old_data, color_data.data(), color_data.size() * sizeof(glm::vec4));
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(glm::vec4), num * sizeof(glm::vec4));
    succes = glUnmapBuffer(GL_ARRAY_BUFFER);
    
    glBindBuffer(GL_ARRAY_BUFFER, size_vbo_);
    old_data =
        glMapBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(float),
                         num * sizeof(float),
                                GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                                    GL_MAP_UNSYNCHRONIZED_BIT);
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    
    memcpy(old_data, size_data.data(), size_data.size() * sizeof(float));
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(float), num * sizeof(float));
    succes = glUnmapBuffer(GL_ARRAY_BUFFER);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocity_buffer_);
    old_data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
                                current_index_ * sizeof(glm::vec4),
                                num * sizeof(glm::vec4),
                         GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                             GL_MAP_UNSYNCHRONIZED_BIT);
    
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    
    memcpy(old_data, vel_data.data(), vel_data.size() * sizeof(glm::vec4));
    glFlushMappedBufferRange(GL_SHADER_STORAGE_BUFFER, current_index_ * sizeof(glm::vec4), num * sizeof(glm::vec4));
    succes = glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    
    glBindBuffer(GL_ARRAY_BUFFER, time_buffer_);
    old_data = glMapBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(float), num * sizeof(float),
                                GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                                    GL_MAP_UNSYNCHRONIZED_BIT);
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    
    memcpy(old_data, time_data.data(), time_data.size() * sizeof(float));
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(float), num * sizeof(float));
    succes = glUnmapBuffer(GL_ARRAY_BUFFER);


    current_index_ = new_index;
    current_index_ = current_index_ % SIZE;
  }

  void ParticleSystem::Update(float dt) {
#if CPUSPAWN == 0
    spawns_ += settings_.spawn_rate * dt;
    
    int new_particles = 0;
    if (!settings_.burst) {
      new_particles = spawns_;
    } else if (created_bursts_ < settings_.number_of_bursts && spawns_ >= settings_.burst_particles) {
      new_particles = settings_.burst_particles;
      created_bursts_++;
    }
    spawns_ -= new_particles;
    
    int new_index = current_index_ + new_particles;
    if (new_index > SIZE) {
      new_index = SIZE;
      new_particles = new_index - current_index_;
    }
#endif

    settings_.compute_shader->use();
    settings_.compute_shader->uniform("dt", dt);
    settings_.compute_shader->uniform("color_delta", settings_.color_delta);
    settings_.compute_shader->uniform("velocity_delta", settings_.velocity_delta);
    settings_.compute_shader->uniform("size_delta", settings_.size_delta);

#if CPUSPAWN == 0
    settings_.compute_shader->uniform("min_i", current_index_);
    settings_.compute_shader->uniform("max_i", new_index);
    settings_.compute_shader->uniform("emit_pos", settings_.emit_pos);
    settings_.compute_shader->uniformv("color", settings_.colors.size(), settings_.colors.data());
    settings_.compute_shader->uniform("num_col", (int)settings_.colors.size());
    settings_.compute_shader->uniform("dir", settings_.direction);
    settings_.compute_shader->uniform("dir_strength", settings_.direction_strength);
    settings_.compute_shader->uniform("max_speed", settings_.velocity);
    settings_.compute_shader->uniform("min_speed", settings_.min_velocity);
    settings_.compute_shader->uniform("size", settings_.size);
    settings_.compute_shader->uniform("time", settings_.time);
#endif

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, position_vbo_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, velocity_buffer_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, color_vbo_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, size_vbo_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, time_buffer_);
    
    glDispatchCompute(SIZE / 1024 + 1, 1, 1);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUseProgram(0);

#if CPUSPAWN == 0
    current_index_ = new_index;
    current_index_ = current_index_ % SIZE;
#endif

#if CPUSPAWN == 1
    spawns_ += settings_.spawn_rate * dt;
    
    int new_particles = 0;
    if (!settings_.burst) {
      new_particles = spawns_;
    } else if (created_bursts_ < settings_.number_of_bursts && spawns_ >= settings_.burst_particles) {
      new_particles = settings_.burst_particles;
      created_bursts_++;
    }
    spawns_ -= new_particles;
    
    
    if (new_particles > 0) {
      Spawn(new_particles);
    }
#endif
  }

  void ParticleSystem::Draw(ShaderProgram& shader) {
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.uniform("image", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, settings_.texture);
    glBindVertexArray(vertex_array_object_);
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_POINTS, 0, SIZE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }

  void ParticleSystem::Settings(const ParticleSettings& ps) {
    settings_ = ps;
    if (settings_.burst) spawns_ = settings_.burst_particles;
  }

  ParticleSettings ParticleSystem::GetSettings() {
    return settings_;
  }

  void ParticleSystem::SetPosition(glm::vec3 pos) {
    settings_.emit_pos = pos;
  }

  void ParticleSystem::SetDirection(glm::vec3 dir) {
    settings_.direction = dir;
  }

  void ParticleSystem::SetTexture(GLuint tex) {
    settings_.texture = tex;
  }

  void ParticleSystem::SetShader(ShaderProgram* ptr) {
    settings_.compute_shader = ptr;
  }

  void ParticleSystem::Reset() {
    spawns_ = settings_.burst_particles;
    created_bursts_ = 0;
    
    std::vector<float> time_data(SIZE, 0.f);

    glBindBuffer(GL_ARRAY_BUFFER, time_buffer_);
    void* old_data = glMapBufferRange(GL_ARRAY_BUFFER, 0, SIZE * sizeof(float),
                                GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                                    GL_MAP_UNSYNCHRONIZED_BIT);
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);

    memcpy(old_data, time_data.data(), time_data.size() * sizeof(float));
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, SIZE * sizeof(float));
    GLboolean succes = glUnmapBuffer(GL_ARRAY_BUFFER);
  }

}  // namespace glob
