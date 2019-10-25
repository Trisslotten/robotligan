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
  if (parent == lookFor) {
    return true;
  }

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

bool AnimationSystem::IsIncluded(int bone, std::vector<int>* included,
                                 std::vector<int>* excluded) {
  for (int i = 0; i < excluded->size(); i++) {
    if (bone == excluded->at(i)) {
      return false;
    }
  }
  for (int i = 0; i < included->size(); i++) {
    if (bone == included->at(i)) {
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
                                    std::vector<int>* bodyInclude,
                                    std::vector<int>* bodyExclude) {
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

  if (bodyInclude != nullptr) {
    for (int i = 0; i < bodyInclude->size(); i++) {
      bool found = false;
      for (int j = 0; j < anim_ptr->body_include_->size(); j++) {
        if (bodyInclude->at(i) == anim_ptr->body_include_->at(j)) {
          found = true;
          break;
        }
      }
      if (!found) {
        if (bodyExclude != nullptr) {
          bool excluded = false;
          for (int j = 0; j < bodyExclude->size(); j++) {
            if (bodyInclude->at(i) == bodyExclude->at(j)) {
              excluded = true;
              break;
            }
          }
          if (!excluded) {
            anim_ptr->body_include_->push_back(bodyInclude->at(i));
          }
        } else {
          anim_ptr->body_include_->push_back(bodyInclude->at(i));
        }
      }
    }
  }

  bool found = false;
  for (auto& group : ac->p_groups) {
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
    ac->p_groups.push_back(pg);
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
  for (auto& group : ac->p_groups) {
    float totStrength = 0.f;
    float finStr = 0.f;
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
      registry.view<AnimationComponent, PlayerComponent, TransformComponent,
                    PhysicsComponent, ModelComponent>();
  for (auto& entity : players) {
    auto& ac = players.get<AnimationComponent>(entity);
    auto& pl = players.get<PlayerComponent>(entity);
    auto& t = players.get<TransformComponent>(entity);
    auto& ph = players.get<PhysicsComponent>(entity);
    auto& m = players.get<ModelComponent>(entity);

    if (ac.init) {
      PlayAnimation("Resting", 0.5f, &ac, 10, 1.f, LOOP);

      PlayAnimation("LookUp", 0.5f, &ac, 21, 0.f, LOOP,
                    &ac.model_data.upperBody, &ac.model_data.arms);
      PlayAnimation("LookDown", 0.5f, &ac, 21, 0.f, LOOP,
                    &ac.model_data.upperBody, &ac.model_data.arms);
      PlayAnimation("LookLeft", 0.5f, &ac, 21, 0.f, LOOP,
                    &ac.model_data.upperBody, &ac.model_data.arms);
      PlayAnimation("LookRight", 0.5f, &ac, 21, 0.f, LOOP,
                    &ac.model_data.upperBody, &ac.model_data.arms);
      PlayAnimation("LookAhead", 1.f, &ac, 21, 0.f, LOOP,
                    &ac.model_data.upperBody, &ac.model_data.arms);
      ac.init = false;
    }

    glm::vec3 LRlookDir =
        glm::normalize(pl.look_dir * glm::vec3(1.f, 0.f, 1.f));
    glm::vec3 UDlookDir = glm::normalize(pl.look_dir);
    glm::vec3 moveDir = ph.velocity;

    // SLIDE ANIMATIONS
    constexpr float pi = glm::pi<float>();
    if (!pl.sprinting) {
      glm::quat offset = -t.rotation;

      float yaw = atan2(moveDir.x, moveDir.z);
      offset += glm::quat(glm::vec3(0.f, yaw - pi / 2.f, 0.f));

      m.rot_offset = offset;

      float strength = 0.f;
      float totStrength = 0.f;
      for (int i = 0; i < 4; i++) {
        int anim = GetActiveAnimationByName(look_anims_[i], &ac);
        switch (i) {
          case 0: {  // U
            strength = glm::clamp(glm::dot(glm::vec3(0.f, 1.f, 0.f), UDlookDir),
                                  0.f, 1.f);
            break;
          }
          case 1: {  // D
            strength = glm::clamp(
                glm::dot(glm::vec3(0.f, -1.f, 0.f), UDlookDir), 0.f, 1.f);
            break;
          }
          case 2: {  // R
            strength =
                abs(std::clamp(glm::dot(glm::vec3(1.f, 0.f, 0.f) * (t.rotation),
                                        glm::vec3(0.f, 0.f, 1.f) *
                                            (t.rotation + m.rot_offset)),
                               -1.f, 0.f));
            break;
          }
          case 3: {  // L
            strength =
                std::clamp(glm::dot(glm::vec3(1.f, 0.f, 0.f) * (t.rotation),
                                    glm::vec3(0.f, 0.f, 1.f) *
                                        (t.rotation + m.rot_offset)),
                           0.f, 1.f);
            break;
          }
        }
        totStrength += strength;
        ac.active_animations.at(anim)->strength_ = strength;
      }
      float LAStrength = 1.f - glm::clamp(totStrength, 0.f, 1.f);
      int LAAnim = GetActiveAnimationByName("LookAhead", &ac);
      ac.active_animations.at(LAAnim)->strength_ = LAStrength;

    } else {
      m.rot_offset = glm::quat();

      float strength = 0.f;
      float totStrength = 0.f;
      float cutoffSpeed = 5.f;
      float speed = glm::length(ph.velocity);
      glm::vec3 h_lookDir;
      for (int i = 0; i < 4; i++) {
        int anim = GetActiveAnimationByName(slide_anims_[i], &ac);
        // int look_anim = GetActiveAnimationByName(look_anims_[i], &ac);
        // ac.active_animations.at(look_anim)->strength_ = 0.f;
        switch (i) {
          case 0: {  // F
            h_lookDir = glm::normalize(LRlookDir);
            strength = std::clamp(glm::dot(h_lookDir, moveDir), 0.f, 1.f);
            break;
          }
          case 1: {  // B
            h_lookDir = glm::normalize(LRlookDir);
            strength = abs(std::clamp(glm::dot(h_lookDir, moveDir), -1.f, 0.f));
            break;
          }
          case 2: {  // R
            h_lookDir = glm::normalize(glm::cross(LRlookDir, up));
            strength = std::clamp(glm::dot(h_lookDir, moveDir), 0.f, 1.f);
            break;
          }
          case 3: {  // L
            h_lookDir = glm::normalize(glm::cross(LRlookDir, up));
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

    // JUMPING ANIMATIONS
    if (pl.jumping) {
      float startStrength = 0.f;
      float endStrength = 0.f;

      /*std::cout << t.position.x << " : " << t.position.y << " : "
                << t.position.z << "\n";*/

      float velCoeff =
          std::clamp(glm::dot(ph.velocity / pl.jump_force, up), 0.f, 1.f);

      startStrength = velCoeff;

      int js = GetActiveAnimationByName("JumpStart", &ac);
      ac.active_animations.at(js)->strength_ = startStrength;
      // std::cout << startStrength << "\n";

      endStrength = 1.f - velCoeff;

      int es = GetActiveAnimationByName("JumpEnd", &ac);
      ac.active_animations.at(es)->strength_ = endStrength;
    }

    // lookDirs
    if (!pl.sprinting) {
      glm::vec3 m_front = glm::normalize(glm::cross(LRlookDir, up));
    }
  }
}
void AnimationSystem::ReceiveGameEvent(GameEvent event) {
  auto registry = engine_->GetCurrentRegistry();
  switch (event.type) {
    case GameEvent::KICK: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.sprint_start.player_id) {
          auto& ac = view.get<AnimationComponent>(entity);
          auto& pc = view.get<PlayerComponent>(entity);
          PlayAnimation("Kick", 4.f, &ac, 14, 1.f, PARTIAL_MUTE,
                        &ac.model_data.upperBody);
          break;
        }
      }
      break;
    };
    case GameEvent::JUMP: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.sprint_start.player_id) {
          auto& ac = view.get<AnimationComponent>(entity);
          auto& pc = view.get<PlayerComponent>(entity);
          pc.jumping = true;
          PlayAnimation("JumpStart", 0.5f, &ac, 25, 1.f, LOOP);
          PlayAnimation("JumpEnd", 0.5f, &ac, 25, 0.f, LOOP);
          pc.jump_force =
              GlobalSettings::Access()->ValueOf("PLAYER_SPEED_JUMP");
          break;
        }
      }
      break;
    };
    case GameEvent::LAND: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.sprint_start.player_id) {
          auto& ac = view.get<AnimationComponent>(entity);
          auto& pc = view.get<PlayerComponent>(entity);
          pc.jumping = false;
          StopAnimation("JumpStart", &ac);
          StopAnimation("JumpEnd", &ac);

          break;
        }
      }
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

          StopAnimation("LookUp", &ac);
          StopAnimation("LookDown", &ac);
          StopAnimation("LookRight", &ac);
          StopAnimation("LookLeft", &ac);
          StopAnimation("LookAhead", &ac);

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

          PlayAnimation("LookUp", 0.5f, &ac, 21, 0.f, LOOP,
                        &ac.model_data.upperBody, &ac.model_data.arms);
          PlayAnimation("LookDown", 0.5f, &ac, 21, 0.f, LOOP,
                        &ac.model_data.upperBody, &ac.model_data.arms);
          PlayAnimation("LookLeft", 0.5f, &ac, 21, 0.f, LOOP,
                        &ac.model_data.upperBody, &ac.model_data.arms);
          PlayAnimation("LookRight", 0.5f, &ac, 21, 0.f, LOOP,
                        &ac.model_data.upperBody, &ac.model_data.arms);
          PlayAnimation("LookAhead", 1.f, &ac, 21, 0.f, LOOP,
                        &ac.model_data.upperBody, &ac.model_data.arms);

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
        } else if (anim->mode_ == MUTE_ALL || anim->mode_ == PARTIAL_MUTE) {
          anim->playing_ = false;
        }
      }
      if (!anim->playing_) {
        // reset important data
        anim->body_include_->clear();
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
        // remove from priority group
        for (auto& group : a.p_groups) {
          for (int a_i = 0; a_i < group.animations.size(); a_i++) {
            if (anim == group.animations.at(a_i)) {
              group.animations.erase(group.animations.begin() + a_i);
              break;
            }
          }
        }
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

            while (anim->current_frame_time_ >=
                   channel->scaling_keys.at(scalingPos).mTime) {
              channel->current_scaling_pos++;
              scalingPos = channel->current_scaling_pos;
            }

            int rotationPos = channel->current_rotation_pos;
            aiQuatKey rotationKey = channel->rotation_keys.at(rotationPos);
            glm::mat4 rotation =
                ConvertToGLM3x3(rotationKey.mValue.GetMatrix());

            while (anim->current_frame_time_ >=
                   channel->rotation_keys.at(rotationPos).mTime) {
              channel->current_rotation_pos++;
              rotationPos = channel->current_rotation_pos;
            }

            int positionPos = channel->current_position_pos;
            aiVectorKey positionKey = channel->position_keys.at(positionPos);
            glm::mat4 position = glm::translate(
                glm::vec3(positionKey.mValue.x, positionKey.mValue.y,
                          positionKey.mValue.z));

            while (anim->current_frame_time_ >=
                   channel->position_keys.at(positionPos).mTime) {
              channel->current_position_pos++;
              positionPos = channel->current_position_pos;
            }

            glm::mat4 combPRS = position * rotation * scaling;

            glm::mat4 finalMat = combPRS * anim->strength_;

            if (anim->mode_ == LOOP) {
              if (anim->priority_ >
                  bonePriorities.at(jointId)) {  // priority override
                if (anim->bonesSpecified()) {
                  if (IsIncluded(jointId, anim->body_include_,
                                 anim->body_exclude_)) {  // body argument
                                                          // success, override
                    bonePriorities.at(jointId) = anim->priority_;
                    a.model_data.bones.at(jointId).transform = finalMat;
                  }
                } else {  // No body argument, override
                  bonePriorities.at(jointId) = anim->priority_;
                  a.model_data.bones.at(jointId).transform = finalMat;
                }
              } else if (anim->priority_ ==
                         bonePriorities.at(jointId)) {  // blend
                if (anim->bonesSpecified()) {
                  if (IsIncluded(jointId, anim->body_include_,
                                 anim->body_exclude_)) {  // body argument
                                                          // success, blend
                    a.model_data.bones.at(jointId).transform += finalMat;
                  }
                } else {  // No body argument, blend
                  a.model_data.bones.at(jointId).transform += finalMat;
                }
              }
            } else if (anim->mode_ == MUTE_ALL) {  // override
              bonePriorities.at(jointId) = 255;
              a.model_data.bones.at(jointId).transform = finalMat;
            } else if (anim->mode_ == PARTIAL_MUTE) {
              if (anim->bonesSpecified()) {
                if (IsIncluded(
                        jointId, anim->body_include_,
                        anim->body_exclude_)) {  // override specified bodyparts
                  bonePriorities.at(jointId) = 254;
                  a.model_data.bones.at(jointId).transform = finalMat;
                } else {
                  if (anim->priority_ >= bonePriorities.at(jointId)) {
                    bonePriorities.at(jointId) = anim->priority_;
                    a.model_data.bones.at(jointId).transform = finalMat;
                  } else if (anim->priority_ == bonePriorities.at(jointId)) {
                    a.model_data.bones.at(jointId).transform += finalMat;
                  }
                }
              }
            }
          }
        }
      }

      anim->current_frame_time_ += anim->speed_ * anim->tick_per_second_ * dt;
    }

    GetDefaultPose(glm::mat4(1.f), &a.model_data.bones.at(rootBone),
                   &a.model_data.bones, a.model_data.globalInverseTransform);

    for (auto bone : a.model_data.bones) {
      boneTransforms.at(bone.id) = bone.f_transform;
    }
    a.bone_transforms = boneTransforms;
  }
}

void AnimationSystem::Reset(entt::registry& registry) {
  auto animation_entities = registry.view<AnimationComponent>();
  for (auto& entity : animation_entities) {
    auto& a = animation_entities.get(entity);
    a.p_groups.clear();
  }
}