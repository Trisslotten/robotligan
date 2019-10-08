#include "replay_machine.hpp"

// Private---------------------------------------------------------------------

// Public----------------------------------------------------------------------
ReplayMachine::ReplayMachine() {
  // H
  // I
  this->replay_deterministic_ = nullptr;
  this->assert_log_ = nullptr;
  this->assert_mode_on_ = false;
}

ReplayMachine::~ReplayMachine() {
  // B
  // Y
  // E
  if (this->replay_deterministic_ != nullptr) {
    delete this->replay_deterministic_;
  }
  if (this->assert_log_ != nullptr) {
    delete this->assert_log_;
  }
}

void ReplayMachine::Init(unsigned int in_seconds,
                         unsigned int in_frames_per_second,
                         float in_snapshot_interval_seconds) {
  // Calculate values required for a deterministic
  // replay and allocate space for one of that size
  unsigned int max_num_of_frames = in_frames_per_second * in_seconds;
  unsigned int frames_between_snapshots =
      in_frames_per_second * in_snapshot_interval_seconds;
  this->replay_deterministic_ =
      new DeterministicReplay(max_num_of_frames, 10, frames_between_snapshots);
}

bool ReplayMachine::SaveReplayFrame(std::bitset<10>& in_bitset,
                                    float& in_x_value, float& in_y_value,
                                    entt::registry& in_registry,
                                    const float& in_dt) {
  // Returns true if end if buffer was reached

  return this->replay_deterministic_->SaveFrame(in_bitset, in_x_value,
                                                in_y_value, in_registry);
}

bool ReplayMachine::LoadReplayFrame(std::bitset<10>& in_bitset,
                                    float& in_x_value, float& in_y_value,
                                    entt::registry& in_registry) {
  // Returns true if end if buffer was reached

  return this->replay_deterministic_->LoadFrame(in_bitset, in_x_value,
                                                in_y_value, in_registry);
}
