#ifndef ANIMATION_SYSTEM_HPP_
#define ANIMATION_SYSTEM_HPP_

#include <entt.hpp>

#include <iostream>

#include "glob/Animation.hpp"
#include "glob/joint.hpp"

#include "ecs/components.hpp"
#include "shared/shared.hpp"

#include <math.h>
#include "glm/gtx/matrix_interpolation.hpp"
#include "glm/gtx/perpendicular.hpp"

class Engine;

class AnimationSystem {
 private:
  float time_ = 0;

  enum ANIM_MODES { LOOP, MUTE_ALL };

  std::string slide_anims_[4] = {"SlideF", "SlideB", "SlideR", "SlideL"};

  Engine* engine_;

  struct priorityGroup {
    std::vector<glob::Animation*> animations;
    char priority;
  };

  std::vector<priorityGroup> p_groups;

 public:
  void Init(Engine* engine);

  glm::mat3 ConvertToGLM3x3(aiMatrix3x3 aiMat);

  void GetDefaultPose(glm::mat4 parent, glob::Joint* bone,
                      std::vector<glob::Joint>* armature,
                      glm::mat4 globalInverseTransform);

  bool IsAChildOf(int parent, int lookFor, AnimationComponent* ac);

  int GetAnimationByName(std::string name, AnimationComponent* ac);

  int GetActiveAnimationByName(std::string name, AnimationComponent* ac);

  void PlayAnimation(std::string name, float speed, AnimationComponent* ac,
                     char priority, float strength, int mode,
                     int bodyArgument = -1);

  void StopAnimation(std::string name, AnimationComponent* ac);

  void StrengthModulator(AnimationComponent* ac);

  void UpdateEntities(entt::registry& registry, float dt);

  void ReceiveGameEvent(GameEvent event);

  void UpdateAnimations(entt::registry& registry, float dt);

  void Reset();
};

#endif  // ANIMATION_SYSTEM_HPP_
