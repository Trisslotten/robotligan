#ifndef ANIMATION_SYSTEM_HPP_
#define ANIMATION_SYSTEM_HPP_


#include <entt.hpp>

#include <iostream>

#include "glob/joint.hpp"
#include "glob/Animation.hpp"

#include "shared/shared.hpp"
#include "ecs/components.hpp"

#include "glm/gtx/matrix_interpolation.hpp"
#include "glm/gtx/perpendicular.hpp"
#include <math.h>

class Engine;

class AnimationSystem {
private:
	float time_ = 0;
	bool p = false;

	enum ANIM_MODES { LOOP, MUTE_ALL };
	bool first = true;

	std::string slide_anims_[4] = { "SlideF", "SlideB", "SlideR", "SlideL" };

	Engine* engine_;


public:

	void init(Engine* engine);

	glm::mat3 convertToGLM3x3(aiMatrix3x3 aiMat);

	void getDefaultPose(glm::mat4 parent, glob::Joint* bone, std::vector<glob::Joint>* armature, glm::mat4 globalInverseTransform);

	bool isAChildOf(int parent, int lookFor, AnimationComponent* ac);

	int getAnimationByName(std::string name, AnimationComponent* ac);

	int getActiveAnimationByName(std::string name, AnimationComponent* ac);

	void playAnimation(std::string name, float speed, AnimationComponent* ac, char priority, float strength, int mode, int bodyArgument = -1);

	void stopAnimation(std::string name, AnimationComponent* ac);

	void strengthModulator(AnimationComponent* ac);

	void updateEntities(entt::registry& registry, float dt);

	void receiveGameEvent(GameEvent event);

	void UpdateAnimations(entt::registry& registry, float dt);
};

#endif  // ANIMATION_SYSTEM_HPP_
