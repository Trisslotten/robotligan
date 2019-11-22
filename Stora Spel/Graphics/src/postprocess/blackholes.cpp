#include "blackholes.hpp"
#include <glob\window.hpp>

void glob::BlackHoles::Create(glm::vec3 position) {
  black_holes_.push_back({position});
}

void glob::BlackHoles::Update(Camera camera) {
  uniform_positions_.clear();
  uniform_radii_.clear();

  for (int i = 0; i < black_holes_.size(); i++) {
    auto& curr = black_holes_[i];

    float elapsed = curr.timer.Elapsed();

    if (curr.timer.Elapsed() < 5.0) {
      glm::vec4 view = camera.GetViewMatrix() * glm::vec4(curr.position, 1);
      glm::vec4 clip = camera.GetProjectionMatrix() * view;

      float radius = 7.f;
      glm::vec4 offset =
          camera.GetProjectionMatrix() * (view + glm::vec4(radius, 0, 0, 0));
      glm::vec3 offset_ndc = glm::vec3(offset) / offset.w;

      glm::vec3 ndc = glm::vec3(clip) / clip.w;
      float ndc_radius = 0.5 * glm::length(offset_ndc - ndc);

      if (clip.w > 0) {
        auto ws = glob::window::GetWindowDimensions();

        uniform_positions_.push_back(ndc * 0.5f + 0.5f);
        uniform_radii_.push_back(ndc_radius * ws.x);
      }
    } else {
      black_holes_.erase(black_holes_.begin() + i);
      i--;
    }
  }
}

void glob::BlackHoles::SetUniforms(ShaderProgram& shader) {
  const int max_black_holes = 8;
  int count = glm::min(max_black_holes, (int)uniform_positions_.size());

  shader.uniform("blackhole_count", count);
  shader.uniformv("blackhole_positions", count, uniform_positions_.data());
  shader.uniformv("blackhole_radii", count, uniform_radii_.data());
}
