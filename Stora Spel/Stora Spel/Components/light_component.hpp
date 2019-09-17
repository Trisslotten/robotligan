#ifndef LIGHT_COMPONENT_HPP_
#define LIGHT_COMPONENT_HPP_

#include <glm/glm.hpp>

struct LightComponent {
	glm::vec3 color = glm::vec3(1.f, 1.f, 1.f);
	float radius = 10.f;
	float ambient = 0.5f;
};

#endif //LIGHT_COMPONENT_HPP_