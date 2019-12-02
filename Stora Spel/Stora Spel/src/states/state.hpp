#ifndef STATE_HPP_
#define STATE_HPP_

#include <NetAPI\packet.hpp>
#include <client replay machine/client_replay_machine.hpp>
#include <ecs/components/button_component.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <glob/graphics.hpp>
#include <list>
#include <playerdata.hpp>
#include <util/timer.hpp>
#include "Chat.hpp"
#include "ecs/components.hpp"
#include "eventdispatcher.hpp"
#include "shared/shared.hpp"
#include <glob/window.hpp>

class Engine;

enum class StateType {
  MAIN_MENU,
  CONNECT_MENU,
  CREATE_SERVER,
  LOBBY,
  PLAY,
  REPLAY,
  SETTINGS,
  NUM_OF_STATES
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
  glm::vec2 ws_;
  entt::registry registry_lobby_;
  void CreateBackgroundEntities();
  void CreateGUIElements();
  void DrawTeamSelect();
  void DrawAbilitySelect();
  glob::GUIHandle red_team_select_back_;
  glob::GUIHandle blue_team_select_back_;
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
  void CreateBackground();
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
  std::string last_msg_ = "Status: Failed to connect, Timeout";
  glob::GUIHandle bg_ = 0;
  glm::vec4 color_ = glm::vec4(1, 1, 1, 1);
  std::string ip_ = "localhost";
  ;
  std::string port_ = "1337";
  glob::Font2DHandle font_test_ = 0;
  entt::registry registry_connect_menu_;
  int prv_ = -1;
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

  glm::vec2 ws_;
  std::string setting_username_ = "fel";
  bool applied_ = false;
  std::chrono::time_point<std::chrono::high_resolution_clock> time_;
};

/////////////////////// PLAY ///////////////////////

class PlayState : public State {
  enum JumbotronEffect {
    TEAM_SCORES,
    MATCH_TIME,
    BEST_PLAYER,
    GOAL_SCORED,
    NUM_EFFECTS
  };

  struct Fishermans {
    EntityID hook_id;
    EntityID owner_id;
  };

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
  void SetEntityIDs(std::vector<EntityID> player_ids, EntityID my_id) {
    player_ids_ = player_ids;
    my_id_ = my_id;
  }
  void SetInitBallData(EntityID ball_id, bool is_real) {
    init_balls_[ball_id] = is_real;
  }

  void SetCurrentStamina(float stamina) { current_stamina_ = stamina; }
  auto* GetReg() { return &registry_gameplay_; }

  void CreateWall(EntityID id, glm::vec3 position, glm::quat rotation, unsigned int team = TEAM_NONE);
  void CreatePickUp(EntityID id, glm::vec3 position);
  void CreateCannonBall(EntityID id, glm::vec3 pos, glm::quat ori, unsigned int creator_team);
  void CreateTeleportProjectile(EntityID id, glm::vec3 pos, glm::quat ori);
  void CreateForcePushObject(EntityID id, glm::vec3 pos, glm::quat ori);
  void CreateMissileObject(EntityID id, glm::vec3 pos, glm::quat ori);
  void CreateFishermanAndHook(EntityID id, glm::vec3 pos, glm::quat ori, EntityID owner_id);
  void CreateBlackHoleObject(EntityID id, glm::vec3 pos, glm::quat ori);
  void CreateMineObject(unsigned int owner_team, EntityID mine_id,
                        glm::vec3 pos);
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
  void OverTime();
  void CreateGoalParticles(float x, entt::registry& registry);

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
  void SetTeam(unsigned int team) { my_team_ = team; }
  void CreateNewBallEntity(bool fake, EntityID id);
  void SetTeam(EntityID id, unsigned int team) { teams_[id] = team; }
  void SetCountdownInProgress(bool val) { countdown_in_progress_ = val; }
  void SetArenaScale(glm::vec3 arena_scale) { arena_scale_ = arena_scale; }

