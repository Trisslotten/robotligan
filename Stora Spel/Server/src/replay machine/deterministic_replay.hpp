#ifndef DETERMINISTIC_REPLAY_HPP_
#define DETERMINISTIC_REPLAY_HPP_

#include <bitset>

#include "bit_pack.hpp"
#include "reg_pack.hpp"

struct PlayerIO {
  std::bitset<10> key_bitset;
  float x_value;
  float y_value;
};

class DeterministicReplay {
 private:
  // Structs :
  struct PlayerIOState {
    std::bitset<10> last_input_state;
    std::bitset<10> last_output_state;
  };

  // Variables : Input log
  BitPack* input_log_;
  unsigned int bits_per_int_;
  PlayerIOState* player_io_;
  unsigned int num_of_players_;

  // Variables : Snapshot log
  RegPack* registry_log_;
  unsigned int snapshot_interval_frames_;
  unsigned int frames_since_last_snapshot_;

  // Variables : Coupling input and state logs
  unsigned int* frame_indices_;
  unsigned int curr_snapshot_index_;

  // Functions : Writing and reading input frames
  void WriteInputFrame(const std::bitset<10>& in_bitset,
                       const float& in_x_value, const float& in_y_value,
                       const unsigned int in_player_num);
  void ReadInputFrame(std::bitset<10>& in_bitset, float& in_x_value,
                      float& in_y_value, const unsigned int in_player_num);

 public:
  DeterministicReplay(unsigned int in_num_of_frames,
                      unsigned int in_num_of_keys,
                      unsigned int in_num_of_players,
                      unsigned int in_snapshot_interval_frames);
  ~DeterministicReplay();

  bool SaveFrame(PlayerIO in_pio[], entt::registry& in_registry);
  bool LoadFrame(PlayerIO in_pio[], entt::registry& in_registry);
};

#endif  // !DETERMINISTIC_REPLAY_HPP_
