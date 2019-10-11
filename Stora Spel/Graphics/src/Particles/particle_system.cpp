#include "particle_system.hpp"

#include <iostream>
#include <chrono>

class Timer {
  bool paused_ = false;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
  std::chrono::time_point<std::chrono::high_resolution_clock> pause_time_;

 public:
  Timer();
  double Restart();
  double Elapsed();
  void Pause();
  void Resume();
};

namespace glob {
  GLsync fence;

  ParticleSystem::ParticleSystem(ShaderProgram* ptr, GLuint tex) : compute_shader_(ptr), texture_(tex) {
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

    //Reset();

    //std::random_device dev;
    //std::mt19937 rng(dev());
    //std::default_random_engine gen;
    //std::uniform_real_distribution<float> dist(-0.5, 0.5);

    //for (int i = 0; i < SIZE / 10; ++i) {
    //  glm::vec3 test = glm::normalize(glm::vec3(dist(gen), dist(gen), dist(gen)));
    //
    //  particles_.push_back(
    //      {glm::vec3(0.f), test * dist(gen),
    //       glm::vec4(dist(gen) + 0.5f, dist(gen) + 0.5f, dist(gen) + 0.5f, 1.f), 0.03f,
    //       0.f});
    //}
  }

  ParticleSystem::~ParticleSystem() { CleanUp(); }

  void ParticleSystem::CleanUp() {
    //glDeleteTextures(1, &texture_);
    //glDeleteBuffers(1, &position_vbo_);
    //glDeleteBuffers(1, &color_vbo_);
    //glDeleteBuffers(1, &size_vbo_);
    //glDeleteBuffers(1, &velocity_buffer_);
    //glDeleteBuffers(1, &time_buffer_);
    //glDeleteVertexArrays(1, &vertex_array_object_);
    position_vbo_ = 0;
    color_vbo_ = 0;
    size_vbo_ = 0;
    vertex_array_object_ = 0;
    texture_ = 0;
    velocity_buffer_ = 0;
    time_buffer_ = 0;
  }

  ParticleSystem::ParticleSystem(ParticleSystem&& other) {
    //particles_ = std::move(other.particles_);
    settings_ = other.settings_;

    compute_shader_ = other.compute_shader_;
    position_vbo_ = other.position_vbo_;
    other.position_vbo_ = 0;
    color_vbo_ = other.color_vbo_;
    other.color_vbo_ = 0;
    size_vbo_ = other.size_vbo_;
    other.size_vbo_ = 0;
    vertex_array_object_ = other.vertex_array_object_;
    other.vertex_array_object_ = 0;
    texture_ = other.texture_;
    other.texture_ = 0;
    //glm::vec4 color_ = glm::vec4(0.3f, 0.4f, 0.6f, 0.3f);
    //float particle_size_ = 0.03f;
    //int SIZE = 10000;
  }

  //ParticleSystem& ParticleSystem::operator=(ParticleSystem&& other) {
  //  if (this != &other) {
  //    CleanUp();
  //    std::swap(position_vbo_, other.position_vbo_);
  //    std::swap(color_vbo_, other.color_vbo_);
  //    std::swap(size_vbo_, other.size_vbo_);
  //    std::swap(vertex_array_object_, other.vertex_array_object_);
  //    std::swap(texture_, other.texture_);
  //  }
  //
  //  return *this;
  //}

