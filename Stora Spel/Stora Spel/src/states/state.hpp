#ifndef STATE_HPP_
#define STATE_HPP_

#include <NetAPI\packet.hpp>
#include <ecs/components/button_component.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <glob/graphics.hpp>
#include <list>
#include <playerdata.hpp>
#include <util/timer.hpp>
#include "Chat.hpp"
#include "eventdispatcher.hpp"
#include "shared/shared.hpp"

class Engine;

enum class StateType {
  MAIN_MENU,
  CONNECT_MENU,
  LOBBY,
  PLAY,
  SETTINGS,
};

class State {
 public:
  // when program starts
  virtual void Startup() = 0;

  // when state changed to this state
  virtual void Init() = 0;

  virtual void Update(float dt) = 0;

  virtual void UpdateNetwork() = 0;

  // when state becomes inactive
  virtual void Cleanup() = 0;

  // the type of state
  virtual StateType Type() = 0;

  void SetEngine(Engine* engine) { engine_ = engine; }

  State() = default;
  ~State() {}

 protected:
  Engine* engine_ = nullptr;

 private:
};

/////////////////////// MAIN MENU ///////////////////////

class MainMenuState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update(float dt) override;
  void UpdateNetwork() override;
  void Cleanup() override;

  StateType Type() { return StateType::MAIN_MENU; }

 private:
  void CreateMainMenu();
  void CreateInformationMenu();
  void CreateBackgroundEnitites();

  entt::registry registry_mainmenu_;
  entt::registry registry_settings_;
  entt::registry registry_information_;

  glob::Font2DHandle font_test_ = 0;
  glob::GUIHandle information_image_ = 0;
  glob::GUIHandle loggo_image_ = 0;
};

/////////////////////// LOBBY ///////////////////////

struct LobbyPlayer {
  unsigned int team;
  bool ready;
};

class LobbyState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update(float dt) override;
  void UpdateNetwork() override;
  void Cleanup() override;

  StateType Type() { return StateType::LOBBY; }

  void HandleUpdateLobbyTeamPacket(NetAPI::Common::Packet& packet);
  void HandlePlayerDisconnect(NetAPI::Common::Packet& packet);
  void SetMyId(int client_id) { my_id_ = client_id; }

 private:
  entt::registry registry_lobby_;
  void CreateBackgroundEntities();
  void CreateGUIElements();
  void DrawTeamSelect();
  void DrawAbilitySelect();
  glob::GUIHandle team_select_back_;
  glob::GUIHandle ability_select_back_;
  glob::GUIHandle ability_back_normal_;
  glob::GUIHandle ability_back_hover_;
  glob::GUIHandle ability_back_selected_;
  glob::GUIHandle ready_back_normal_;
  glob::GUIHandle ready_back_hover_;
  glob::GUIHandle ready_icon_;
  glob::GUIHandle ready_empty_icon_;
  glob::GUIHandle chatbox_back_;

  ServerStateType server_state_;
  std::vector<glob::GUIHandle> ability_icons_;
  std::vector<std::string> ability_tooltips_;
  glob::Font2DHandle font_team_names_;
  glob::Font2DHandle font_test_;
  std::unordered_map<int, LobbyPlayer> lobby_players_;

  void ReadyButtonFunc();
  ButtonComponent* ready_button_c = nullptr;
  bool me_ready_ = false;

  void SendJoinTeam(unsigned int team);
  int my_id_ = 0;
  int my_selected_ability_ = 1;

  entt::entity GetAbilityButton(std::string find_string);
  void SelectAbilityHandler(int id);

  bool IsAbilityBlackListed(int id);
  std::vector<int> ability_blacklist;

  void SendMyName();
};
/////////////////////// ConnectMenuState
class ConnectMenuState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update(float dt) override;
  void UpdateNetwork() override;
  void Cleanup() override;
  StateType Type() { return StateType::CONNECT_MENU; }

 private:
  int isconnected_ = 0;
  struct InputField {
    InputField(){};
    InputField(glm::vec2 in_size, glm::vec2 in_pos,
               std::string initial_text = "") {
      size = in_size;
      pos = in_pos;
    }
    bool focus = false;
    std::string input_field = "";
    glm::vec2 size = glm::vec2(0.0f, 0.0f);
    glm::vec2 pos = glm::vec2(0.0f, 0.0f);
    glob::GUIHandle hndl = 0;
  };
  int frames_ = 0;
  bool connection_success_ = true;
  std::string last_msg_ = "Failed to connect: Timeout";
  std::string ip_ = "localhost";
  std::string port_ = "1337";
  glob::Font2DHandle font_test_ = 0;
  entt::registry registry_connect_menu_;
};

/////////////////////// SETTINGS ///////////////////
class SettingsState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update(float dt) override;
  void UpdateNetwork() override;
  void Cleanup() override;
  StateType Type() { return StateType::SETTINGS; }

 private:
  void CreateSettingsMenu();
  void SaveSettings();
  glob::Font2DHandle font_test_ = 0;
  entt::registry registry_settings_;

  float setting_fov_ = 90.f;
  float setting_volume_ = 100.f;
  float setting_mouse_sens_ = 1.0f;

  std::string setting_username_ = "fel";
};

/////////////////////// PLAY ///////////////////////

