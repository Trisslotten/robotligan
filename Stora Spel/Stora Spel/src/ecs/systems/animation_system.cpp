#include "animation_system.hpp"

#include "engine.hpp"

void AnimationSystem::Init(Engine* engine) { engine_ = engine; }

glm::mat3 AnimationSystem::ConvertToGLM3x3(aiMatrix3x3 aiMat) {
  return {aiMat.a1, aiMat.b1, aiMat.c1, aiMat.a2, aiMat.b2,
          aiMat.c2, aiMat.a3, aiMat.b3, aiMat.c3};
}

void AnimationSystem::GetDefaultPose(glm::mat4 parent, glob::Joint* bone,
                                     std::vector<glob::Joint>* armature,
                                     glm::mat4 globalInverseTransform) {
  glm::mat4 parentTransform = parent;

  glm::mat4 globalTransform = parentTransform * bone->transform;

  bone->f_transform = globalInverseTransform * globalTransform * bone->offset;

  for (int i = 0; i < bone->children.size(); i++) {
    GetDefaultPose(globalTransform, &armature->at(bone->children.at(i)),
                   armature, globalInverseTransform);
  }
}

bool AnimationSystem::IsAChildOf(int parent, int lookFor,
                                 AnimationComponent* ac) {
  for (int j = 0; j < ac->model_data.bones.at(parent).children.size(); j++) {
    if (ac->model_data.bones.at(parent).children.at(j) == lookFor) {
      return true;
    }
    bool ret =
        IsAChildOf(ac->model_data.bones.at(parent).children.at(j), lookFor, ac);
    if (ret) {
      return true;
    }
  }
  return false;
}

int AnimationSystem::GetAnimationByName(std::string name,
                                        AnimationComponent* ac) {
  std::vector<glob::Animation>* anims = &ac->model_data.animations;
  for (int i = 0; i < anims->size(); i++) {
    if (anims->at(i).name_ == name) {
      return i;
    }
  }
  return -1;
}

int AnimationSystem::GetActiveAnimationByName(std::string name,
                                              AnimationComponent* ac) {
  std::vector<glob::Animation*>* anims = &ac->active_animations;
  for (int i = 0; i < anims->size(); i++) {
    if (anims->at(i)->name_ == name) {
      return i;
    }
  }
  return -1;
}

void AnimationSystem::PlayAnimation(std::string name, float speed,
                                    AnimationComponent* ac, char priority,
                                    float strength, int mode,
                                    int bodyArgument) {
  int anim = GetAnimationByName(name, ac);
  if (anim == -1) {
    // std::cout << "WARNING: Could not find animation " << name << "!\n";
    return;
  }

  for (int i = 0; i < ac->active_animations.size(); i++) {
    if (ac->active_animations.at(i) == &ac->model_data.animations.at(anim)) {
      // std::cout << "WARNING: The animation \"" <<
      // ac->model_data.animations.at(anim).name_ << "\" is already playing,
      // cannot stack the same animation!\n";
      return;
    }
  }
  glob::Animation* anim_ptr = &ac->model_data.animations.at(anim);
  ac->active_animations.push_back(anim_ptr);
  anim_ptr->speed_ = speed;
  anim_ptr->priority_ = priority;
  anim_ptr->mode_ = mode;
  anim_ptr->playing_ = true;

  if (bodyArgument != -1) {
    anim_ptr->body_argument_ = bodyArgument;
  }

  bool found = false;
  for (auto& group : p_groups) {
    if (group.priority == priority) {
      group.animations.push_back(anim_ptr);
      found = true;
      break;
	}
  }
  if (!found) {
    priorityGroup pg;
    pg.priority = priority;
    pg.animations.push_back(anim_ptr);
    p_groups.push_back(pg);
  }

  // std::cout << "Playing " << anim_ptr->name_ << "\n";
}

void AnimationSystem::StopAnimation(std::string name, AnimationComponent* ac) {
  int anim = GetActiveAnimationByName(name, ac);
  if (anim == -1) {
    // std::cout << "WARNING: Could not find animation " << name << "!\n";
    return;
  }
  ac->active_animations.at(anim)->playing_ = false;
}

void AnimationSystem::StrengthModulator(AnimationComponent* ac) {
  for (auto& group : p_groups) {
    float totStrength = 0;
    for (auto& anim : group.animations) {
      totStrength += anim->strength_;
	}
    for (auto& anim : group.animations) {
      anim->strength_ /= totStrength;
    }
  }
}