  void ParticleSystem::Spawn(int num) {
    std::uniform_real_distribution<float> dist(-0.2, 0.2);
    std::uniform_real_distribution<float> dist1(-0.2, 0.2);

    std::vector<Particle> particles;
    particles.reserve(num);

    //int new_particles = SIZE;
    int new_index = current_index_ + num;
    if (new_index > SIZE) {
      new_index = SIZE;
      num = new_index - current_index_;
    }

    for (int i = 0; i < num; ++i) {
      glm::vec3 test = glm::vec3(dist1(gen_), dist1(gen_), dist1(gen_));
      test = glm::normalize(test);

      glm::vec3 vel = test * dist(gen_);
      particles.push_back({glm::vec4(test * 0.4f + settings_.emit_pos, 0.f),
                           glm::vec4(vel, 0.f), settings_.color,
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

    // Flush opengl errors
    //GLenum err;
    //while ((err = glGetError()) != GL_NO_ERROR)
    //  ;

    glBindBuffer(GL_ARRAY_BUFFER, position_vbo_);
    void* old_data =
        glMapBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(glm::vec4), num * sizeof(glm::vec4),
                         GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                             GL_MAP_UNSYNCHRONIZED_BIT);
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);

    memcpy(old_data, pos_data.data(), pos_data.size() * sizeof(glm::vec4));
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(glm::vec4),
                             num * sizeof(glm::vec4));
    GLboolean succes = glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, color_vbo_);
    old_data =
        glMapBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(glm::vec4),
                         num * sizeof(glm::vec4),
                                GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                                    GL_MAP_UNSYNCHRONIZED_BIT);
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    
    memcpy(old_data, color_data.data(), color_data.size() * sizeof(glm::vec4));
    glFlushMappedBufferRange(GL_ARRAY_BUFFER,
                             current_index_ * sizeof(glm::vec4),
                             num * sizeof(glm::vec4));
    succes = glUnmapBuffer(GL_ARRAY_BUFFER);
    
    glBindBuffer(GL_ARRAY_BUFFER, size_vbo_);
    old_data =
        glMapBufferRange(GL_ARRAY_BUFFER, current_index_ * sizeof(float),
                         num * sizeof(float),
                                GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                                    GL_MAP_UNSYNCHRONIZED_BIT);
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    
    memcpy(old_data, size_data.data(), size_data.size() * sizeof(float));
    glFlushMappedBufferRange(GL_ARRAY_BUFFER,
                             current_index_ * sizeof(float),
                             num * sizeof(float));
    succes = glUnmapBuffer(GL_ARRAY_BUFFER);
    //
    //while ((err = glGetError()) != GL_NO_ERROR)
    //  ;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocity_buffer_);
    old_data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
                                current_index_ * sizeof(glm::vec4),
                                num * sizeof(glm::vec4),
                         GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                             GL_MAP_UNSYNCHRONIZED_BIT);
    
    //err = glGetError();
    //if (err != GL_NO_ERROR) {
    //  std::cout << "opengl error\n";
    //}
    glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    
    memcpy(old_data, vel_data.data(), vel_data.size() * sizeof(glm::vec4));
    glFlushMappedBufferRange(GL_SHADER_STORAGE_BUFFER,
                             current_index_ * sizeof(glm::vec4),
                             num * sizeof(glm::vec4));
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
    static bool first = true;
    if (first) {
      Reset();
      first = false;
    }

    compute_shader_->use();
    compute_shader_->uniform("dt", dt);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, position_vbo_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, velocity_buffer_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, color_vbo_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, size_vbo_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, time_buffer_);
    
    glDispatchCompute(10000 / 1024 + 1, 1, 1);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUseProgram(0);


    spawns_ += settings_.spawn_rate * dt;
    int new_particles = spawns_;

    spawns_ -= new_particles;

    if (new_particles > 0)
      Spawn(new_particles);

    //static float time = 0.f;
    //Timer timer;
    //time += dt;

    //for (auto& p : particles_) {
    //  p.time += dt;
    //  p.velocity.y += 0.01;
    //  p.position += p.velocity * 0.01f;
    //  if (p.size > -.05f)
    //    p.size -= 0.01f;
    //  //p.size += dist(gen);
    //
    //  //if (p.size > 0.3f) p.size = 0.05f;
    //  //else if (p.size < 0.03f)
    //  //  p.size = 0.05f;
    //
    //
    //  //if (p.time > 1) p.color = glm::vec4(1.0f, 0.2f, 0.0f, 0.1f);
    //}


    //for (auto it = particles_.begin(); it != particles_.end();) {
    //  if (it->time > 5) {
    //    it = particles_.erase(it);
    //  } else {
    //    it++;
    //  }
    //}
    
    //if (time < 0.3f) return;
    //std::default_random_engine gen1;
    //std::uniform_real_distribution<float> dist1(-0.5, 0.5);
    //new_particles = SIZE / 5 * dt;
    //for (int i = 0; i < new_particles; ++i) {
    //  glm::vec3 test =
    //      glm::normalize(glm::vec3(dist1(gen), dist1(gen), dist1(gen)));
    //
    //  particles_.push_back(
    //      {glm::vec3(0.f, -4.f, 0.f), test * dist1(gen), glm::vec4(0.6f, 0.6f, .6f, 1.0f),
    //       0.03f, 0.f});
    //}
    //if (time > 1.0f) {
    //  time -= 1.0f;
    //  auto t = timer.Elapsed();
    //  //std::cout << "update time: " << t << std::endl;
    //}
  }

  void ParticleSystem::Draw(ShaderProgram& shader, const Camera& camera) {
    //static float time = 0.f;
    //Timer timer;
    //time += 0.01f;

    //std::vector<Vertex> pos_data;
    //pos_data.reserve(SIZE);
    //std::vector<glm::vec4> color_data;
    //color_data.reserve(SIZE);
    //std::vector<float> size_data;
    //size_data.reserve(SIZE);
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
    
    //glm::vec3 cam_pos = camera.GetPosition();
    //glm::vec3 dir = camera.GetDir();
    //std::sort(particles_.begin(), particles_.end(), [&](auto a, auto b) {
    //  float adist = glm::dot(a.position - cam_pos, dir);
    //  float bdist = glm::dot(b.position - cam_pos, dir);
    //
    //  return adist > bdist;
    //});

    //Timer loop;
    //for (auto& p : particles_) {
    //  Vertex vertex;
    //
    //  vertex.pos = p.position;
    //  pos_data.push_back(vertex);
    //
    //  color_data.push_back(p.color);
    //  //color_data.push_back(glm::vec4(1.f, .2f, .0f, 0.1f));
    //  size_data.push_back(p.size);
    //  //glm::vec3 up(0.f, 1.f, 0.f);
    //  //up *= 2.f;
    //  //glm::vec3 right(1.f, 0.f, 0.f);
    //  //
    //  //vertex.pos = p.position + -right - up;
    //  //data.push_back(vertex);
    //  //
    //  //vertex.pos = p.position + right - up;
    //  //data.push_back(vertex);
    //  //
    //  //vertex.pos = p.position + -right + up;
    //  //data.push_back(vertex);
    //  //
    //  //vertex.pos = p.position + right + up;
    //  //data.push_back(vertex);
    //}
    //auto ttt = loop.Elapsed();


    //
    //std::vector<glm::vec3> quad_vertices{
    //    {-1, -1, 0}, {1, -1, 0}, {-1, 1, 0}, {1, 1, 0}};

    //if (pos_data.size() > SIZE || size_data.size() > SIZE ||
    //    color_data.size() > SIZE) {
    //  std::cout << "Bad siiiiiiiiiize\n";
    //}
    //
    //Timer memTimer;
    //glBindBuffer(GL_ARRAY_BUFFER, position_vbo_);
    //void* old_data
    //=
    //    glMapBufferRange(GL_ARRAY_BUFFER, 0, SIZE,
    //                     GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
    //                         GL_MAP_UNSYNCHRONIZED_BIT);
    //glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    //
    //memcpy(old_data, pos_data.data(),
    //       pos_data.size() * sizeof(glm::vec3));
    //glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, SIZE);
    //GLboolean succes = glUnmapBuffer(GL_ARRAY_BUFFER);
    //
    //glBindBuffer(GL_ARRAY_BUFFER, color_vbo_);
    //old_data =
    //    glMapBufferRange(GL_ARRAY_BUFFER, 0, SIZE,
    //                     GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
    //                         GL_MAP_UNSYNCHRONIZED_BIT);
    //glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    //
    //memcpy(old_data, color_data.data(), color_data.size() * sizeof(glm::vec4));
    //glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, SIZE);
    //succes = glUnmapBuffer(GL_ARRAY_BUFFER);
    //
    //glBindBuffer(GL_ARRAY_BUFFER, size_vbo_);
    //old_data = glMapBufferRange(GL_ARRAY_BUFFER, 0, SIZE,
    //                            GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
    //                                GL_MAP_UNSYNCHRONIZED_BIT);
    //glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    //
    //memcpy(old_data, size_data.data(), size_data.size() * sizeof(float));
    //glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, SIZE);
    //succes = glUnmapBuffer(GL_ARRAY_BUFFER);


    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDepthFunc(GL_ALWAYS);
    //glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    //shader.uniform("size", particle_size_);
    //shader.uniform("color", color_);
    shader.uniform("ourTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glBindVertexArray(vertex_array_object_);
    //glDrawArrays(GL_POINTS, 0, pos_data.size());
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_POINTS, 0, SIZE);
    glDepthMask(GL_TRUE);
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
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    //auto t = timer.Elapsed();
    //auto tt = memTimer.Elapsed();
    //if (t > 0.010) std::cout << "renderer bottleneck\n";
    //if (time > 1.0f) {
    //  time -= 1.0f;
    //  std::cout << "render time: " << t << " mem time: " << tt << " loop: " << ttt << std::endl;
    //}
  }

  void ParticleSystem::Settings(const ParticleSettings& ps) { settings_ = ps; }
  void ParticleSystem::SetTexture(GLuint tex) { texture_ = tex; }

  void ParticleSystem::Reset() {
    //static std::default_random_engine gen;
    //std::uniform_real_distribution<float> dist(-0.01, 0.01);
    //std::uniform_real_distribution<float> dist1(-0.5, 0.5);
    //
    //std::vector<Particle> particles;
    //particles.reserve(SIZE);
    //
    //int new_particles = SIZE;
    //current_index_ = new_particles;
    //for (int i = 0; i < new_particles; ++i) {
    // glm::vec3 test =
    //     glm::normalize(glm::vec3(dist1(gen), dist1(gen), dist1(gen)));
    //
    // particles.push_back(
    //     {glm::vec4(test, 0.f), glm::vec4(test * dist1(gen), 0.f), glm::vec4(1.0f, 0.2f,
    //     .1f, 0.5f),
    //      0.08f, 0.f});
    //}
    //
    //std::vector<glm::vec4> pos_data;
    //pos_data.reserve(SIZE);
    //std::vector<glm::vec4> vel_data;
    //vel_data.reserve(SIZE);
    //std::vector<glm::vec4> color_data;
    //color_data.reserve(SIZE);
    //std::vector<float> size_data;
    //size_data.reserve(SIZE);
    std::vector<float> time_data(SIZE, 0.f);
    //for () {
      //Vertex vertex;
      //
      //vertex.pos = p.position;
      //pos_data.push_back(p.position);
      ////vel_data.push_back(glm::vec4(0.1f, 1.f, 0.0f, 0.0f));
      //vel_data.push_back(p.velocity);
      //color_data.push_back(p.color);
      ////color_data.push_back(glm::vec4(1.f, .2f, .0f, 0.1f));
      //size_data.push_back(p.size);

      //time_data.push_back(0.0f);
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
    //}

    // Flush opengl errors
    //GLenum err;
    //while ((err = glGetError()) != GL_NO_ERROR)
    //  ;
    //
    //glBindBuffer(GL_ARRAY_BUFFER, position_vbo_);
    // void* old_data =
    //    glMapBufferRange(GL_ARRAY_BUFFER, 0, SIZE * sizeof(glm::vec4),
    //                     GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
    //                         GL_MAP_UNSYNCHRONIZED_BIT);
    // glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    //
    // memcpy(old_data, pos_data.data(),
    //       pos_data.size() * sizeof(glm::vec4));
    // glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, SIZE * sizeof(glm::vec4));
    // GLboolean succes = glUnmapBuffer(GL_ARRAY_BUFFER);
    //
    // glBindBuffer(GL_ARRAY_BUFFER, color_vbo_);
    // old_data =
    //    glMapBufferRange(GL_ARRAY_BUFFER, 0, SIZE * sizeof(glm::vec4),
    //                     GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
    //                         GL_MAP_UNSYNCHRONIZED_BIT);
    // glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    //
    // memcpy(old_data, color_data.data(), color_data.size() * sizeof(glm::vec4));
    // glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, SIZE * sizeof(glm::vec4));
    // succes = glUnmapBuffer(GL_ARRAY_BUFFER);
    //
    // glBindBuffer(GL_ARRAY_BUFFER, size_vbo_);
    // old_data = glMapBufferRange(GL_ARRAY_BUFFER, 0, SIZE * sizeof(float),
    //                            GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
    //                                GL_MAP_UNSYNCHRONIZED_BIT);
    // glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    //
    // memcpy(old_data, size_data.data(), size_data.size() * sizeof(float));
    // glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, SIZE * sizeof(float));
    // succes = glUnmapBuffer(GL_ARRAY_BUFFER);
    //
    // while ((err = glGetError()) != GL_NO_ERROR);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocity_buffer_);
    // old_data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, SIZE * sizeof(glm::vec4),
    //                             GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
    //                                 GL_MAP_UNSYNCHRONIZED_BIT);
    //
    // err = glGetError();
    // if (err != GL_NO_ERROR) {
    //   std::cout << "opengl error\n";
    // }
    // glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    // 
    // memcpy(old_data, vel_data.data(), vel_data.size() * sizeof(glm::vec4));
    // glFlushMappedBufferRange(GL_SHADER_STORAGE_BUFFER, 0, SIZE * sizeof(glm::vec4));
    // succes = glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

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




Timer::Timer() { start_ = std::chrono::high_resolution_clock::now(); }

double Timer::Restart() {
  auto now = std::chrono::high_resolution_clock::now();
  double diff =
      std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_)
          .count();
  start_ = std::chrono::high_resolution_clock::now();
  paused_ = false;
  return diff / 1000000000.0;
}

double Timer::Elapsed() {
  auto now = std::chrono::high_resolution_clock::now();
  if (paused_) now = pause_time_;
  double diff =
      std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_)
          .count();
  return diff / 1000000000.0;
}

void Timer::Pause() {
  if (!paused_) {
    pause_time_ = std::chrono::high_resolution_clock::now();
    paused_ = true;
  }
}

void Timer::Resume() {
  if (paused_) {
    auto now = std::chrono::high_resolution_clock::now();
    start_ += now - pause_time_;
    paused_ = false;
  }
}

/*
void TimerGroup::add(Timer* t)
{
        

}

void TimerGroup::pauseAll()
{
}

void TimerGroup::resumeAll()
{
}

void TimerGroup::restartAll()
{
}
*/