#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

class Camera {
private:

	//Variables for the view matrix
	glm::vec3 position_;			//Camera position

	float yaw_;						//Yaw is for looking left/right
	float pitch_;					//Pitch is for looking up/down
	glm::vec3 cam_direction_;		//Vector FROM camera in the direction its looking

	glm::vec3 world_up_;			//Up in the world
	glm::vec3 cam_right_;			//Straight right of the camera
	glm::vec3 cam_up_;				//Direction of the top of the camera

	//Variables for the perpective matrix
	float fov_;
	float nearplane_;
	float farplane_;
	

	//Matrices
	glm::mat4 view_mat_;			//View matrix
	glm::mat4 perspective_mat_;		//Perspective matrix

	//Functions
	void UpdateDirectionalVectors();

public:
	Camera(
		glm::vec3 in_pos,
		glm::vec3 in_target,
		float in_fov_deg,
		float in_aspect,
		float in_nearplane,
		float in_farplane
	);
	~Camera();

	glm::mat4 GetViewPerspectiveMatrix() const;

	void LookAtPoint(glm::vec3 in_target);
};

#endif // !CAMERA_HPP
