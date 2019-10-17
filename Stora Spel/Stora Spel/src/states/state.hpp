#ifndef STATE_HPP_
#define STATE_HPP_

#include <list>
#include <NetAPI\packet.hpp>
#include <ecs/components/button_component.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <glob/graphics.hpp>
#include <util/timer.hpp>
#include "Chat.hpp"
#include "shared/shared.hpp"
#include <ecs/components/button_component.hpp>
#include <playerdata.hpp>

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

  entt::registry registry_mainmenu_;
  entt::registry registry_settings_;
  entt::registry registry_information_;

  glob::Font2DHandle font_test_ = 0;
  glob::GUIHandle information_image_ = 0;
  // Inherited via State
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

  std::vector<glob::GUIHandle> ability_icons_;
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
  struct InputField {
    InputField() {};
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
  std::string ip_ = "localhost";
  std::string port_ = "1337";
  InputField ip_field_;
  InputField port_field_;
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
  void SetMyTarget(EntityID id) { my_target_ = id; }
  void ReceiveGameEvent(const GameEvent& e);
  void Reset();
  void EndGame();
  void TestParticles();

  void OnServerFrame();
  void AddAction(int action) { actions_.push_back(action); }
  void ClearActions() {
    actions_.clear();
    actions_.push_back(100);
  }

  void UpdateHistory(int id) { while (history_.size() > 0 && history_.front().id <= id) history_.pop_front(); }
  void SetPitchYaw(float pitch, float yaw);

 private:
  void CreateInitialEntities();
  void CreatePlayerEntities();
  void CreateArenaEntity();
  void CreateBallEntity();
  void CreateInGameMenu();

  void TestCreateLights();

  void ToggleInGameMenu();
  void UpdateInGameMenu(bool show_menu);
  void UpdateGameplayTimer();

  void DrawTopScores();
  void DrawTarget();
  FrameState SimulateMovement(std::vector<int> &action, FrameState& state, float dt);
  void MovePlayer(float dt);
  
  ////////////////////////////////////////

  entt::registry registry_gameplay_;

  std::vector<EntityID> player_ids_;
  EntityID my_id_, ball_id_;
  float current_stamina_ = 0.f;

  std::unordered_map<EntityID, std::pair<glm::vec3, glm::quat>> transforms_;
  std::unordered_map<EntityID, std::pair<glm::vec3, glm::quat>> new_transforms_;
  FrameState server_predicted_;
  entt::entity my_entity_;

  std::unordered_map<EntityID, std::pair<glm::vec3, bool>> physics_;

  entt::entity blue_goal_light_;
  entt::entity red_goal_light_;

  glob::Font2DHandle font_test_ = 0;
  glob::Font2DHandle font_scores_ = 0;
  glob::E2DHandle e2D_test_, e2D_test2_, e2D_target_;
  glob::GUIHandle in_game_menu_gui_ = 0;
  glob::GUIHandle gui_test_, gui_teamscore_, gui_stamina_base_,
    gui_stamina_fill_, gui_stamina_icon_, gui_quickslots_, gui_minimap_, gui_minimap_goal_red_, gui_minimap_goal_blue_, gui_minimap_player_red_, gui_minimap_player_blue_, gui_minimap_ball_;

  std::vector<glob::GUIHandle> ability_handles_;

  bool show_in_game_menu_buttons_ = false;

  int my_primary_ability_id = 0;
  int match_time_ = 300;
  int countdown_time_ = 5;

  Timer end_game_timer_;
  bool game_has_ended_ = false;
  bool goals_swapped_ = false;
  EntityID my_target_ = -1;

  glob::ModelHandle test_ball_;
  std::list<PlayerData> history_;
  FrameState predicted_state_;
  float latency_; //do we need?
  std::vector<int> actions_;
  int frame_id = 0;
  float accum_pitch_ = 0.0f;
  float accum_yaw_ = 0.0f;
};

#endif  // STATE_HPP_