class PlayState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update(float dt) override;
  void UpdateNetwork() override;
  void Cleanup() override;

  StateType Type() { return StateType::PLAY; }

  void SetEntityTransform(EntityID player_id, glm::vec3 pos,
                          glm::quat orientation);
  void SetEntityPhysics(EntityID player_id, glm::vec3 vel, bool is_airborne);
  void SetCameraOrientation(glm::quat orientation);
  void SetEntityIDs(std::vector<EntityID> player_ids, EntityID my_id,
                    EntityID ball_id) {
    player_ids_ = player_ids;
    my_id_ = my_id;
    ball_id_ = ball_id;
  }
  void SetCurrentStamina(float stamina) { current_stamina_ = stamina; }
  auto* GetReg() { return &registry_gameplay_; }

  void CreateWall(EntityID id, glm::vec3 position, glm::quat rotation);
  void CreatePickUp(EntityID id, glm::vec3 position);
  void CreateCannonBall(EntityID id);
  void CreateTeleportProjectile(EntityID id);
  void CreateForcePushObject(EntityID id);
  void CreateMissileObject(EntityID id);
  void DestroyEntity(EntityID id);
  void SwitchGoals();
  void SetMyPrimaryAbility(int id) { my_primary_ability_id = id; }
  void SetMatchTime(int time, int countdown_time) {
    match_time_ = time;
    countdown_time_ = countdown_time;
  }
  void SetPlayerLookDir(EntityID id, glm::vec3 look_dir);
  void SetPlayerMoveDir(EntityID id, glm::vec3 move_dir);
  void SetMyTarget(EntityID id) { my_target_ = id; }
  void ReceiveGameEvent(const GameEvent& e);
  void Reset();
  void EndGame();
  void TestParticles();

  void OnServerFrame();
  void AddAction(int action) { actions_.push_back(action); }
  void ClearActions() { actions_.clear(); }

  void UpdateHistory(int id) {
    while (history_.size() > 0 && history_.front().id <= id)
      history_.pop_front();
  }
  void AddPitchYaw(float pitch, float yaw);
  void SetPitchYaw(float pitch, float yaw);
  auto* GetPlayerIDs() { return &player_ids_; }

  float GetPitch() { return pitch_; }
  float GetYaw() { return yaw_; }
  void SetTeam(unsigned int team) {my_team_ = team;}
  void CreateNewBallEntity(bool fake, EntityID id);
  void SetTeam(EntityID id, unsigned int team) { teams_[id] = team; }

 private:
  ServerStateType server_state_;
  void CreateInitialEntities();
  void CreatePlayerEntities();
  void CreateArenaEntity();
  void CreateBallEntity();
  void CreateInGameMenu();

  void TestCreateLights();

  void ToggleInGameMenu();
  void UpdateInGameMenu(bool show_menu);
  void UpdateGameplayTimer();
  void UpdateSwitchGoalTimer();

  void DrawNameOverPlayer();

  void DrawTopScores();
  void DrawTarget();
  void DrawQuickslots();
  FrameState SimulateMovement(std::vector<int>& action, FrameState& state,
                              float dt);
  void MovePlayer(float dt);
  void MoveBall(float dt);
  void Collision();

  EntityID ClientIDToEntityID(long client_id);
  ////////////////////////////////////////

  entt::registry registry_gameplay_;

  std::vector<EntityID> player_ids_;
  EntityID my_id_, ball_id_;
  float current_stamina_ = 0.f;

  std::unordered_map<EntityID, std::pair<glm::vec3, glm::quat>> transforms_;
  std::unordered_map<EntityID, std::pair<glm::vec3, glm::quat>> new_transforms_;
  std::unordered_map<EntityID, glm::vec3> player_look_dirs_;
  std::unordered_map<EntityID, glm::vec3> player_move_dirs_;
  FrameState server_predicted_;
  entt::entity my_entity_, arena_entity_;

  std::unordered_map<EntityID, std::pair<glm::vec3, bool>> physics_;

  std::unordered_map<EntityID, unsigned int> teams_;

  entt::entity blue_goal_light_;
  entt::entity red_goal_light_;

  glob::Font2DHandle font_test_ = 0;
  glob::Font2DHandle font_scores_ = 0;
  glob::E2DHandle e2D_test_, e2D_test2_, e2D_target_;
  glob::GUIHandle in_game_menu_gui_ = 0;
  glob::GUIHandle gui_test_, gui_teamscore_, gui_stamina_base_,
      gui_stamina_fill_, gui_stamina_icon_, gui_quickslots_, gui_minimap_,
      gui_minimap_goal_red_, gui_minimap_goal_blue_, gui_minimap_player_red_,
      gui_minimap_player_blue_, gui_minimap_ball_;

  std::vector<glob::GUIHandle> ability_handles_;

  bool show_in_game_menu_buttons_ = false;

  int my_primary_ability_id = 0;
  unsigned int my_team_;
  int match_time_ = 300;
  int countdown_time_ = 5;
  // For switch goal
  bool countdown_in_progress_ = false;

  Timer end_game_timer_;
  bool game_has_ended_ = false;
  bool goals_swapped_ = false;
  EntityID my_target_ = -1;

  glob::ModelHandle test_ball_;
  std::list<PlayerData> history_;
  FrameState predicted_state_;

  std::vector<int> actions_;
  int frame_id = 0;
  float pitch_ = 0.0f;
  float yaw_ = 0.0f;
  
  float timer_ = 0.0f;
  float primary_cd_ = 0.0f;
};

#endif  // STATE_HPP_