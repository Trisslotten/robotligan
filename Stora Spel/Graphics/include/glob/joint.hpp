#ifndef GLOB_JOINT_HPP_
#define GLOB_JOINT_HPP_

#include <vector>
#include <string>

namespace glob {
	struct Joint {
		std::vector<int> children;
		glm::vec3 position = glm::vec3(0.f);
		glm::mat4 transform = glm::mat4(0.f);
		char id;
		std::string name;
	};
}

#endif // GLOB_JOINT_HPP_