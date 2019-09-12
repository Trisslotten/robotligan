#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>
#include "glob/camera.hpp"

const float pi = 3.14159265358979323846f;

struct CameraComponent {
  Camera* cam_ = nullptr;
  glm::vec3 offset_ = glm::vec3(0,0,0);
  float yaw_ = 0;
  float pitch_ = 0;
  bool clamped_ = true;

  void clampAngles() { //prevent camera from spinning around an axis infinitely if clamped.
    if (clamped_) {
      pitch_ = glm::clamp(pitch_, glm::radians(-89.0f), glm::radians(89.0f));
      if (yaw_ > pi * 2.f || yaw_ < pi * -2.f) {
        yaw_ = 0.0f;
      }
	}
  }

  void SetAngles(float yaw, float pitch) {
    yaw_ = yaw;
    pitch_ = pitch;
    clampAngles();
    cam_->SetAnglesViaRadians(yaw_, pitch_);
  }
  void AddAngles(float yaw, float pitch) {
    yaw_ += yaw;
    pitch_ -= pitch;
    clampAngles();
    cam_->SetAnglesViaRadians(yaw_, pitch_);
  }
};

#endif  // CAMERA_H_