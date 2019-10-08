#ifndef REPLAY_MACHINE_HPP_
#define REPLAY_MACHINE_HPP_

#include <math.h>
#include <bitset>

#include <entity/registry.hpp>
#include <entity/snapshot.hpp>

#include "deterministic_replay.hpp"
//#include "bit_pack.hpp"
#include "reg_pack.hpp"

class ReplayMachine {
 private:
  // Variables : Deterministic replay
  DeterministicReplay* replay_deterministic_;

  //---

  // Variables : State log for assert mode
  RegPack* assert_log_;
  bool assert_mode_on_;

  //---

 public:
  ReplayMachine();
  ~ReplayMachine();

  // NTS:	Delete copy constructor
  //		and assignment operator
  ReplayMachine(ReplayMachine&) = delete;
  void operator=(ReplayMachine const&) = delete;

  void Init(unsigned int in_seconds, unsigned int in_frames_per_second,
            float in_snapshot_interval_seconds);

  bool SaveReplayFrame(std::bitset<10>& in_bitset, float& in_x_value,
                       float& in_y_value, entt::registry& in_registry,
                       const float& in_dt);
  bool LoadReplayFrame(std::bitset<10>& in_bitset, float& in_x_value,
                       float& in_y_value, entt::registry& in_registry);
};

#endif  // !REPLAY_MACHINE_HPP_