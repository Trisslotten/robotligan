#ifndef CLIENT_REPLAY_MACHINE_HPP_
#define CLIENT_REPLAY_MACHINE_HPP_

#include <vector>

#include <util/timer.hpp>

#include "geometric_replay.hpp"


class ClientReplayMachine {
 private:
  GeometricReplay* primary_replay_;
  unsigned int replay_length_sec_;
  unsigned int replay_frames_per_sec_;
  
  std::vector<GeometricReplay*> stored_replays_;
  unsigned int selected_replay_index_;
  
  Timer replay_timer_;

 public:
  ClientReplayMachine(unsigned int in_replay_length_sec,
                      unsigned int in_frames_per_sec);
  ~ClientReplayMachine();

  void RecordFrame(entt::registry& in_registry);
  
  void StoreReplay();
  unsigned int NumberOfStoredReplays() const;
  int CurrentlySelectedReplay() const;
  bool SelectReplay(unsigned int in_index);
  bool LoadFrame(entt::registry& in_registry);

  std::string GetSelectedReplayString();
};

#endif  // !CLIENT_REPLAY_MACHINE_HPP_
