#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <NetAPI/socket/Client.hpp>
#include <NetAPI/socket/tcpclient.hpp>
#include <entt.hpp>
#include <glob/graphics.hpp>
#include <limits>
#include <unordered_map>
#include "Chat.hpp"
#include "shared/shared.hpp"
#include "states/state.hpp"

struct PlayerScoreBoardInfo {
  int points = 0;
  int goals = 0;
  unsigned int team = TEAM_RED;
};

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
  void ChangeState(StateType state) { wanted_state_type_ = state; }
  NetAPI::Socket::Client& GetClient() { return client_; }
  NetAPI::Common::Packet& GetPacket() { return packet_; }
  void SetSendInput(bool should_send) { should_send_input_ = should_send; }
  void SetEnableChat(bool should_enable) { this->enable_chat_ = should_enable; }

  std::unordered_map<PlayerID, std::string> player_names_;

  AbilityID GetSecondaryAbility() { return second_ability_; }

 std::vector<unsigned int> GetTeamScores() { return scores_; }

  int GetGameplayTimer() const;
  int GetCountdownTimer() const;

  Chat* GetChat() { return &chat; }

 private:
  void SetKeybinds();

  void UpdateChat(float dt);
  void UpdateSystems(float dt);
  void HandlePacketBlock(NetAPI::Common::Packet& packet);

  void DrawScoreboard();

  NetAPI::Socket::Client client_;
  NetAPI::Common::Packet packet_;

  StateType wanted_state_type_ = StateType::MAIN_MENU;
  State* current_state_ = nullptr;
  MainMenuState main_menu_state_;
  LobbyState lobby_state_;
  PlayState play_state_;
  ConnectMenuState connect_menu_state_;
  entt::registry* registry_current_;

  bool should_send_input_ = false;

  std::unordered_map<int, int> keybinds_;
  std::unordered_map<int, int> mousebinds_;
  std::unordered_map<int, int> key_presses_;
  std::unordered_map<int, int> mouse_presses_;
  float accum_yaw_ = 0.f;
  float accum_pitch_ = 0.f;

  glob::Font2DHandle font_test_ = 0;
  glob::Font2DHandle font_test2_ = 0;
  glob::Font2DHandle font_test3_ = 0;

  glob::GUIHandle gui_scoreboard_back_ = 0;

  bool take_game_input_ = true;

  // TODO: move to states
  std::vector<unsigned int> scores_;

  int gameplay_timer_sec_ = 0;
  int countdown_timer_sec_ = 0;

  Chat chat;
  std::string message_ = "";

  bool enable_chat_ = false;

  AbilityID second_ability_ = AbilityID::NULL_ABILITY;
  unsigned int new_team_ = std::numeric_limits<unsigned int>::max();

  std::unordered_map<PlayerID, PlayerScoreBoardInfo> player_scores_;
};

#endif  // ENGINE_HPP_