#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>
#include "glob/camera.hpp"

struct CameraComponent {
  Camera* cam;
  glm::vec3 offset;
  float yaw_ = 0;
  float pitch_ = 0;

  void clampAngles() {
    pitch_ = glm::clamp(pitch_, -90.f, 90.f);
    if (yaw_ > 360.f || yaw_ < -360.f) {
      yaw_ = 0;	
	}
  }

  void SetAngles(float yaw, float pitch) {
    yaw_ = yaw;
    pitch_ = pitch;
    clampAngles();
    cam->SetAnglesViaDegrees(yaw_, pitch_);
  }
  void AddAngles(float yaw, float pitch) {
    yaw_ += yaw;
    pitch_ -= pitch;
    clampAngles();
    cam->SetAnglesViaDegrees(yaw_, pitch_);
  }
};

#endif  // CAMERA_H_