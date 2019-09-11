#include "camera.hpp"

//Private------------------------------------------------------------------------------------------
void Camera::UpdateDirectionalVectors() {
	
	//Calculate the direction using yaw and pitch
	this->cam_direction_.x = cos(glm::radians(this->yaw_)) * cos(glm::radians(this->pitch_));
	this->cam_direction_.y = sin(glm::radians(this->pitch_));
	this->cam_direction_.z = sin(glm::radians(this->yaw_)) * cos(glm::radians(this->pitch_));
	this->cam_direction_ = glm::normalize(this->cam_direction_);

	//Calcualte what is right using up and direction
	this->cam_right_ = glm::cross(this->world_up_, this->cam_direction_);

	//Calculate what is up using right and direction
	this->cam_up_ = glm::cross(this->cam_right_, this->cam_direction_);

}

//Public-------------------------------------------------------------------------------------------

Camera::Camera(
	glm::vec3 in_pos,
	glm::vec3 in_target,
	float in_fov_deg,
	float in_aspect,
	float in_nearplane,
	float in_farplane
) {

	//STEP 1: Fix values for and calculate view-matrix
	this->position_ = in_pos;
	this->world_up_ = glm::vec3(0.0f, 1.0f, 0.0f);

	//Calculate yaw and pitch values
	this->LookAtPoint(in_target);

	///Calculate directional vectors
	this->UpdateDirectionalVectors();

	//Calculate view matrix
	this->view_mat_ = glm::lookAt(
		this->position_,
		this->position_ + this->cam_direction_,
		this->world_up_
	);

	//STEP 2: Calculate perpective-matrix
	this->perspective_mat_ = glm::perspective(
		glm::radians(in_fov_deg),
		in_aspect,
		in_nearplane,
		in_farplane
	);

}

Camera::~Camera() {}

glm::mat4 Camera::GetViewPerspectiveMatrix() const {
	return this->perspective_mat_ * this->view_mat_;
}

void Camera::LookAtPoint(glm::vec3 in_target) {

	//Create vector from camera to target point
	glm::vec3 temp_dir = glm::normalize(in_target - this->position_);
	glm::vec3 temp_dir_f = temp_dir;

	//Flatten it onto the xz-plane
	temp_dir_f.y = 0.0f;
	temp_dir_f = glm::normalize(temp_dir_f);

	//Calculate the oriented angle between it and (1,0,0)
	//That is the camera's yaw 
	this->yaw_ = glm::orientedAngle(
		temp_dir_f,
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	//Check the angle between the flattened and unflattened angle
	//That is the camera's pitch
	this->pitch_ = glm::orientedAngle(
		temp_dir_f,
		temp_dir,
		glm::cross(temp_dir_f, temp_dir)
	);

}