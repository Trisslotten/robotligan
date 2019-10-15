#include "animation_system.hpp"

#include "engine.hpp"

void AnimationSystem::init(Engine* engine) {
	engine_ = engine;
}

glm::mat3 AnimationSystem::convertToGLM3x3(aiMatrix3x3 aiMat) {
	return {
		  aiMat.a1, aiMat.b1, aiMat.c1,
		  aiMat.a2, aiMat.b2, aiMat.c2,
		  aiMat.a3, aiMat.b3, aiMat.c3
	};
}

void AnimationSystem::getDefaultPose(glm::mat4 parent, glob::Joint* bone, std::vector<glob::Joint>* armature, glm::mat4 globalInverseTransform) {
	glm::mat4 parentTransform = parent;

	glm::mat4 globalTransform = parentTransform * bone->transform;

	bone->f_transform = globalInverseTransform * globalTransform * bone->offset;

	for (int i = 0; i < bone->children.size(); i++) {
		getDefaultPose(globalTransform, &armature->at(bone->children.at(i)), armature, globalInverseTransform);
	}
}

bool AnimationSystem::isAChildOf(int parent, int lookFor, AnimationComponent* ac) {
	for (int j = 0; j < ac->model_data.bones.at(parent).children.size(); j++) {
		if (ac->model_data.bones.at(parent).children.at(j) == lookFor) {
			return true;
		}
		bool ret = isAChildOf(ac->model_data.bones.at(parent).children.at(j), lookFor, ac);
		if (ret) {
			return true;
		}
	}
	return false;
}

int AnimationSystem::getAnimationByName(std::string name, AnimationComponent* ac) {
	std::vector<glob::Animation>* anims = &ac->model_data.animations;
	for (int i = 0; i < anims->size(); i++) {
		if (anims->at(i).name_ == name) {
			return i;
		}
	}
	return -1;
}

int AnimationSystem::getActiveAnimationByName(std::string name, AnimationComponent* ac) {
	std::vector<glob::Animation*>* anims = &ac->active_animations;
	for (int i = 0; i < anims->size(); i++) {
		if (anims->at(i)->name_ == name) {
			return i;
		}
	}
	return -1;
}

