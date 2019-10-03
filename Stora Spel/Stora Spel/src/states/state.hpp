#ifndef STATE_HPP_
#define STATE_HPP_

#include <entt.hpp>
#include <glm/glm.hpp>
#include <glob/graphics.hpp>
#include "shared/shared.hpp"

class Engine;

enum class StateType {
  MAIN_MENU,
  LOBBY,
  PLAY,
};

class State {
 public:
  // when program starts
  virtual void Startup() = 0;

  // when state changed to this state
  virtual void Init() = 0;

  virtual void Update() = 0;

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
  void Update() override;
  void UpdateNetwork() override;
  void Cleanup() override;

  StateType Type() { return StateType::MAIN_MENU; }

 private:
  void CreateMainMenu();
  void CreateSettingsMenu();

  entt::registry registry_mainmenu_;
  entt::registry registry_settings_;

  glob::Font2DHandle font_test_ = 0;

  // Inherited via State
};

/////////////////////// LOBBY ///////////////////////

class LobbyState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update() override;
  void UpdateNetwork() override;
  void Cleanup() override;

  StateType Type() { return StateType::LOBBY; }

 private:
  entt::registry registry_lobby_;
};

/////////////////////// PLAY ///////////////////////

class PlayState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update() override;
  void UpdateNetwork() override;
  void Cleanup() override;

  StateType Type() { return StateType::PLAY; }

  void SetEntityTransform(EntityID player_id, glm::vec3 pos,
                          glm::quat orientation);
  void SetCameraOrientation(glm::quat orientation);

  void SetEntityIDs(std::vector<EntityID> player_ids, EntityID my_id,
                    EntityID ball_id) {
    player_ids_ = player_ids;
    my_id_ = my_id;
    ball_id_ = ball_id;
  }

 private:
  void CreateInitialEntities();
  void CreatePlayerEntities();
  void CreateArenaEntity();
  void CreateBallEntity();
  void CreateInGameMenu();

  void TestCreateLights();

  void UpdateInGameMenu(bool show_menu);

  ////////////////////////////////////////

  entt::registry registry_gameplay_;

  std::vector<EntityID> player_ids_;
  EntityID my_id_, ball_id_;

  std::unordered_map<EntityID, std::pair<glm::vec3, glm::quat>> transforms_;

  glob::Font2DHandle font_test_ = 0;
  glob::E2DHandle e2D_test_, e2D_test2_;
  glob::GUIHandle in_game_menu_gui_ = 0;
  glob::GUIHandle gui_test_, gui_teamscore_, gui_stamina_base_,
      gui_stamina_fill_, gui_stamina_icon_, gui_quickslots_;

  bool show_in_game_menu_buttons_ = false;
};

#endif  // STATE_HPP_