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

  bool bonesSpecified() {
    return (body_include_->size() > 0 || body_exclude_->size() > 0);
  }
};
}  // namespace glob

#endif PLAYABLE_ANIMATION_HPP_