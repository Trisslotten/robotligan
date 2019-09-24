#ifndef ANIMATION_HPP_
#define ANIMATION_HPP_

#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace glob {

	struct JointTransform {
		char jointID = 0;
		glm::mat4 transform;
	};

	struct Keyframe {
		float frame = 0.f;
		float value = 0.f;

		float b1 = 0.f;//for future bezier curves
		float b2 = 0.f;
		
		std::vector<JointTransform> transforms;
	};

	class Animation {
	private:
		float current_frame_ = 0.f;
		std::vector<Keyframe> key_frames_;
		std::string name_;
		float tick_per_second_ = 24.f;
		float duration_ = 0.f;

	public:

	};

}  // namespace glob

#endif  // ANIMATION_HPP_