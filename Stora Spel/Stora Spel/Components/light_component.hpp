#ifndef LIGHT_COMPONENT_HPP_
#define LIGHT_COMPONENT_HPP_

#include <glm/glm.hpp>

struct LightComponent {
	glm::vec4 Color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	float radius = 10.f;
};

#endif //LIGHT_COMPONENT_HPP_