  void FetchMapAndArena(entt::registry& in_registry);
  void SetCanSmash(bool val) { can_smash_ = val; }
  void SetGoalsSwappedAtStart(bool val) { goals_swapped_at_start_ = val; }

  // Replay stuff
  bool IsRecording() const { return this->recording_; }
  // void SetRecording(bool in_val) { this->recording_ = in_val; }
  //

 private:
  struct GuiNotRespnding {
    glob::GUIHandle hndl = 0;
    glm::vec2 drawpos = glm::vec2(0.0f, 0.0f);
    double scale = 1.0;
    glob::Font2DHandle font = 0;
    uint64_t timeleft = 10000;
    GuiNotRespnding() {
      hndl = glob::GetGUIItem("Assets/GUI_elements/gray_bg.png");
      drawpos = glm::vec2(glob::window::GetWindowDimensions().x -
                              glob::window::GetWindowDimensions().x * 0.35,
                          glob::window::GetWindowDimensions().y -
                              glob::window::GetWindowDimensions().y * 0.2);
      font = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
    }
    void Draw(uint64_t time) {
      int time_int = std::ceil((10000 - time) / 1000);
      glob::Submit(hndl, drawpos, scale);
      glob::Submit(font, drawpos + glm::vec2(50.0f, 70.0f), 29,
                   "Connection problems, disconnecting in " +
                       std::to_string(time_int) + std::string("s"),
                   glm::vec4(0.0, 0.0, 0.0, 1.0));
    }
  } server_not_responding_;
  ServerStateType server_state_;
  void CreateInitialEntities();
  void CreatePlayerEntities();
  void CreateArenaEntity();
  void CreateAudienceEntities();
  void CreateMapEntity();
  void CreateBallEntities();
  void CreateSpotlights();
  void CreateJumbotron();
  void ParticleComponentDestroyed(entt::entity e, entt::registry& registry);
  void SoundComponentDestroyed(entt::entity e, entt::registry& registry);
  void CreateInGameMenu();
  void AddPlayer();
  void TestCreateLights();

  void ToggleInGameMenu();
  void UpdateInGameMenu(bool show_menu);
  void UpdateGameplayTimer();
  void UpdateSwitchGoalTimer();

  void DrawNameOverPlayer();
  void DrawWallOutline();
  void DrawFishingLines();

  void DrawTopScores();
  void DrawTarget();
  void DrawQuickslots();
  void DrawStunTimer();

  void DrawMiniMap();
  void DrawJumbotronText();

  FrameState SimulateMovement(std::vector<int>& action, FrameState& state,
                              float dt);
  void MovePlayer(float dt);
  void MoveBall(float dt);
  void BlackHoleMovement(float dt);
  void Collision();
  void UpdateGravity();
  unsigned long GetBestPlayer();

  EntityID ClientIDToEntityID(long client_id);
  ////////////////////////////////////////

  entt::registry registry_gameplay_;

  std::vector<EntityID> player_ids_;
  EntityID my_id_;

  std::unordered_map<EntityID, bool> init_balls_;
  float current_stamina_ = 0.f;

  std::unordered_map<EntityID, std::pair<glm::vec3, glm::quat>> transforms_;
  std::unordered_map<EntityID, std::pair<glm::vec3, glm::quat>> new_transforms_;
  std::unordered_map<EntityID, glm::vec3> player_look_dirs_;
  std::unordered_map<EntityID, glm::vec3> player_move_dirs_;
  FrameState server_predicted_;
  entt::entity my_entity_, arena_entity_, map_visual_entity_;

  std::vector<entt::entity> Audiences;

  std::unordered_map<EntityID, std::pair<glm::vec3, bool>> physics_;

  std::unordered_map<EntityID, unsigned int> teams_;

  entt::entity blue_goal_light_;
  entt::entity red_goal_light_;

