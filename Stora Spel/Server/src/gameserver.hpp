#ifndef GAME_SERVER_HPP_
#define GAME_SERVER_HPP_

#include <entt.hpp>
#include <entity/registry.hpp>
#include <glm/glm.hpp>
#include <NetAPI/socket/server.hpp>
#include "util/timer.hpp"

class GameServer {
 public:
  ~GameServer();
  void Init();
  void Update(float dt);
 private:
  NetAPI::Socket::Server server_;

  entt::registry registry_;

  glm::vec3 testpos_{0};

  Timer global_timer;
};

#endif  // GAME_SERVER_HPP_