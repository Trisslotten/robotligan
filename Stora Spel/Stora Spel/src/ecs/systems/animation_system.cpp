#include "animation_system.hpp"

#include "engine.hpp"
#include "glob/AssimpToGLMConverter.hpp"

void AnimationSystem::Init(Engine* engine) { engine_ = engine; }

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

bool AnimationSystem::IsExcluded(int bone, std::vector<int>* excluded) {
  for (int i = 0; i < excluded->size(); i++) {
    if (bone == excluded->at(i)) {
      return true;
    }
  }
  return false;
}

int AnimationSystem::GetAnimationByName(std::string name,
                                        AnimationComponent* ac) {
  std::vector<glob::Animation*>* anims = &ac->model_data.animations;
  for (int i = 0; i < anims->size(); i++) {
    if (anims->at(i)->name_ == name) {
      return i;
    }
  }
  return -1;
}

int AnimationSystem::GetActiveAnimationByName(std::string name,
                                              AnimationComponent* ac) {
  for (int i = 0; i < ac->active_animations.size(); i++) {
    if (ac->active_animations.at(i)->animation_->name_ == name) {
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
    std::cout
        << "WARNING: Attempting to play animation: Could not find animation \""
        << name << "\"!\n";
    return;
  }

  for (int i = 0; i < ac->active_animations.size(); i++) {
    if (ac->active_animations.at(i)->animation_ ==
        ac->model_data.animations.at(anim)) {
      std::cout << "WARNING: Attempting to play animation: \""
                << ac->model_data.animations.at(anim)->name_
                << "\" is already playing, cannot stack the same animation!\n";
      return;
    }
  }
  glob::PlayableAnimation* p_anim = new glob::PlayableAnimation;
  p_anim->animation_ = ac->model_data.animations.at(anim);

  p_anim->bone_position_ =
      new std::vector<glm::vec3>(ac->model_data.bones.size());
  p_anim->bone_rotation_ =
      new std::vector<glm::quat>(ac->model_data.bones.size());
  p_anim->bone_scale_ = new std::vector<glm::vec3>(ac->model_data.bones.size());

  p_anim->speed_ = speed;
  p_anim->priority_ = priority;
  p_anim->mode_ = mode;
  p_anim->playing_ = true;
  p_anim->time_ = 0;
  p_anim->strength_ = strength;

  if (ac->model_data.humanoid) {
    if (bodyInclude != nullptr) {
      for (int i = 0; i < bodyInclude->size(); i++) {
        bool found = false;
        for (int j = 0; j < p_anim->body_include_->size(); j++) {
          if (bodyInclude->at(i) == p_anim->body_include_->at(j)) {
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
              p_anim->body_include_->push_back(bodyInclude->at(i));
            }
          } else {
            p_anim->body_include_->push_back(bodyInclude->at(i));
          }
        }
      }
    }
  }

  bool found = false;
  for (auto& group : ac->p_groups) {
    if (group.priority == priority) {
      group.animations.push_back(p_anim);
      found = true;
      break;
    }
  }
  if (!found) {
    priorityGroup pg;
    pg.priority = priority;
    pg.animations.push_back(p_anim);
    ac->p_groups.push_back(pg);
  }

  ac->active_animations.push_back(p_anim);
  // std::cout << "Playing " << p_anim.animation_->name_ << "\n";
}