void AnimationSystem::playAnimation(std::string name, float speed, AnimationComponent* ac, char priority, float strength, int mode, int bodyArgument) {
	int anim = getAnimationByName(name, ac);
	if (anim == -1) {
		//std::cout << "WARNING: Could not find animation " << name << "!\n";
		return;
	}

	for (int i = 0; i < ac->active_animations.size(); i++) {
		if (ac->active_animations.at(i) == &ac->model_data.animations.at(anim)) {
			//std::cout << "WARNING: The animation \"" << ac->model_data.animations.at(anim).name_ << "\" is already playing, cannot stack the same animation!\n";
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

	//std::cout << "Playing " << anim_ptr->name_ << "\n";
}

void AnimationSystem::stopAnimation(std::string name, AnimationComponent* ac) {
	int anim = getActiveAnimationByName(name, ac);
	if (anim == -1) {
		//std::cout << "WARNING: Could not find animation " << name << "!\n";
		return;
	}
	ac->active_animations.at(anim)->playing_ = false;
}

void AnimationSystem::strengthModulator(AnimationComponent* ac) {
	for (auto& targetAnimation : ac->active_animations) {
		float totStrength = 0.f;
		for (auto& otherAnimation : ac->active_animations) {
			if (targetAnimation->priority_ == otherAnimation->priority_) {
				totStrength += otherAnimation->strength_;
			}
		}
		targetAnimation->strength_ = targetAnimation->strength_ / totStrength;
	}
}

void AnimationSystem::updateEntities(entt::registry& registry, float dt) {
	auto players = registry.view<AnimationComponent, ModelComponent, PlayerComponent, TransformComponent, CameraComponent, PhysicsComponent>();
	for (auto& entity : players) {
		auto& ac = players.get<AnimationComponent>(entity);
		auto& m = players.get<ModelComponent>(entity);
		auto& pl = players.get<PlayerComponent>(entity);
		auto& t = players.get<TransformComponent>(entity);
		auto& c = players.get<CameraComponent>(entity);
		auto& ph = players.get<PhysicsComponent>(entity);
		
		playAnimation("Resting", 0.5f, &ac, 10, 1.f, LOOP);

		glm::vec3 lookDir = c.GetLookDir() * glm::vec3(1.f, 0.f, 1.f);
		glm::vec3 h_lookDir;
		glm::vec3 moveDir = glm::normalize(ph.velocity);

		//SLIDE ANIMATIONS
		bool standStill = false;
		if (abs(ph.velocity.x + ph.velocity.y + ph.velocity.z) < 1.f) {
			standStill = true;
		}
		float strength = 0.f;
		if (!standStill) {
			for (int i = 0; i < 4; i++) {
				int anim = getActiveAnimationByName(slide_anims_[i], &ac);
				if (anim != -1) {
					switch (i) {
						case 0: {//F
							h_lookDir = glm::normalize(lookDir);
							strength = std::clamp(glm::dot(h_lookDir, moveDir), 0.f, 1.f);
							ac.active_animations.at(anim)->strength_ = strength;
							break;
						}
						case 1: {//B
							h_lookDir = glm::normalize(lookDir);
							strength = abs(std::clamp(glm::dot(h_lookDir, moveDir), -1.f, 0.f));
							ac.active_animations.at(anim)->strength_ = strength;
							break;
						}
						case 2: {//R
							h_lookDir = glm::normalize(glm::cross(lookDir, glm::vec3(0.f, 1.f, 0.f)));
							strength = std::clamp(glm::dot(h_lookDir, moveDir), 0.f, 1.f);
							ac.active_animations.at(anim)->strength_ = strength;
							break;
						}
						case 3: {//L
							h_lookDir = glm::normalize(glm::cross(lookDir, glm::vec3(0.f, 1.f, 0.f)));
							strength =  abs(std::clamp(glm::dot(h_lookDir, moveDir), -1.f, 0.f));
							ac.active_animations.at(anim)->strength_ = strength;
							break;
						}
					}
				}
			}
		}
		else {
			int f = getActiveAnimationByName("SlideF", &ac);
			int b = getActiveAnimationByName("SlideB", &ac);
			if (f != -1) {
				ac.active_animations.at(f)->strength_ = 0.5f;
			}
			if (b != -1) {
				ac.active_animations.at(b)->strength_ = 0.5f;
			}
		}

		//RUNNING ANIMATIONS
	}
}
void AnimationSystem::receiveGameEvent(GameEvent event) {

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
		auto view = registry->view<IDComponent, AnimationComponent>();
		for (auto entity : view) {
			auto& id_c = view.get<IDComponent>(entity);
			auto& ac = view.get<AnimationComponent>(entity);
			if (id_c.id == event.sprint_start.player_id) {

				playAnimation("Run", 1.f, &ac, 15, 1.f, LOOP);

				break;
			}
		}
		break;
	};
	case GameEvent::RUN_END: {
		auto view = registry->view<IDComponent, AnimationComponent>();
		for (auto entity : view) {
			auto& id_c = view.get<IDComponent>(entity);
			auto& ac = view.get<AnimationComponent>(entity);
			if (id_c.id == event.sprint_start.player_id) {

				stopAnimation("Run", &ac);

				break;
			}
		}
		break;
	};
	case GameEvent::SPRINT_START: {
		auto view = registry->view<IDComponent, AnimationComponent>();
		for (auto entity : view) {
			auto& id_c = view.get<IDComponent>(entity);
			auto& ac = view.get<AnimationComponent>(entity);
			if (id_c.id == event.sprint_start.player_id) {

				playAnimation("SlideF", 1.f, &ac, 20, 0.f, LOOP);
				playAnimation("SlideB", 1.f, &ac, 20, 0.f, LOOP);
				playAnimation("SlideR", 1.f, &ac, 20, 0.f, LOOP);
				playAnimation("SlideL", 1.f, &ac, 20, 0.f, LOOP);

				break;
			}
		}
		break;
	};
	case GameEvent::SPRINT_END: {
		auto view = registry->view<IDComponent, AnimationComponent>();
		for (auto entity : view) {
			auto& id_c = view.get<IDComponent>(entity);
			auto& ac = view.get<AnimationComponent>(entity);
			if (id_c.id == event.sprint_end.player_id) {

				stopAnimation("SlideF", &ac);
				stopAnimation("SlideB", &ac);
				stopAnimation("SlideR", &ac);
				stopAnimation("SlideL", &ac);

				break;
			}
		}
		break;
	}
	};
}

