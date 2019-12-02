#include "shockwaves.hpp"
#include <glob\window.hpp>

namespace glob {

void Shockwaves::Create(glm::vec3 position, float duration, float size) {
  if (duration > 0 && size > 0) {
    shockwaves_.push_back({position, duration, size});
    shockwaves_.back().timer.Restart();
  }
}

void Shockwaves::Update(Camera camera) {
  uniform_positions_.clear();
  uniform_time_ratios_.clear();
  uniform_radii_.clear();

  for (int i = 0; i < shockwaves_.size(); i++) {
    Shockwave& curr = shockwaves_[i];

    float elapsed = curr.timer.Elapsed();
    float time_ratio = elapsed / curr.duration;

    if (time_ratio < 1.0) {
      glm::vec4 view = camera.GetViewMatrix() * glm::vec4(curr.position, 1);
      glm::vec4 clip = camera.GetProjectionMatrix() * view;

      float radius = curr.size * time_ratio * time_ratio;
      glm::vec4 offset =
          camera.GetProjectionMatrix() * (view + glm::vec4(radius, 0, 0, 0));
      glm::vec3 offset_ndc = glm::vec3(offset) / offset.w;

      glm::vec3 ndc = glm::vec3(clip) / clip.w;
      float ndc_radius = 0.5 * glm::length(offset_ndc - ndc);
      
      
      if(clip.w > 0) {
        auto ws = glob::window::GetWindowDimensions();

        uniform_positions_.push_back(ndc * 0.5f + 0.5f);
        uniform_time_ratios_.push_back(time_ratio);
        uniform_radii_.push_back(ndc_radius * ws.x);
      }
    } else {
      shockwaves_.erase(shockwaves_.begin() + i);
      i--;
    }
  }
}

void Shockwaves::SetUniforms(ShaderProgram& shader) {
  const int max_shockwaves = 8;
  int count = glm::min(max_shockwaves, (int)uniform_positions_.size());

  shader.uniform("shockwave_count", count);
  shader.uniformv("shockwave_positions", count, uniform_positions_.data());
  shader.uniformv("shockwave_time_ratios", count, uniform_time_ratios_.data());
  shader.uniformv("shockwave_radii", count, uniform_radii_.data());
  
}

}  // namespace glob