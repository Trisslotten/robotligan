#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <NetAPI/socket/tcpclient.hpp>
#include <entt.hpp>
#include <unordered_map>
#include <glob/graphics.hpp>
#include "shared/shared.hpp"

class Engine {
 public:
  Engine();
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;

  void Init();

  void Update(float dt);

  void UpdateNetwork();

  void Render();

 private:
  void UpdateSystems(float dt);
  
  NetAPI::Socket::TcpClient tcp_client_;
  entt::registry registry_;

  std::unordered_map<int, PlayerAction> keybinds_;
  std::unordered_map<int, PlayerAction> mousebinds_;

  glob::Font2DHandle font_test_ = 0;
};

#endif  // ENGINE_HPP_