#ifndef GLOB_JOINT_HPP_
#define GLOB_JOINT_HPP_

#include <vector>
#include <string>

#include "glm/glm.hpp"

namespace glob {
	struct Joint {
		std::vector<int> children;
		glm::mat4 offset = glm::mat4(0.f);
		glm::mat4 transform = glm::mat4(0.f);
		glm::mat4 f_transform = glm::mat4(0.f);
		char id;
		std::string name;
	};
}

#endif // GLOB_JOINT_HPP_