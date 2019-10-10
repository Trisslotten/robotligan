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
		int current_position_pos = 0;
		int current_rotation_pos = 0;
		int current_scaling_pos = 0;
		std::vector <aiVectorKey> position_keys;
		std::vector <aiQuatKey> rotation_keys;
		std::vector <aiVectorKey> scaling_keys;

	};

	class Animation {
	private:

	public:
		std::string name_ = "NO_NAME";
		float current_frame_time_ = 0.f;
		std::vector<Channel> channels_;
		float tick_per_second_ = 24.f;
		float duration_ = 0.f;

		float speed_ = 1.f;
		bool playing_ = false;
		float strength_ = 1.f;

		std::vector<glm::mat4> armature_transform_;

		char priority_ = 10;
		int mode_ = 0;
		int body_argument_ = -1;
	};

}  // namespace glob

#endif  // ANIMATION_HPP_