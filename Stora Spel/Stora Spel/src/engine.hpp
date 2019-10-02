#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <NetAPI/socket/Client.hpp>
#include <NetAPI/socket/tcpclient.hpp>
#include <entt.hpp>
#include <glob/graphics.hpp>
#include <unordered_map>
#include "shared/shared.hpp"
#include "states/state.hpp"

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
  void ChangeState(StateType state) {
    wanted_state_type_ = state;
  }
  NetAPI::Socket::Client& getClient() {
    return client_;
  }

 private:
  void SetKeybinds();

  void UpdateSystems(float dt);
  void HandlePacketBlock(NetAPI::Common::Packet& packet);

  NetAPI::Socket::Client client_;

  StateType wanted_state_type_ = StateType::MAIN_MENU;
  State* current_state_ = nullptr;
  MainMenuState main_menu_state_;
  LobbyState lobby_state_;
  PlayState play_state_;

  entt::registry* registry_current_;

  PlayerID my_id = -1;

  std::unordered_map<int, int> keybinds_;
  std::unordered_map<int, int> mousebinds_;
  std::unordered_map<int, int> key_presses_;
  std::unordered_map<int, int> mouse_presses_;
  float accum_yaw_ = 0.f;
  float accum_pitch_ = 0.f;

  glob::Font2DHandle font_test_ = 0;
  glob::Font2DHandle font_test2_ = 0;
  glob::Font2DHandle font_test3_ = 0;
};

#endif  // ENGINE_HPP_