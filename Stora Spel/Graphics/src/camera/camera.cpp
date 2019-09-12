#include "glob/camera.hpp"

// Private------------------------------------------------------------------------------------------
void Camera::UpdateDirectionalVectors() {
  // Calculate the direction using yaw and pitch
  this->cam_direction_.x = cos(this->yaw_) * cos(this->pitch_);
  this->cam_direction_.y = sin(this->pitch_);
  this->cam_direction_.z = sin(this->yaw_) * cos(this->pitch_);
  this->cam_direction_ = glm::normalize(this->cam_direction_);

  // Calcualte what is right using up and direction
  this->cam_right_ = glm::cross(this->world_up_, this->cam_direction_);

  // Calculate what is up using right and direction
  this->cam_up_ = glm::cross(this->cam_right_, this->cam_direction_);
}

void Camera::UpdateViewMatrix() {
  this->view_mat_ = glm::lookAt(
      this->position_, this->position_ + this->cam_direction_, this->world_up_);
}

// Public-------------------------------------------------------------------------------------------

Camera::Camera(glm::vec3 in_pos, glm::vec3 in_target, float in_fov_deg,
               float in_aspect, float in_nearplane, float in_farplane) {
  // STEP 1: Fix values for and calculate view-matrix
  this->position_ = in_pos;
  this->world_up_ = glm::vec3(0.0f, 1.0f, 0.0f);

  // Calculate yaw and pitch values
  this->LookAtPoint(in_target);

  /// Calculate directional vectors
  // this->UpdateDirectionalVectors();		//NTS: Call moved into
  // LookAtPoint()

  // Calculate view matrix
  this->UpdateViewMatrix();

  // STEP 2: Calculate perpective-matrix
  this->perspective_mat_ = glm::perspective(glm::radians(in_fov_deg), in_aspect,
                                            in_nearplane, in_farplane);
}

Camera::~Camera() {}

glm::vec3 Camera::GetPosition() const { return this->position_; }

glm::mat4 Camera::GetViewPerspectiveMatrix() const {
  return this->perspective_mat_ * this->view_mat_;
}

void Camera::MoveCamera(glm::vec3 in_vec) {
  this->position_.x += in_vec.x;
  this->position_.y += in_vec.y;
  this->position_.z += in_vec.z;
}

void Camera::LookAtPoint(glm::vec3 in_target) {
  // Create vector from camera to target point
  glm::vec3 temp_dir = glm::normalize(in_target - this->position_);

  // Flatten a copy of it onto the xz-plane
  glm::vec3 temp_dir_f = temp_dir;
  temp_dir_f.y = 0.0f;
  temp_dir_f = glm::normalize(temp_dir_f);

  // Calculate the oriented angle between it and (1,0,0)
  // That is the camera's yaw
  this->yaw_ = glm::orientedAngle(temp_dir_f, glm::vec3(1.0f, 0.0f, 0.0f),
                                  glm::vec3(0.0f, 1.0f, 0.0f));

  // Calculate the reference vector
  // The vector needs to be the product of a ccw rotation
  // If the target's y-value is higher than our position we are looking up
  glm::vec3 ref_vec = glm::vec3(0.0f);
  if (in_target.y > this->position_.y) {
    // Cross from the flat vector to the actual one
    ref_vec = glm::cross(temp_dir_f, temp_dir);
  } else {
    // Otherwise cross from the actual vector to the flat one
    ref_vec = glm::cross(temp_dir, temp_dir_f);
  }

  // Check the angle between the flattened and unflattened angle
  // That is the camera's pitch
  this->pitch_ = glm::orientedAngle(temp_dir_f, temp_dir, ref_vec);

  // Update the directional vectors and then the view matrix
  this->UpdateDirectionalVectors();
  this->UpdateViewMatrix();
}

void Camera::TurnCameraViaDegrees(float in_yaw_deg, float in_pitch_deg) {
  this->yaw_ += glm::radians(in_yaw_deg);
  this->pitch_ += glm::radians(in_pitch_deg);
  this->UpdateDirectionalVectors();
  this->UpdateViewMatrix();
}

void Camera::TurnCameraViaRadians(float in_yaw_rad, float in_pitch_rad) {
  this->yaw_ += in_yaw_rad;
  this->pitch_ += in_pitch_rad;
  this->UpdateDirectionalVectors();
  this->UpdateViewMatrix();
}
