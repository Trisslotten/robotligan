#ifndef REPLAY_MACHINE_HPP_
#define REPLAY_MACHINE_HPP_

#include <math.h>
#include <bitset>

#include <entity/registry.hpp>
#include <entity/snapshot.hpp>

#include "deterministic_replay.hpp"
#include "assert_module.hpp"

class ReplayMachine {
 private:
  // Variables	:	Deterministic replay
  DeterministicReplay* replay_deterministic_;
  float recording_max_seconds_;
  float recording_elapsed_seconds_;

  PlayerIO* player_io_arr_;
  unsigned int num_of_players_;

  // Variables	:	Assert mode
  AssertModule* assert_module_;
  bool assert_mode_on_;

 public:
  ReplayMachine();
  ~ReplayMachine();

  // NTS:	Delete copy constructor
  //		and assignment operator
  ReplayMachine(ReplayMachine&) = delete;
  void operator=(ReplayMachine const&) = delete;

  void Init(unsigned int in_seconds, unsigned int in_frames_per_second,
            float in_snapshot_interval_seconds, unsigned int in_num_of_players, bool in_asset_mode);

  bool SaveReplayFrame(std::bitset<10>& in_bitset, float& in_x_value,
                       float& in_y_value, entt::registry& in_registry,
                       const float& in_dt, unsigned int in_player_index);
  bool LoadReplayFrame(std::bitset<10>& in_bitset, float& in_x_value,
                       float& in_y_value, entt::registry& in_registry,
                       unsigned int in_player_index);
};

#endif  // !REPLAY_MACHINE_HPP_