void AnimationSystem::StopAnimation(std::string name, AnimationComponent* ac) {
  int anim = GetActiveAnimationByName(name, ac);
  if (anim == -1) {
    std::cout
        << "WARNING: Attempting to stop animation: Could not find animation \""
        << name << "\"!\n";
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

    if (pl.localPlayer) {
      if (ac.init) {
        PlayAnimation("Resting", 1.f, &ac, 10, 1.f, LOOP);
        ac.init = false;
      }

      if (pl.can_smash && !pl.started_smash) {
        PlayAnimation("ReadyKick", 1.f, &ac, 15, 1.f, LOOP);
        pl.started_smash = true;
      }
      if (!pl.can_smash && pl.started_smash) {
        StopAnimation("ReadyKick", &ac);
        pl.started_smash = false;
	  }
    } else {
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

        if (pl.running) {
          PlayAnimation("Run", 1.f, &ac, 15, 1.f, LOOP);
        }
        if (pl.jumping) {
          PlayAnimation("JumpStart", 0.5f, &ac, 25, 1.f, LOOP);
          PlayAnimation("JumpEnd", 0.5f, &ac, 25, 0.f, LOOP);
        }
        if (pl.sprinting) {
          PlayAnimation("SlideF", 1.f, &ac, 20, 0.f, LOOP);
          PlayAnimation("SlideB", 1.f, &ac, 20, 0.f, LOOP);
          PlayAnimation("SlideR", 1.f, &ac, 20, 0.f, LOOP);
          PlayAnimation("SlideL", 1.f, &ac, 20, 0.f, LOOP);
        }

        ac.init = false;
      }

      glm::vec3 LRlookDir =
          glm::normalize(pl.look_dir * glm::vec3(1.f, 0.f, 1.f));
      glm::vec3 UDlookDir = glm::normalize(pl.look_dir);
      glm::vec3 moveDir;
      if (abs(ph.velocity.x) > 0.01 || abs(ph.velocity.y > 0.01) ||
          abs(ph.velocity.z > 0.01)) {
        moveDir = ph.velocity;
        pl.vel_dir = moveDir;
      } else {
        moveDir = pl.vel_dir;
      }
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
              strength = glm::clamp(
                  glm::dot(glm::vec3(0.f, 1.f, 0.f), UDlookDir), 0.f, 1.f);
              break;
            }
            case 1: {  // D
              strength = glm::clamp(
                  glm::dot(glm::vec3(0.f, -1.f, 0.f), UDlookDir), 0.f, 1.f);
              break;
            }
            case 2: {  // R
              strength = abs(
                  std::clamp(glm::dot(glm::vec3(1.f, 0.f, 0.f) * (t.rotation),
                                      glm::vec3(0.f, 0.f, 1.f) *
                                          (t.rotation + m.rot_offset) / 1.5f),
                             -1.f, 0.f));
              break;
            }
            case 3: {  // L
              strength =
                  std::clamp(glm::dot(glm::vec3(1.f, 0.f, 0.f) * (t.rotation),
                                      glm::vec3(0.f, 0.f, 1.f) *
                                          (t.rotation + m.rot_offset) / 1.5f),
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
          switch (i) {
            case 0: {  // F
              h_lookDir = glm::normalize(LRlookDir);
              strength = std::clamp(glm::dot(h_lookDir, moveDir), 0.f, 1.f);
              break;
            }
            case 1: {  // B
              h_lookDir = glm::normalize(LRlookDir);
              strength =
                  abs(std::clamp(glm::dot(h_lookDir, moveDir), -1.f, 0.f));
              break;
            }
            case 2: {  // R
              h_lookDir = glm::normalize(glm::cross(LRlookDir, up));
              strength = std::clamp(glm::dot(h_lookDir, moveDir), 0.f, 1.f);
              break;
            }
            case 3: {  // L
              h_lookDir = glm::normalize(glm::cross(LRlookDir, up));
              strength =
                  abs(std::clamp(glm::dot(h_lookDir, moveDir), -1.f, 0.f));
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
          if (anim != -1)
            ac.active_animations.at(anim)->strength_ = strength;
          totStrength += strength;
        }
        if (speed < cutoffSpeed) {
          int f = GetActiveAnimationByName("SlideF", &ac);
          int b = GetActiveAnimationByName("SlideB", &ac);
          float defaultPoseModifier =
              std::clamp(1.f - totStrength, 0.f, 1.f) / 2.f;
          if (f != -1)
            ac.active_animations.at(f)->strength_ = defaultPoseModifier;
          if (b != -1)
            ac.active_animations.at(b)->strength_ = defaultPoseModifier;
        }
      }

      // RUNNING ANIMATIONS

      // JUMPING ANIMATIONS
      if (pl.jumping) {
        float startStrength = 0.f;
        float endStrength = 0.f;

        float velCoeff =
            std::clamp(glm::dot(ph.velocity / pl.jump_force, up), 0.f, 1.f);

        startStrength = velCoeff;

        try {
          int js = GetActiveAnimationByName("JumpStart", &ac);
          // std::cout << js << " : js\n";
          if (js < 0 || js >= ac.active_animations.size()) {
            std::cout << "Error: could not find animation JumpStart"
                      << std::endl;
          } else {
            ac.active_animations.at(js)->strength_ = startStrength;
          }
          // std::cout << startStrength << "\n";

          endStrength = 1.f - velCoeff;

          int es = GetActiveAnimationByName("JumpEnd", &ac);
          // std::cout << es << " : es\n";
          if (es < 0 || es >= ac.active_animations.size()) {
            std::cout << "Error: could not find animation JumpEnd" << std::endl;
          } else {
            ac.active_animations.at(es)->strength_ = endStrength;
          }

        } catch (std::exception& e) {
          // ???
          // std::cout << e.what() << '\n';
        }
      }

      // lookDirs
      if (!pl.sprinting) {
        glm::vec3 m_front = glm::normalize(glm::cross(LRlookDir, up));
      }
    }
  }
}
void AnimationSystem::ReceiveGameEvent(GameEvent event) {
  auto registry = engine_->GetCurrentRegistry();
  switch (event.type) {
    case GameEvent::SHOOT: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.shoot.player_id) {
          auto& ac = view.get<AnimationComponent>(entity);
          auto& pc = view.get<PlayerComponent>(entity);
          if (pc.localPlayer) {
            PlayAnimation("Shoot", 1.f, &ac, 14, 1.f, MUTE_ALL);
          } else {
            PlayAnimation("Shoot", 4.f, &ac, 14, 1.f, PARTIAL_MUTE,
                          &ac.model_data.upperBody);
          }
          break;
        }
      }
      break;
    };
    case GameEvent::KICK: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.kick.player_id) {
          auto& ac = view.get<AnimationComponent>(entity);
          auto& pc = view.get<PlayerComponent>(entity);
          if (pc.localPlayer) {
            PlayAnimation("Kick", 1.f, &ac, 14, 1.f, MUTE_ALL);
          } else {
            PlayAnimation("Kick", 4.f, &ac, 14, 1.f, PARTIAL_MUTE,
                          &ac.model_data.upperBody);
          }
          break;
        }
      }
      break;
    };
    case GameEvent::JUMP: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.jump.player_id) {
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
        if (view.get<IDComponent>(entity).id == event.land.player_id) {
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
        if (view.get<IDComponent>(entity).id == event.run_start.player_id) {
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
        if (view.get<IDComponent>(entity).id == event.run_end.player_id) {
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
          if (pc.localPlayer) {
            PlayAnimation("Slide", 1.f, &ac, 20, 1.f, LOOP);
          } else {
            PlayAnimation("SlideF", 1.f, &ac, 20, 0.f, LOOP);
            PlayAnimation("SlideB", 1.f, &ac, 20, 0.f, LOOP);
            PlayAnimation("SlideR", 1.f, &ac, 20, 0.f, LOOP);
            PlayAnimation("SlideL", 1.f, &ac, 20, 0.f, LOOP);

            StopAnimation("LookUp", &ac);
            StopAnimation("LookDown", &ac);
            StopAnimation("LookRight", &ac);
            StopAnimation("LookLeft", &ac);
            StopAnimation("LookAhead", &ac);
          }

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
          if (pc.localPlayer) {
            StopAnimation("Slide", &ac);
          } else {
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
          }

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

    std::vector<glm::mat4> boneTransforms(a.model_data.bones.size());
    std::vector<int> bonePriorities(a.model_data.bones.size());

    for (int i = 0; i < a.active_animations.size(); i++) {
      glob::PlayableAnimation* anim = a.active_animations.at(i);
      bool removedAnimation = false;
      if (anim->time_ >= anim->animation_->duration_) {
        // Loop for the time being
        if (anim->mode_ == LOOP) {
          anim->time_ = 0;
        } else if (anim->mode_ == MUTE_ALL || anim->mode_ == PARTIAL_MUTE) {
          anim->playing_ = false;
        }
      }
      if (!anim->playing_) {
        // remove from list
        a.active_animations.at(i)->clear();
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
        for (int j = 0; j < anim->animation_->channels_.size();
             j++) {  // all channels (bones)
          glob::Channel* channel = &anim->animation_->channels_.at(j);
          int jointId = (int)channel->boneID;

          if (jointId != rootBone) {
            glm::vec3 pos =
                InterpolateVector(anim->time_, &channel->position_keys);
            glm::mat4 position = glm::translate(pos);

            glm::mat4 rotation =
                InterpolateQuat(anim->time_, &channel->rotation_keys);

            glm::vec3 scale =
                InterpolateVector(anim->time_, &channel->scaling_keys);
            glm::mat4 scaling = glm::scale(scale);

            anim->bone_position_->at(jointId) = pos;
            anim->bone_rotation_->at(jointId) = rotation;
            anim->bone_scale_->at(jointId) = scale;

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

      anim->time_ += anim->speed_ * anim->animation_->tick_per_second_ * dt;
    }

    GetDefaultPose(glm::mat4(1.f), &a.model_data.bones.at(rootBone),
                   &a.model_data.bones, a.model_data.globalInverseTransform);

    for (auto bone : a.model_data.bones) {
      boneTransforms.at(bone.id) = bone.f_transform;
    }
    a.bone_transforms = boneTransforms;
  }
}

glm::vec3 AnimationSystem::InterpolateVector(float time,
                                             std::vector<aiVectorKey>* keys) {
  if (keys->size() == 1) {
    aiVector3D ret = keys->at(0).mValue;
    return glm::vec3(ret.x, ret.y, ret.x);
  }

  int pos = 0;
  for (int i = 0; i < keys->size() - 1; i++) {
    if (time < (float)keys->at(i + 1).mTime) {
      pos = i;
      break;
    }
  }

  int nextPos = pos + 1;

  float dt = (float)(keys->at(nextPos).mTime - keys->at(pos).mTime);

  float f = (time - (float)keys->at(pos).mTime) / dt;

  glm::vec3 start = glob::AssToGLM::ConvertToGLMVec3(keys->at(pos).mValue);
  glm::vec3 end = glob::AssToGLM::ConvertToGLMVec3(keys->at(nextPos).mValue);

  glm::vec3 dtv3 = end - start;

  return start + f * dtv3;
}

glm::mat4 AnimationSystem::InterpolateQuat(float time,
                                           std::vector<aiQuatKey>* keys) {
  aiQuaternion ret;
  if (keys->size() == 1) {
    ret = keys->at(0).mValue;
    return glm::mat4(glob::AssToGLM::ConvertToGLM3x3(ret.GetMatrix()));
  }

  int pos = 0;
  for (int i = 0; i < keys->size() - 1; i++) {
    if (time < (float)keys->at(i + 1).mTime) {
      pos = i;
      break;
    }
  }

  int nextPos = pos + 1;

  float dt = (float)(keys->at(nextPos).mTime - keys->at(pos).mTime);

  float f = (time - (float)keys->at(pos).mTime) / dt;

  aiQuaternion start = keys->at(pos).mValue;
  aiQuaternion end = keys->at(nextPos).mValue;
  aiQuaternion::Interpolate(ret, start, end, f);

  return glm::mat4(
      glob::AssToGLM::ConvertToGLM3x3(ret.Normalize().GetMatrix()));
}

void AnimationSystem::Reset(entt::registry& registry) {
  auto animation_entities = registry.view<AnimationComponent>();
  for (auto& entity : animation_entities) {
    auto& a = animation_entities.get(entity);
    a.p_groups.clear();
  }
}