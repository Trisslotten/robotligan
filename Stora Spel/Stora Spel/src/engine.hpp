#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <NetAPI/socket/Client.hpp>
#include <NetAPI/socket/tcpclient.hpp>
#include <entt.hpp>
#include <glob/graphics.hpp>
#include <limits>
#include <unordered_map>
#include <util/global_settings.hpp>
#include <vector>
#include "Chat.hpp"
#include "client replay machine/client_replay_machine.hpp"
#include "ecs/systems/animation_system.hpp"
#include "ecs/systems/sound_system.hpp"
#include "ecs/systems/particle_system.hpp"
#include "shared/shared.hpp"
#include "states/state.hpp"
struct PlayerStatInfo {
  int points = 0;
  int goals = 0;
  int assists = 0;
  int saves = 0;
  unsigned int team = TEAM_RED;
  EntityID enttity_id = 0;
};
class Engine {
 public:
  Engine();
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  bool should_quit = false;
  int IsConnected() { return (int)(client_.IsConnected() * 2); }
  void Init();
  void Update(float dt);
  void UpdateNetwork();
  void Render();
  void UpdateSettingsValues() {
    sound_system_.GetSoundEngine().SetMasterVolume(
        GlobalSettings::Access()->ValueOf("SOUND_VOLUME") / 100.f);
    glob::GetCamera().SetFov(GlobalSettings::Access()->ValueOf("GRAPHICS_FOV"));
    mouse_sensitivity_ = GlobalSettings::Access()->ValueOf("INPUT_MOUSE_SENS");
  }

  void SetCurrentRegistry(entt::registry* registry);
  void ChangeState(StateType state) {
    wanted_state_type_ = state;
    previous_state_ = current_state_->Type();
    UpdateSettingsValues();
  }
  NetAPI::Socket::Client& GetClient() { return client_; }
  NetAPI::Common::Packet& GetPacket() { return packet_; }
  void SetTakeInput(bool should_take) { take_game_input_ = should_take; }
  void SetSendInput(bool should_send) { should_send_input_ = should_send; }
  void SetEnableChat(bool should_enable) { this->enable_chat_ = should_enable; }
  SoundSystem& GetSoundSystem() { return sound_system_; }
  slob::SoundEngine& GetSoundEngine() { return sound_system_.GetSoundEngine(); }
  AnimationSystem& GetAnimationSystem() { return animation_system_; }
  entt::registry* GetCurrentRegistry() { return registry_current_; }
  std::unordered_map<int, int> GetKeyBinds() { return keybinds_; };

  std::unordered_map<long, std::string> player_names_;
  void SetSecondaryAbility(AbilityID id) { second_ability_ = id; }
  AbilityID GetSecondaryAbility() { return second_ability_; }
  std::vector<unsigned int> GetTeamScores() { return scores_; }
  std::vector<int>* GetPlayingPlayers();
  void SetPlayingPlayers(std::unordered_map<int, LobbyPlayer> plyrs) {
    playing_players_ = plyrs;
  }
  int GetGameplayTimer() const;
  int GetCountdownTimer() const;
  float GetSwitchGoalCountdownTimer() const;
  int GetSwitchGoalTime() const;
  unsigned int GetPlayerTeam(EntityID id) {
    for (auto p_score : player_scores_) {
      if (p_score.second.enttity_id == id) {
        return p_score.second.team;
      }
    }
    return TEAM_RED;
  }

  void DrawScoreboard();

  Chat* GetChat() { return &chat_; }

  StateType GetPreviousStateType() { return previous_state_; }
  ServerStateType GetServerState() { return server_state_; }
  void SetServerState(ServerStateType state) { server_state_ = state; }
  void ReInit() {
    play_state_.Cleanup();
    play_state_.Init();
  }

  std::unordered_map<long, PlayerStatInfo> GetPlayerScores() {
    return player_scores_;
  }

  // Replay stuff---
  ClientReplayMachine* GetReplayMachinePtr() { return this->replay_machine_; }
  bool IsRecording() const { return this->play_state_.IsRecording(); }
  // Replay stuff---

 private:
  void SetKeybinds();

  void UpdateChat(float dt);
  void UpdateSystems(float dt);
  void HandlePacketBlock(NetAPI::Common::Packet& packet);

  NetAPI::Socket::Client client_;
  NetAPI::Common::Packet packet_;
  int server_connected_ = 0;

  std::vector<unsigned> client_pings_;
  StateType wanted_state_type_ = StateType::MAIN_MENU;
  State* current_state_ = nullptr;
  MainMenuState main_menu_state_;
  LobbyState lobby_state_;
  PlayState play_state_;
  ReplayState replay_state_;
  ConnectMenuState connect_menu_state_;
  SettingsState settings_state_;
  CreateServerState create_server_state_;

  // Registry
  entt::registry* registry_current_;
  std::unordered_map<int, LobbyPlayer> playing_players_;
  bool should_send_input_ = false;

  std::unordered_map<int, int> keybinds_;
  std::unordered_map<int, int> mousebinds_;
  std::unordered_map<int, int> key_presses_;
  std::unordered_map<int, int> mouse_presses_;

  glob::Font2DHandle font_test_ = 0;
  glob::Font2DHandle font_test2_ = 0;
  glob::Font2DHandle font_test3_ = 0;

  glob::GUIHandle gui_scoreboard_back_ = 0;

  bool take_game_input_ = true;

  // TODO: move to states
  std::vector<unsigned int> scores_;

  int gameplay_timer_sec_ = 0;
  int countdown_timer_sec_ = 0;
  float switch_goal_timer_ = 0.f;
  int switch_goal_time_ = 0;

  Chat chat_;
  std::string message_ = "";

  bool enable_chat_ = false;
  ServerStateType server_state_;
  SoundSystem sound_system_;
  AnimationSystem animation_system_;
  ParticleSystem particle_system_;

  AbilityID second_ability_ = AbilityID::NULL_ABILITY;
  unsigned int new_team_ = std::numeric_limits<unsigned int>::max();

  std::unordered_map<long, PlayerStatInfo> player_scores_;

  StateType previous_state_;

  float mouse_sensitivity_ = 1.0f;

  std::list<NetAPI::Common::Packet> packet_test;
  std::list<float> time_test;

  // Replay Variables ---
  ClientReplayMachine* replay_machine_ = nullptr;
  // Replay Variables ---
};

#endif  // ENGINE_HPP_