#ifndef STATE_HPP_
#define STATE_HPP_

#include <entt.hpp>

class Engine;

enum class StateType {
  MAIN_MENU,
  LOBBY,
  PLAY,
};

class State {
 public:
  virtual void Startup() = 0;
  virtual void Init() = 0;
  virtual void Update() = 0;
  virtual void Cleanup() = 0;
  virtual StateType Type() = 0;

  void SetEngine(Engine* engine) { engine_ = engine; }
  Engine* GetEngine() { return engine_; }

  State() = default;
  ~State() {}

 private:
  Engine* engine_ = nullptr;
};

class MainMenuState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update() override;
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

class LobbyState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update() override;
  void Cleanup() override;

  StateType Type() { return StateType::LOBBY; }

 private:
  entt::registry registry_lobby_;
};

class PlayState : public State {
 public:
  void Startup() override;
  void Init() override;
  void Update() override;
  void Cleanup() override;

  StateType Type() { return StateType::PLAY; }

 private:
  void CreateInGameMenu();

  entt::registry registry_gameplay_;

   glob::Font2DHandle font_test_ = 0;
};

#endif  // STATE_HPP_