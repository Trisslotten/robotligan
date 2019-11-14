#ifndef CLIENT_REPLAY_MACHINE_HPP_
#define CLIENT_REPLAY_MACHINE_HPP_

#include <vector>

#include "geometric_replay.hpp"

class Engine;

class ClientReplayMachine {
 private:
  GeometricReplay* primary_replay_;
  unsigned int replay_length_sec_;
  unsigned int replay_frames_per_sec_;
  
  std::vector<GeometricReplay*> stored_replays_;
  unsigned int selected_replay_index_;
  Engine* engine_;

 public:
  ClientReplayMachine(unsigned int in_replay_length_sec,
                      unsigned int in_frames_per_sec);
  ~ClientReplayMachine();

  void RecordFrame(entt::registry& in_registry);
  
  void StoreReplay();
  unsigned int NumberOfStoredReplays() const;
  int CurrentlySelectedReplay() const;
  bool SelectReplay(unsigned int in_index);
  void ResetSelectedReplay();

  bool LoadFrame(entt::registry& in_registry);
  

  std::string GetSelectedReplayStringTree();
  std::string GetSelectedReplayStringState();

  void SetEngine(Engine* eng) {
    engine_ = eng;
    primary_replay_->SetEngine(eng);
  }
  void ReceiveGameEvent(GameEvent event);
  
};

#endif  // !CLIENT_REPLAY_MACHINE_HPP_
