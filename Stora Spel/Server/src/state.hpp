#ifndef STATE_HPP_
#define STATE_HPP_
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include "ecs/components.hpp"
#include "util/timer.hpp"

class GameServer;

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
  void Init(GameServer& game_server);
  void Update(GameServer& game_server);
  virtual void Cleanup(GameServer& game_server);
  LobbyState() = default;
  ~LobbyState() {}

 private:
  Timer start_game_timer;
};
class PlayState : public State {
 public:
  virtual void Init(GameServer& game_server);
  virtual void Update(GameServer& game_server);
  virtual void Cleanup(GameServer& game_server);
  PlayState() = default;
  ~PlayState() {}

 private:
};
#endif  // !STATE_HPP_
