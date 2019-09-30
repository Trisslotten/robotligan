#ifndef ANIMATION_HPP_
#define ANIMATION_HPP_

#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace glob {

	struct Channel {
		char boneID = 0;

		std::vector <aiVectorKey> positionKeys;
		std::vector <aiQuatKey> rotationKeys;
		std::vector <aiVectorKey> scalingKeys;
	};

	class Animation {
	private:

	public:
		std::string name_;
		float current_frame_time_ = 0.f;
		std::vector<Channel> channels_;
		float tick_per_second_ = 24.f;
		float duration_ = 0.f;
	};

}  // namespace glob

#endif  // ANIMATION_HPP_