void AnimationSystem::UpdateEntities(entt::registry& registry, float dt) {
  auto players =
      registry.view<AnimationComponent, PlayerComponent,
                    TransformComponent, PhysicsComponent>();
  for (auto& entity : players) {
    auto& ac = players.get<AnimationComponent>(entity);
    auto& pl = players.get<PlayerComponent>(entity);
    auto& t = players.get<TransformComponent>(entity);
    auto& ph = players.get<PhysicsComponent>(entity);

    PlayAnimation("Resting", 0.5f, &ac, 10, 1.f, LOOP);

    glm::vec3 lookDir = t.Forward() * glm::vec3(1.f, 0.f, 1.f);
    glm::vec3 h_lookDir;
    glm::vec3 moveDir = glm::normalize(ph.velocity);

    // SLIDE ANIMATIONS
    if (pl.sprinting) {
      float strength = 0.f;
      float totStrength = 0.f;
      float cutoffSpeed = 5.f;
      float speed = glm::length(ph.velocity);
      for (int i = 0; i < 4; i++) {
        int anim = GetActiveAnimationByName(slide_anims_[i], &ac);
        switch (i) {
          case 0: {  // F
            h_lookDir = glm::normalize(lookDir);
            strength = std::clamp(glm::dot(h_lookDir, moveDir), 0.f, 1.f);
            break;
          }
          case 1: {  // B
            h_lookDir = glm::normalize(lookDir);
            strength = abs(std::clamp(glm::dot(h_lookDir, moveDir), -1.f, 0.f));
            break;
          }
          case 2: {  // R
            h_lookDir =
                glm::normalize(glm::cross(lookDir, glm::vec3(0.f, 1.f, 0.f)));
            strength = std::clamp(glm::dot(h_lookDir, moveDir), 0.f, 1.f);
            break;
          }
          case 3: {  // L
            h_lookDir =
                glm::normalize(glm::cross(lookDir, glm::vec3(0.f, 1.f, 0.f)));
            strength = abs(std::clamp(glm::dot(h_lookDir, moveDir), -1.f, 0.f));
            break;
          }
        }
        if (speed < cutoffSpeed) {
          strength = std::clamp(strength - pl.sprint_coeff, 0.f, 1.f);
          pl.sprint_coeff += 1.f * dt;
        } else {
          pl.sprint_coeff -= 1.f * dt;
        }
        pl.sprint_coeff = std::clamp(pl.sprint_coeff, 0.f, 1.f);
        ac.active_animations.at(anim)->strength_ = strength;
        totStrength += strength;
      }
      if (speed < cutoffSpeed) {
        int f = GetActiveAnimationByName("SlideF", &ac);
        int b = GetActiveAnimationByName("SlideB", &ac);
        float defaultPoseModifier =
            std::clamp(1.f - totStrength, 0.f, 1.f) / 2.f;
        ac.active_animations.at(f)->strength_ = defaultPoseModifier;
        ac.active_animations.at(b)->strength_ = defaultPoseModifier;
      }
    }

    // RUNNING ANIMATIONS
  }
}
void AnimationSystem::ReceiveGameEvent(GameEvent event) {
  auto registry = engine_->GetCurrentRegistry();
  switch (event.type) {
    case GameEvent::GOAL: {
      break;
    };
    case GameEvent::JUMP: {
      break;
    };
    case GameEvent::LAND: {
      break;
    };
    case GameEvent::RUN_START: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.sprint_start.player_id) {
          auto& ac = view.get<AnimationComponent>(entity);
          auto& pc = view.get<PlayerComponent>(entity);
          pc.running = true;
          PlayAnimation("Run", 1.f, &ac, 15, 1.f, LOOP);

          break;
        }
      }
      break;
    };
    case GameEvent::RUN_END: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.sprint_start.player_id) {
          auto& ac = view.get<AnimationComponent>(entity);
          auto& pc = view.get<PlayerComponent>(entity);
          pc.running = false;
          StopAnimation("Run", &ac);

          break;
        }
      }
      break;
    };
    case GameEvent::SPRINT_START: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.sprint_start.player_id) {
          auto& pc = view.get<PlayerComponent>(entity);
          auto& ac = view.get<AnimationComponent>(entity);
          pc.sprinting = true;
          PlayAnimation("SlideF", 1.f, &ac, 20, 0.f, LOOP);
          PlayAnimation("SlideB", 1.f, &ac, 20, 0.f, LOOP);
          PlayAnimation("SlideR", 1.f, &ac, 20, 0.f, LOOP);
          PlayAnimation("SlideL", 1.f, &ac, 20, 0.f, LOOP);

          break;
        }
      }
      break;
    };
    case GameEvent::SPRINT_END: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.sprint_end.player_id) {
          auto& pc = view.get<PlayerComponent>(entity);
          auto& ac = view.get<AnimationComponent>(entity);
          pc.sprinting = false;
          StopAnimation("SlideF", &ac);
          StopAnimation("SlideB", &ac);
          StopAnimation("SlideR", &ac);
          StopAnimation("SlideL", &ac);
          pc.sprint_coeff = 0.f;

          break;
        }
      }
      break;
    }
  };
}

