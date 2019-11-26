#include "ecs/components/animation_component.hpp"
#include "ecs/systems/animation_system.hpp"

namespace PAC {

const char prioLook = 21;
const char prioSlide = 20;
const char prioRun = 15;
const char prioKick = 14;
const char prioShoot = 14;
const char prioJump = 25;

void playLookAnims(AnimationSystem* system, AnimationComponent& ac,
                   bool local) {
  std::vector<int>* include = &ac.model_data.upperBody;
  std::vector<int>* exclude = &ac.model_data.arms;
  if (!local) {
    system->PlayAnimation("LookUp", 0.f, &ac, prioLook, 0.f, system->LOOP, include,
                          exclude);
    system->PlayAnimation("LookDown", 0.f, &ac, prioLook, 0.f, system->LOOP,
                          include, exclude);
    system->PlayAnimation("LookLeft", 0.f, &ac, prioLook, 0.f, system->LOOP,
                          include, exclude);
    system->PlayAnimation("LookRight", 0.f, &ac, prioLook, 0.f, system->LOOP,
                          include, exclude);
    system->PlayAnimation("LookAhead", 1.f, &ac, prioLook, 0.f, system->LOOP,
                          include, exclude);
  }
}

void stopLookAnims(AnimationSystem* system, AnimationComponent& ac,
                   bool local) {
  if (!local) {
    system->StopAnimation("LookUp", &ac);
    system->StopAnimation("LookDown", &ac);
    system->StopAnimation("LookLeft", &ac);
    system->StopAnimation("LookRight", &ac);
    system->StopAnimation("LookAhead", &ac);
  }
}

void playRunAnims(AnimationSystem* system, AnimationComponent& ac, bool local) {
  system->PlayAnimation("Run", 1.f, &ac, prioRun, 1.f, system->LOOP);
}

void stopRunAnims(AnimationSystem* system, AnimationComponent& ac, bool local) {
  system->StopAnimation("Run", &ac);
}

void playSlideAnims(AnimationSystem* system, AnimationComponent& ac,
                    bool local) {
  if (local) {
    system->PlayAnimation("Slide", 1.f, &ac, prioSlide, 1.f, system->LOOP);
  } else {
    system->PlayAnimation("SlideF", 1.f, &ac, prioSlide, 0.f, system->LOOP);
    system->PlayAnimation("SlideB", 1.f, &ac, prioSlide, 0.f, system->LOOP);
    system->PlayAnimation("SlideR", 1.f, &ac, prioSlide, 0.f, system->LOOP);
    system->PlayAnimation("SlideL", 1.f, &ac, prioSlide, 0.f, system->LOOP);
  }
}

void stopSlideAnims(AnimationSystem* system, AnimationComponent& ac,
                    bool local) {
  if (local) {
    system->StopAnimation("Slide", &ac);
  } else {
    system->StopAnimation("SlideF", &ac);
    system->StopAnimation("SlideB", &ac);
    system->StopAnimation("SlideR", &ac);
    system->StopAnimation("SlideL", &ac);
  }
}

void playKickAnims(AnimationSystem* system, AnimationComponent& ac,
                   bool local) {
  if (local) {
    system->PlayAnimation("Kick", 1.f, &ac, prioKick, 1.f, system->MUTE_ALL);
  } else {
    system->PlayAnimation("Kick", 4.f, &ac, prioKick, 1.f, system->PARTIAL_MUTE,
                  &ac.model_data.upperBody);
  }
}

void playShootAnims(AnimationSystem* system, AnimationComponent& ac,
                   bool local) {
  if (local) {
    system->PlayAnimation("Shoot", 1.f, &ac, prioShoot, 1.f, system->MUTE_ALL);
  } else {
    system->PlayAnimation("Shoot", 4.f, &ac, prioShoot, 1.f, system->PARTIAL_MUTE,
                  &ac.model_data.upperBody);
  }
}

void playJumpAnims(AnimationSystem* system, AnimationComponent& ac,
	bool local) {
  system->PlayAnimation("JumpStart", 0.5f, &ac, prioJump, 1.f, system->LOOP);
  system->PlayAnimation("JumpEnd", 0.5f, &ac, prioJump, 0.f, system->LOOP);
}

void stopJumpAnims(AnimationSystem* system, AnimationComponent& ac,
                    bool local) {
  system->StopAnimation("JumpStart", &ac);
  system->StopAnimation("JumpEnd", &ac);
}
}  // namespace PAC