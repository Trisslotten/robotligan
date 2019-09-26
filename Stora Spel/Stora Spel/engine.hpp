#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <NetAPI/socket/tcpclient.hpp>
#undef min
#undef max
#include <entt.hpp>
#include <glob/graphics.hpp>
#include <unordered_map>
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

  void SetCurrentRegistry(entt::registry* registry);

  void SetPriorRegistry(entt::registry* registry);

 private:
  void UpdateSystems(float dt);

  void CreateMainMenu();
  void CreateSettingsMenu();
  void CreateInGameMenu();

  NetAPI::Socket::TcpClient tcp_client_;
  entt::registry registry_gameplay_;
  entt::registry registry_mainmenu_;
  entt::registry registry_settings_;
  entt::registry registry_in_game_menu_;
  entt::registry* registry_prior_to_;
  entt::registry* registry_current_;

  std::unordered_map<int, PlayerAction> keybinds_;
  std::unordered_map<int, PlayerAction> mousebinds_;

  glob::Font2DHandle font_test_ = 0;

  bool mouse_locked_ = true;
};

#endif  // ENGINE_HPP_