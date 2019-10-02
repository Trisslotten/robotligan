#include "state.hpp"

#include <glob/window.hpp>
#include "engine.hpp"
#include "entitycreation.hpp"

//////////////////////////////////////////////////////////////////////////////
////////////// Main Menu /////////////////////////////////////////////////////

void MainMenuState::Startup() {
  CreateMainMenu();
  CreateSettingsMenu();
}

void MainMenuState::Init() {
  //

  GetEngine()->SetCurrentRegistry(&registry_mainmenu_);
}

void MainMenuState::Update() {
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
  b_c->button_func = [&]() { GetEngine()->ChangeState(StateType::LOBBY); };

  // SETTINGS BUTTON - change registry to registry_settings_
  b_c = GenerateButtonEntity(registry_mainmenu_, "SETTINGS",
                             glm::vec2(100, 140), font_test_);
  b_c->button_func = [&]() {
    GetEngine()->SetCurrentRegistry(&registry_settings_);
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
    GetEngine()->SetCurrentRegistry(&registry_mainmenu_);
  };
}

//////////////////////////////////////////////////////////////////////////////
//////////////// Lobby ///////////////////////////////////////////////////////
void LobbyState::Startup() {}

void LobbyState::Init() {
  //
}

void LobbyState::Update() {
  //
}

void LobbyState::Cleanup() {
  //
}

//////////////////////////////////////////////////////////////////////////////
////////////////  Play ///////////////////////////////////////////////////////
void PlayState::Startup() {
  //
}

void PlayState::Init() {
  //
}

void PlayState::Update() {
  //
}

void PlayState::Cleanup() {
  //
}

void PlayState::CreateInGameMenu() {

  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");


// CONTINUE BUTTON -- change registry to registry_gameplay_
  ButtonComponent* in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "CONTINUE", glm::vec2(550, 430), font_test_, false);
  in_game_buttons_->button_func = [&]() {
    // All the logic here
  };
  // SETTINGS BUTTON -- change registry to registry_settings_
  in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "SETTINGS", glm::vec2(550, 360), font_test_, false);

  in_game_buttons_->button_func = [&] {
    // All the logic here
  };

  // END GAME -- change registry to registry_mainmenu_
  in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "TO LOBBY", glm::vec2(550, 290), font_test_, false);
  in_game_buttons_->button_func = [&] {
    // All the logic here
  };

  in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "EXIT", glm::vec2(550, 220), font_test_, false);
  in_game_buttons_->button_func = [&] { exit(0); };
}