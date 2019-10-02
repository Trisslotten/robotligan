#ifndef STATE_HPP_
#define STATE_HPP_
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <shared.hpp>
#include "ecs/components.hpp"
#include "util/timer.hpp"

class GameServer;

enum class StateType {
  LOBBY = 0,
  PLAY,
};

class State {
 public:
  virtual void Init(GameServer& game_server) = 0;
  virtual void Update(GameServer& game_server) = 0;
  virtual void Cleanup(GameServer& game_server) = 0;
  State() = default;
  ~State() {}

 private:
};

class LobbyState : public State {
 public:
  void Init(GameServer& game_server) override;
  void Update(GameServer& game_server) override;
  void Cleanup(GameServer& game_server) override;
  LobbyState() = default;
  ~LobbyState() {}

  void SetClientIsReady(int client_id, bool is_ready) {
  clients_ready_[client_id] = is_ready;
  }

 private:
  std::unordered_map<int, bool> clients_ready_;
  Timer start_game_timer;
  bool starting = false;
};

class PlayState : public State {
 public:
  void Init(GameServer& game_server) override;
  void Update(GameServer& game_server) override;
  void Cleanup(GameServer& game_server) override;
  PlayState() = default;
  ~PlayState() {}

  void SetPlayerInput(int client_id, uint16_t actions, float pitch, float yaw) {
    players_inputs_[client_id] = std::make_pair(actions, glm::vec2(pitch, yaw));
  }

 private:
  std::unordered_map<int, std::pair<uint16_t, glm::vec2>> players_inputs_;
};

#endif  // !STATE_HPP_