void AnimationSystem::UpdateAnimations(entt::registry& registry, float dt) {
  UpdateEntities(registry, dt);

  auto animation_entities = registry.view<AnimationComponent>();
  for (auto& entity : animation_entities) {
    auto& a = animation_entities.get(entity);

    StrengthModulator(&a);

    int rootBone = a.model_data.armatureRoot;

    std::vector<glm::mat4> boneTransforms;
    std::vector<int> bonePriorities;
    for (int i = 0; i < a.model_data.bones.size(); i++) {
      boneTransforms.push_back(glm::mat4(1.f));
      bonePriorities.push_back(1);
    }

    time_ += dt;

    for (int i = 0; i < a.active_animations.size(); i++) {
      glob::Animation* anim = a.active_animations.at(i);
      bool removedAnimation = false;
      if (anim->current_frame_time_ >= anim->duration_) {
        // Loop for the time being
        if (anim->mode_ == LOOP) {
          anim->current_frame_time_ = 0;
          for (int j = 0; j < anim->channels_.size(); j++) {
            anim->channels_.at(j).current_position_pos = 0;
            anim->channels_.at(j).current_rotation_pos = 0;
            anim->channels_.at(j).current_scaling_pos = 0;
          }
        } else if (anim->mode_ == MUTE_ALL) {
          anim->playing_ = false;
        }
      }
      if (!anim->playing_) {
        // reset important data
        anim->body_argument_ = -1;
        anim->current_frame_time_ = 0.f;
        anim->playing_ = false;
        for (int j = 0; j < anim->channels_.size(); j++) {
          glob::Channel* channel = &anim->channels_.at(j);
          channel->current_scaling_pos = 0;
          channel->current_position_pos = 0;
          channel->current_rotation_pos = 0;
        }
        // remove from list
        a.active_animations.erase(a.active_animations.begin() + i);
        i--;
        removedAnimation = true;
		//remove from priority group
        for (auto& group : p_groups) {
          for (int a_i = 0; a_i < group.animations.size(); a_i++) {
            if (anim == group.animations.at(a_i)) {
              group.animations.erase(group.animations.begin() + a_i);
              break;
			}
		  }
		}
      } else {
        anim->current_frame_time_ += anim->speed_ * anim->tick_per_second_ * dt;
      }

      if (!removedAnimation) {
        // hell (aka bone rotation update)
        for (int j = 0; j < anim->channels_.size();
             j++) {  // all channels (bones)
          glob::Channel* channel = &anim->channels_.at(j);
          int jointId = (int)channel->boneID;

          if (jointId != rootBone) {
            int scalingPos = channel->current_scaling_pos;
            aiVectorKey scalingKey = channel->scaling_keys.at(scalingPos);
            glm::mat4 scaling = glm::scale(glm::vec3(
                scalingKey.mValue.x, scalingKey.mValue.y, scalingKey.mValue.z));

            if (anim->current_frame_time_ >= scalingKey.mTime) {
              channel->current_scaling_pos++;
            }

            int rotationPos = channel->current_rotation_pos;
            aiQuatKey rotationKey = channel->rotation_keys.at(rotationPos);
            glm::mat4 rotation =
                ConvertToGLM3x3(rotationKey.mValue.GetMatrix());

            if (anim->current_frame_time_ >= rotationKey.mTime) {
              channel->current_rotation_pos++;
            }

            int positionPos = channel->current_position_pos;
            aiVectorKey positionKey = channel->position_keys.at(positionPos);
            glm::mat4 position = glm::translate(
                glm::vec3(positionKey.mValue.x, positionKey.mValue.y,
                          positionKey.mValue.z));

            if (anim->current_frame_time_ >= positionKey.mTime) {
              channel->current_position_pos++;
            }

            glm::mat4 combPRS = position * rotation * scaling;

            glm::mat4 finalMat = combPRS * anim->strength_;

            if (anim->mode_ == LOOP) {
              if (anim->priority_ >
                  bonePriorities.at(jointId)) {  // priority override
                if (anim->body_argument_ != -1) {
                  if (IsAChildOf(anim->body_argument_, jointId,
                                 &a)) {  // body argument success, override
                    bonePriorities.at(jointId) = anim->priority_;
                    a.model_data.bones.at(jointId).transform = finalMat;
                  }
                } else {  // No body argument, override
                  bonePriorities.at(jointId) = anim->priority_;
                  a.model_data.bones.at(jointId).transform = finalMat;
                }
              } else if (anim->priority_ ==
                         bonePriorities.at(jointId)) {  // blend
                if (anim->body_argument_ != -1) {
                  if (IsAChildOf(anim->body_argument_, jointId,
                                 &a)) {  // body argument success, blend
                    a.model_data.bones.at(jointId).transform += finalMat;
                  }
                } else {  // No body argument, blend
                  a.model_data.bones.at(jointId).transform += finalMat;
                }
              }
            } else if (anim->mode_ == MUTE_ALL) {
              if (anim->priority_ >= bonePriorities.at(jointId)) {  // override
                bonePriorities.at(jointId) = anim->priority_;
                a.model_data.bones.at(jointId).transform = finalMat;
              }
            }
          }
        }
      }
    }

    GetDefaultPose(glm::mat4(1.f), &a.model_data.bones.at(rootBone),
                   &a.model_data.bones, a.model_data.globalInverseTransform);

    for (auto bone : a.model_data.bones) {
      boneTransforms.at(bone.id) = bone.f_transform;
    }
    a.bone_transforms = boneTransforms;
  }
}


void AnimationSystem::Reset() { p_groups.clear();}