void AnimationSystem::UpdateAnimations(entt::registry& registry, float dt) {

	updateEntities(registry, dt);

	auto animation_entities = registry.view<AnimationComponent>();
	for (auto& entity : animation_entities) {
		auto& a = animation_entities.get(entity);

		strengthModulator(&a);

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
				//Loop for the time being
				if (anim->mode_ == LOOP) {
					anim->current_frame_time_ = 0;
					for (int j = 0; j < anim->channels_.size(); j++) {
						anim->channels_.at(j).current_position_pos = 0;
						anim->channels_.at(j).current_rotation_pos = 0;
						anim->channels_.at(j).current_scaling_pos = 0;
					}
				}
				else if (anim->mode_ == MUTE_ALL) {
					anim->playing_ = false;
				}
			}
			if (!anim->playing_) {
				//reset important data
				anim->body_argument_ = -1;
				anim->current_frame_time_ = 0.f;
				anim->playing_ = false;
				for (int j = 0; j < anim->channels_.size(); j++) {
					glob::Channel* channel = &anim->channels_.at(j);
					channel->current_scaling_pos = 0;
					channel->current_position_pos = 0;
					channel->current_rotation_pos = 0;
				}
				//remove from list
				a.active_animations.erase(a.active_animations.begin() + i);
				i--;
				removedAnimation = true;
			}
			else {
				anim->current_frame_time_ += anim->speed_ * anim->tick_per_second_ * dt;
			}

			if (!removedAnimation) {
				//hell (aka bone rotation update)
				for (int j = 0; j < anim->channels_.size(); j++) {//all channels (bones)
					glob::Channel* channel = &anim->channels_.at(j);
					int jointId = (int)channel->boneID;

					if (jointId != rootBone) {

						int scalingPos = channel->current_scaling_pos;
						aiVectorKey scalingKey = channel->scaling_keys.at(scalingPos);
						glm::mat4 scaling = glm::scale(glm::vec3(scalingKey.mValue.x, scalingKey.mValue.y, scalingKey.mValue.z));

						if (anim->current_frame_time_ >= scalingKey.mTime) {
							channel->current_scaling_pos++;
						}

						float scaleInterpolation = 0.f;

						int rotationPos = channel->current_rotation_pos;
						aiQuatKey rotationKey = channel->rotation_keys.at(rotationPos);
						glm::mat4 rotation = convertToGLM3x3(rotationKey.mValue.GetMatrix());

						if (anim->current_frame_time_ >= rotationKey.mTime) {
							channel->current_rotation_pos++;
						}

						float rotationInterpolation = 0.f;

						int positionPos = channel->current_position_pos;
						aiVectorKey positionKey = channel->position_keys.at(positionPos);
						glm::mat4 position = glm::translate(glm::vec3(positionKey.mValue.x, positionKey.mValue.y, positionKey.mValue.z));

						if (anim->current_frame_time_ >= positionKey.mTime) {
							channel->current_position_pos++;
						}

						float positionInterpolation = 0.f;

						glm::mat4 combPRS = position * rotation * scaling;

						glm::mat4 finalMat = combPRS * anim->strength_;

						if (anim->mode_ == LOOP) {
							if (anim->priority_ > bonePriorities.at(jointId)) {//priority override
								if (anim->body_argument_ != -1) {
									if (isAChildOf(anim->body_argument_, jointId, &a)) {//body argument success, override
										bonePriorities.at(jointId) = anim->priority_;
										a.model_data.bones.at(jointId).transform = finalMat;
									}
								}
								else { //No body argument, override
									bonePriorities.at(jointId) = anim->priority_;
									a.model_data.bones.at(jointId).transform = finalMat;
								}
							}
							else if (anim->priority_ == bonePriorities.at(jointId)) {//blend
								if (anim->body_argument_ != -1) {
									if (isAChildOf(anim->body_argument_, jointId, &a)) {//body argument success, blend
										a.model_data.bones.at(jointId).transform += finalMat;
									}
								}
								else { //No body argument, blend
									a.model_data.bones.at(jointId).transform += finalMat;
								}
							}
						}
						else if (anim->mode_ == MUTE_ALL) {
							if (anim->priority_ >= bonePriorities.at(jointId)) {//override
								bonePriorities.at(jointId) = anim->priority_;
								a.model_data.bones.at(jointId).transform = finalMat;
							}
						}

					}
				}
			}
		}

		getDefaultPose(glm::mat4(1.f), &a.model_data.bones.at(rootBone), &a.model_data.bones, a.model_data.globalInverseTransform);

		for (auto bone : a.model_data.bones) {
			boneTransforms.at(bone.id) = bone.f_transform;
		}
		a.bone_transforms = boneTransforms;

	}

}