  glob::Font2DHandle font_test_ = 0;
  glob::Font2DHandle font_scores_ = 0;
  glob::E2DHandle e2D_test_, e2D_test2_, e2D_target_, e2D_outline_;
  glob::GUIHandle in_game_menu_gui_ = 0;
  glob::GUIHandle gui_test_, gui_teamscore_, gui_stamina_base_,
      gui_stamina_fill_, gui_stamina_icon_, gui_quickslots_, gui_minimap_,
      gui_minimap_goal_red_, gui_minimap_goal_blue_, gui_minimap_player_me_,
      gui_minimap_player_red_, gui_minimap_player_blue_, gui_minimap_ball_,
      gui_crosshair_;

  std::vector<glob::GUIHandle> ability_handles_;

  bool show_in_game_menu_buttons_ = false;

  int my_primary_ability_id = 0;
  unsigned int my_team_;
  int match_time_ = 300;
  int countdown_time_ = 5;
  // For switch goal
  bool countdown_in_progress_ = false;

  bool game_has_ended_ = false;
  std::chrono::time_point<std::chrono::system_clock> overtime_start_time_, overtime_end_time_;
  bool overtime_has_started_ = false;
  bool overtime_check_time = true;
  bool goals_swapped_ = false;
  bool goals_swapped_at_start_ = false;
  EntityID my_target_ = -1;

  glob::ModelHandle test_ball_;
  std::list<PlayerData> history_;
  FrameState predicted_state_;
  glm::vec3 arena_scale_;
  std::vector<int> actions_;
  int frame_id = 0;
  float pitch_ = 0.0f;
  float yaw_ = 0.0f;

  float timer_ = 0.0f;
  float primary_cd_ = 0.0f;

  bool sprinting_ = false;

  int current_jumbo_effect_ = TEAM_SCORES;
  Timer jumbo_effect_timer_;
  float jumbo_effect_time_ = 5.0f;

  bool can_smash_ = false;

  bool im_stunned_ = false;
  Timer stun_timer_;
  float my_stun_time_;

  bool switching_goals_ = false;

  std::vector<Fishermans> fishers_;

  // Replay stuff
  bool recording_ = false;
  // Replay stuff
};

class ReplayState : public State {
 private:
  // Registry
  entt::registry replay_registry_;

  // Replay variables
  bool replaying_ = false;
  unsigned int num_of_replays_ = 0;
  unsigned int replay_counter_ = 0;

  // End game time handling
  unsigned int replay_state_duration_ = 0;
  Timer replay_state_timer_;

  // Menu stuff
  glob::GUIHandle in_game_menu_gui_ = 0;
  bool show_in_game_menu_buttons_ = false;

  // More variables
  glob::Font2DHandle font_test_ = 0;
  glm::vec3 arena_scale_ = glm::vec3(0.f);
  bool goals_swapped_ = false;

  // Functions
  void AddConstantStuff();
  void AddArenaStuff();
  void AddBatmanLights();
  void AddLights();
  void AddSpotlights();
  void AddCamera(glm::vec3 in_cam_pos);

  void UpdateCamera();
  void UpdatePickUpMovement(/*float dt*/);

  void StartReplayMode();
  void PlayReplay();

  void ToggleInGameMenu();
  void UpdateInGameMenu(bool show_menu);
  void CreateInGameMenu();

  void SoundComponentDestroyed(entt::entity e, entt::registry& registry);

  void ShowScoreboard();

 public:
  void Startup() override;
  void Init() override;
  void Update(float dt) override;
  void UpdateNetwork() override;
  void Cleanup() override;

  StateType Type() { return StateType::REPLAY; }
  void SetArenaScale(glm::vec3 in_scale) { this->arena_scale_ = in_scale; }
};

class CreateServerState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update(float dt) override;
  void UpdateNetwork() override;
  void Cleanup() override;
  ~CreateServerState();
  StateType Type() { return StateType::CREATE_SERVER; }
  bool started_ = false;

 private:
  glob::Font2DHandle font_test_ = 0;
  entt::registry registry_create_server_;
  glob::GUIHandle bg_ = 0;
  std::string ip_ = "";
  std::string port_ = "1337";
  std::string max_players_ = "6";
  void CreateServer();
};
#endif  // STATE_HPP_