#include "state.hpp"

#include <glob/window.hpp>
#include "engine.hpp"
#include "entitycreation.hpp"

void MainMenuState::Startup() {
  CreateMainMenu();
  CreateSettingsMenu();
}

void MainMenuState::Init() {
  auto& client = engine_->GetClient();
  //
  engine_->SetSendInput(false);
  engine_->SetCurrentRegistry(&registry_mainmenu_);
  engine_->SetEnableChat(false);

  if (client.IsConnected()) {
    client.Disconnect();
  }
}

void MainMenuState::Update() {
  //
}

void MainMenuState::UpdateNetwork() {
  //
}

void MainMenuState::Cleanup() {
  //
}

void MainMenuState::CreateMainMenu() {
  glob::window::SetMouseLocked(false);
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  // PLAY BUTTON - change registry to registry_gameplay_
  ButtonComponent* b_c = GenerateButtonEntity(registry_mainmenu_, "PLAY",
                                              glm::vec2(100, 200), font_test_);
  b_c->button_func = [&]() { engine_->ChangeState(StateType::CONNECT_MENU); };

  // SETTINGS BUTTON - change registry to registry_settings_
  b_c = GenerateButtonEntity(registry_mainmenu_, "SETTINGS",
                             glm::vec2(100, 140), font_test_);
  b_c->button_func = [&]() {
    engine_->SetCurrentRegistry(&registry_settings_);
  };

  // EXIT BUTTON - close the game
  b_c = GenerateButtonEntity(registry_mainmenu_, "EXIT", glm::vec2(100, 80),
                             font_test_);
  b_c->button_func = [&]() { exit(0); };
}

void MainMenuState::CreateSettingsMenu() {
  // BACK BUTTON in SETTINGS - go back to main menu
  ButtonComponent* b_c = GenerateButtonEntity(registry_settings_, "BACK",
                                              glm::vec2(100, 200), font_test_);
  b_c->button_func = [&]() {
    engine_->SetCurrentRegistry(&registry_mainmenu_);
  };
}
