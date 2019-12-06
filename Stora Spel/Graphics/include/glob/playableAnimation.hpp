#ifndef PLAYABLE_ANIMATION_HPP_
#define PLAYABLE_ANIMATION_HPP_

#include "Animation.hpp"

namespace glob {
class PlayableAnimation {
 private:
 public:
  int id_ = -1;
  glob::Animation* animation_;

  float time_;
  char priority_;
  float strength_;
  int mode_ = 0;
  float speed_;

  bool playing_ = false; //marks this to be removed from the active list by the system

  std::vector<int>* body_include_ = new std::vector<int>;
  std::vector<int>* body_exclude_ = new std::vector<int>;

  std::vector<glm::vec3>* bone_position_;
  std::vector<glm::vec3>* bone_scale_;
  std::vector<glm::quat>* bone_rotation_;

  std::vector<int> modes;

  bool stopping_ = false;
  float fade_ = 0.f;

  bool bonesSpecified() {
    return (body_include_->size() > 0 || body_exclude_->size() > 0);
  }

  void clear() {
    delete bone_position_;
    delete bone_rotation_;
    delete bone_scale_;

    delete body_include_;
    delete body_exclude_;
  }
};
}  // namespace glob

#endif PLAYABLE_ANIMATION_HPP_