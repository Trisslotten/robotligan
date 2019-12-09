#include "animation_system.hpp"

#include "engine.hpp"
#include "glob/AssimpToGLMConverter.hpp"
#include "util/player_animation_controller.hpp"

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
      if (ac->active_animations.at(i)->stopping_) {
        ac->active_animations.at(i)->stopping_ = false;
        return;
      }
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

  p_anim->stopping_ = false;
  p_anim->fade_ = 0.f;

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
  ac->active_animations.at(anim)->stopping_ = true;
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

        if (pl.running) {
          PAC::playRunAnims(this, ac, pl.localPlayer);
        } else if (pl.jumping) {
          PAC::playJumpAnims(this, ac, pl.localPlayer);
        } else if (pl.sprinting) {
          PAC::playSlideAnims(this, ac, pl.localPlayer);
        } else {
          PAC::playLookAnims(this, ac, pl.localPlayer);
        }

        ac.init = false;
      }

      glm::vec3 LRlookDir =
          glm::normalize(pl.look_dir * glm::vec3(1.f, 0.f, 1.f));
      glm::vec3 UDlookDir = glm::normalize(pl.look_dir);
      glm::vec3 moveDir;
      if (abs(ph.velocity.x) > 0.01 || abs(ph.velocity.z > 0.01)) {
        moveDir = ph.velocity;
        pl.vel_dir = moveDir;
      } else {
        moveDir = pl.vel_dir;
      }

      float lookMoveOffset = glm::dot(
          LRlookDir, glm::normalize(moveDir * glm::vec3(1.f, 0.1, 1.f)));

      bool backwards = (lookMoveOffset < -0.2);

	  float rotator = dt * 5.f;
      float yaw = 0.f;

	  //Kick animations
	  if (pl.kicking || pl.shooting) {
        if (GetActiveAnimationByName("SlideF", &ac) < 0) {
          PAC::playSlideAnims(this, ac, pl.localPlayer);
        }
        rotator = dt * 10.f;
        int animNumKick = GetActiveAnimationByName("Kick", &ac);
        int animNumShoot = GetActiveAnimationByName("Shoot", &ac);
        if (animNumKick >= 0) {
          glob::PlayableAnimation* animKick =
              ac.active_animations.at(animNumKick);
          if (animKick->stopping_) {
            pl.kicking = false;
            if (!pl.sprinting && !pl.shooting) {
              PAC::stopSlideAnims(this, ac, pl.localPlayer);
            }
          }
        }
        if (animNumShoot >= 0) {
          glob::PlayableAnimation* animShoot =
              ac.active_animations.at(animNumShoot);
          if (animShoot->stopping_) {
            pl.shooting = false;
            if (!pl.sprinting && !pl.kicking) {
              PAC::stopSlideAnims(this, ac, pl.localPlayer);
            }
          }
        }
      }

      // LOOK ANIMATIONS
      constexpr float pi = glm::pi<float>();
      if (!pl.sprinting && !pl.kicking && !pl.shooting) {
        glm::quat offset = -t.rotation;

        if (backwards && pl.running && !pl.jumping) {
          yaw = atan2(-moveDir.x, -moveDir.z);
        } else {
          yaw = atan2(moveDir.x, moveDir.z);
        }

        float lYaw = yaw + pi;
        float SA =
            fmod(fmod((lYaw - ac.yawInterpolator + pi), pi * 2.f) + (pi * 3.f),
                 pi * 2.f) -
            pi;

        ac.yawInterpolator += fmod(SA * rotator, pi);

        offset += glm::quat(glm::vec3(0.f, ac.yawInterpolator - pi / 2.f, 0.f));
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
      }

      // RUN ANIMATIONS
      if (pl.running && GetActiveAnimationByName("Run_B", &ac) != -1 &&
          GetActiveAnimationByName("Run", &ac) != -1) {
        int RAnim;
        if (backwards) {
          RAnim = GetActiveAnimationByName("Run_B", &ac);

        } else {
          RAnim = GetActiveAnimationByName("Run", &ac);
        }

        ac.active_animations.at(RAnim)->strength_ = 1.f;
      }

      // SLIDE ANIMATIONS
      if (pl.sprinting || pl.kicking || pl.shooting) {
        glm::quat offset = -t.rotation;

		float yaw = atan2(pl.look_dir.x, pl.look_dir.z);

		        float lYaw = yaw + pi;
                float SA =
                    fmod(fmod((lYaw - ac.yawInterpolator + pi), pi * 2.f) +
                             (pi * 3.f),
                         pi * 2.f) -
                    pi;

                ac.yawInterpolator += fmod(SA * rotator, pi);

                offset += glm::quat(
                    glm::vec3(0.f, ac.yawInterpolator - pi / 2.f, 0.f));
                m.rot_offset = offset;

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
          ac.active_animations.at(anim)->strength_ = strength;
          totStrength += strength;
        }
        if (speed < cutoffSpeed) {
          int f = GetActiveAnimationByName("SlideF", &ac);
          int b = GetActiveAnimationByName("SlideB", &ac);
          float defaultPoseModifier =
              std::clamp(1.f - totStrength, 0.f, 1.f) / 2.f;
          if (f >= 0 && b >= 0) {
            ac.active_animations.at(f)->strength_ = defaultPoseModifier;
            ac.active_animations.at(b)->strength_ = defaultPoseModifier;
          }
        }
      }

      // JUMPING ANIMATIONS
      if (pl.jumping) {
        float startStrength = 1.f;
        float endStrength = 0.f;
        float velCoeff = 1.f;
        float vertVel = std::clamp(
            glm::vec3((ph.velocity / pl.jump_force) * up).y, -1.f, 1.f);

        if (vertVel != 0) {
          startStrength = std::clamp(vertVel, 0.f, 1.f);

          try {
            int js = GetActiveAnimationByName("JumpStart", &ac);
            if (js < 0 || js >= ac.active_animations.size()) {
              std::cout << "Error: could not find animation JumpStart"
                        << std::endl;
            } else {
              ac.active_animations.at(js)->strength_ = startStrength;
            }

            endStrength = std::clamp((1.f - vertVel) / 2.f, 0.f, 1.f);

            int es = GetActiveAnimationByName("JumpEnd", &ac);
            if (es < 0 || es >= ac.active_animations.size()) {
              std::cout << "Error: could not find animation JumpEnd"
                        << std::endl;
            } else {
              ac.active_animations.at(es)->strength_ = endStrength;
            }

          } catch (std::exception& e) {
            // ???
            // std::cout << e.what() << '\n';
          }
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
          if (!pc.shooting) {
            PAC::playShootAnims(this, ac, pc.localPlayer);
            pc.shooting = true;
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
          if (!pc.kicking) {
            PAC::playKickAnims(this, ac, pc.localPlayer);
            pc.kicking = true;
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
          PAC::playJumpAnims(this, ac, pc.localPlayer);
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
          PAC::stopJumpAnims(this, ac, pc.localPlayer);

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
          PAC::playRunAnims(this, ac, pc.localPlayer);

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
          PAC::stopRunAnims(this, ac, pc.localPlayer);

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
          PAC::playSlideAnims(this, ac, pc.localPlayer);
          PAC::stopLookAnims(this, ac, pc.localPlayer);
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
          PAC::stopSlideAnims(this, ac, pc.localPlayer);
          PAC::playLookAnims(this, ac, pc.localPlayer);
          pc.sprint_coeff = 0.f;

          break;
        }
      }
      break;
    }

    case GameEvent::PLAYER_IDLE: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id == event.player_idle.player_id) {
          auto& pc = view.get<PlayerComponent>(entity);
          auto& ac = view.get<AnimationComponent>(entity);

          PAC::playLookAnims(this, ac, pc.localPlayer);

          break;
        }
      }
      break;
    }

    case GameEvent::PLAYER_IDLE_END: {
      auto view =
          registry->view<IDComponent, AnimationComponent, PlayerComponent>();
      for (auto entity : view) {
        if (view.get<IDComponent>(entity).id ==
            event.player_idle_end.player_id) {
          auto& pc = view.get<PlayerComponent>(entity);
          auto& ac = view.get<AnimationComponent>(entity);
          PAC::stopLookAnims(this, ac, pc.localPlayer);
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
          anim->stopping_ = true;
          anim->time_ = anim->animation_->duration_ - 0.1f;
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

            glm::quat rotation =
                InterpolateQuat(anim->time_, &channel->rotation_keys);

            glm::vec3 scale =
                InterpolateVector(anim->time_, &channel->scaling_keys);
            glm::mat4 scaling = glm::scale(scale);

            // Unused for the moment, potential fix for additive animation
            // scaling problems
            anim->bone_position_->at(jointId) = pos;
            anim->bone_rotation_->at(jointId) = glm::normalize(rotation);
            anim->bone_scale_->at(jointId) = scale;

          }
        }
      }
      if (!anim->stopping_) {
        anim->time_ += anim->speed_ * anim->animation_->tick_per_second_ * dt;
      }

	  if (anim->stopping_ && anim->fade_ <= 0) {
        anim->playing_ = false;
      } else if (anim->stopping_) {
        anim->fade_ -= dt * 5.f;
      } else if (!anim->stopping_) {
        anim->fade_ += dt * 5.f;
	  }

      anim->fade_ = std::clamp(anim->fade_, 0.f, 1.f);
    }

    std::vector<glm::vec3> f_pos(a.model_data.bones.size(), glm::vec3(0.f));
    std::vector<glm::quat> f_rot(a.model_data.bones.size(), glm::quat());
    std::vector<glm::vec3> f_scale(a.model_data.bones.size(), glm::vec3(0.f));

    for (int i = 0; i < a.active_animations.size(); i++) {
      glob::PlayableAnimation* anim = a.active_animations.at(i);
      for (int j = 0; j < anim->bone_position_->size();
           j++) {  // all channels (bones)
        glob::Channel* channel = &anim->animation_->channels_.at(j);
        int jointId = (int)channel->boneID;

		bool blend = false;
        bool set = false;

        switch (anim->mode_) {
          case (LOOP): {
            if (anim->priority_ >
                bonePriorities.at(jointId)) {  // priority override
              if (anim->bonesSpecified()) {
                if (IsIncluded(jointId, anim->body_include_,
                               anim->body_exclude_)) {  // body argument
                                                        // success, override
                  bonePriorities.at(jointId) = anim->priority_;
                  set = true;
                }
              } else {  // No body argument, override
                bonePriorities.at(jointId) = anim->priority_;
                set = true;
              }
            } else if (anim->priority_ ==
                       bonePriorities.at(jointId)) {  // blend
              if (anim->bonesSpecified()) {
                if (IsIncluded(jointId, anim->body_include_,
                               anim->body_exclude_)) {  // body argument
                                                        // success, blend
                  blend = true;
                }
              } else {  // No body argument, blend

				  blend = true;
              }
            }
            break;
          }

          case (MUTE_ALL): {  // override
            bonePriorities.at(jointId) = 255;
            set = true;
            break;
          }

          case (PARTIAL_MUTE): {
            if (anim->bonesSpecified()) {
              if (IsIncluded(jointId, anim->body_include_,
                             anim->body_exclude_)) {  // override specified
                                                      // bodyparts
                bonePriorities.at(jointId) = 254;
                set = true;
              } else {
                if (anim->priority_ >=
                    bonePriorities.at(jointId)) {  // priority override
                  bonePriorities.at(jointId) = anim->priority_;
                  set = true;
                } else if (anim->priority_ ==
                           bonePriorities.at(jointId)) {  // blend
                  blend = true;
                }
              }
            }
            break;
          }
        }

		if (blend) {
          interpolatePRS(f_pos.at(jointId), f_rot.at(jointId),
                         f_scale.at(jointId), anim->bone_position_->at(jointId),
                         anim->bone_rotation_->at(jointId),
                                 anim->bone_scale_->at(jointId),
                                 anim->strength_ * anim->fade_);
		} else if(set) {
                  if (anim->stopping_) {
                    interpolatePRS(
                        f_pos.at(jointId), f_rot.at(jointId),
                        f_scale.at(jointId), anim->bone_position_->at(jointId),
                        anim->bone_rotation_->at(jointId),
                        anim->bone_scale_->at(jointId), anim->strength_ * anim->fade_);
                  } else {
                    setPRS(f_pos.at(jointId), f_rot.at(jointId),
                           f_scale.at(jointId),
                           anim->bone_position_->at(jointId),
                           anim->bone_rotation_->at(jointId),
                           anim->bone_scale_->at(jointId));
                  }
		}

      }
    }

    // add all AC positions/rotations/scales togeather here
	for (int i = 0; i < a.model_data.bones.size(); i++) {
      if (i != rootBone) {

      glm::mat4 mat = glm::mat4(glm::translate(f_pos.at(i)) * glm::mat4(glm::normalize(f_rot.at(i))) * glm::scale(f_scale.at(i)));

	  a.model_data.bones.at(i).transform = mat;
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

glm::quat AnimationSystem::InterpolateQuat(float time,
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

  glm::quat q;
  q.w = ret.w;
  q.x = ret.x;
  q.y = ret.y;
  q.z = ret.z;
  return q;
}

void AnimationSystem::interpolatePRS(glm::vec3& j_pos, glm::quat& j_rot,
                                     glm::vec3& j_scale, glm::vec3 pos,
                                     glm::quat rot, glm::vec3 scale,
                                     float str) {
  j_pos = j_pos + (j_pos - pos) * str;
  j_rot = glm::slerp(j_rot, rot, str);
  j_scale = j_scale + (j_scale - scale) * str;
}

void AnimationSystem::setPRS(glm::vec3& j_pos, glm::quat& j_rot,
                             glm::vec3& j_scale, glm::vec3 pos, glm::quat rot,
                             glm::vec3 scale) {
  j_pos = pos;
  j_rot = rot;
  j_scale = scale;
}

void AnimationSystem::Reset(entt::registry& registry) {
  auto animation_entities = registry.view<AnimationComponent>();
  for (auto& entity : animation_entities) {
    auto& a = animation_entities.get(entity);
    a.p_groups.clear();
  }
}