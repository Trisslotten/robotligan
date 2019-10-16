#ifndef DETERMINISTIC_REPLAY_HPP_
#define DETERMINISTIC_REPLAY_HPP_

#include <bitset>

#include "bit_pack.hpp"
#include "reg_pack.hpp"

class DeterministicReplay {
 private:
  // Variables : Input log
  BitPack* input_log_;
  unsigned int bits_per_int_;
  std::bitset<10> last_input_state_;
  std::bitset<10> last_output_state_;

  // Variables : Snapshot log
  RegPack* registry_log_;
  unsigned int snapshot_interval_frames_;
  unsigned int frames_since_last_snapshot_;

  // Variables : Coupling input and state logs
  unsigned int* frame_indices_;
  unsigned int curr_snapshot_index_;

  // Functions : Writing and reading input frames
  void WriteInputFrame(const std::bitset<10>& in_bitset,
                       const float& in_x_value, const float& in_y_value);
  void ReadInputFrame(std::bitset<10>& in_bitset, float& in_x_value,
                      float& in_y_value);

 public:
  DeterministicReplay(unsigned int in_num_of_frames,
                      unsigned int in_num_of_keys,
                      unsigned int in_num_of_players,
                      unsigned int in_snapshot_interval_frames);
  ~DeterministicReplay();

  bool SaveFrame(std::bitset<10>& in_bitset, float& in_x_value,
                 float& in_y_value, entt::registry& in_registry);
  bool LoadFrame(std::bitset<10>& in_bitset, float& in_x_value,
                 float& in_y_value, entt::registry& in_registry);
};

#endif  // !DETERMINISTIC_REPLAY_HPP_
