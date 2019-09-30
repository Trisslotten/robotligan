#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <iostream>

#include <button_system.hpp>
#include <camera_component.hpp>
#include <collision_system.hpp>
#include <physics_system.hpp>
#include <render_system.hpp>
#include "Components/button_component.hpp"
#include "Components/light_component.hpp"
#include "Components/transform_component.hpp"
#include "ability_controller_system.hpp"
#include "entitycreation.hpp"
#include "player_controller_system.hpp"
#include "util/global_settings.hpp"
#include "util/input.hpp"

Engine::Engine() {}

Engine::~Engine() {}

void Engine::Init() {
  glob::Init();
  Input::Initialize();

  // tcp_client_.Connect("192.168.1.47", 1337);

  // Tell the GlobalSettings class to do a first read from the settings file
  GlobalSettings::Access()->UpdateValuesFromFile();

  glm::vec3 start_positions[3] = {
      glm::vec3(5.f, 0.f, 0.f),   // Ball
      glm::vec3(-9.f, 4.f, 0.f),  // Player
      glm::vec3(0.f, 0.f, 0.f)    // Others
  };
  CreateEntities(registry_gameplay_, start_positions, 3);
  CreateMainMenu();
  CreateSettingsMenu();
  CreateInGameMenu();
  registry_current_ = &registry_mainmenu_;

  // Create light
  auto light = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(light, glm::vec3(0.3f, 0.3f, 1.0f),
                                            15.f, 0.2f);
  registry_gameplay_.assign<TransformComponent>(
      light, glm::vec3(12.f, -4.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  light = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(light, glm::vec3(1.f, 0.3f, 0.3f),
                                            15.f, 0.f);
  registry_gameplay_.assign<TransformComponent>(
      light, glm::vec3(-12.f, -4.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  // Create in-game menu background
  in_game_menu_gui_ =
      glob::GetGUIItem("Assets/GUI_elements/ingame_menu_V1.png");

  keybinds_[GLFW_KEY_W] = PlayerAction::WALK_FORWARD;
  keybinds_[GLFW_KEY_S] = PlayerAction::WALK_BACKWARD;
  keybinds_[GLFW_KEY_A] = PlayerAction::WALK_LEFT;
  keybinds_[GLFW_KEY_D] = PlayerAction::WALK_RIGHT;
  keybinds_[GLFW_KEY_LEFT_SHIFT] = PlayerAction::SPRINT;
  keybinds_[GLFW_KEY_SPACE] = PlayerAction::JUMP;
  keybinds_[GLFW_KEY_Q] = PlayerAction::ABILITY_PRIMARY;
  keybinds_[GLFW_KEY_E] = PlayerAction::ABILITY_SECONDARY;
  mousebinds_[GLFW_MOUSE_BUTTON_1] = PlayerAction::KICK;
  mousebinds_[GLFW_MOUSE_BUTTON_2] = PlayerAction::SHOOT;
}

void Engine::Update(float dt) {
  Input::Reset();

  UpdateSystems(dt);
}

void Engine::UpdateNetwork() {
  std::bitset<PlayerAction::NUM_ACTIONS> actions;

  for (auto const& [key, action] : keybinds_)
    if (Input::IsKeyDown(key)) actions.set(action, true);

  for (auto const& [button, action] : mousebinds_)
    if (Input::IsMouseButtonDown(button)) actions.set(action, true);

  uint16_t action_bits = actions.to_ulong();

  NetAPI::Common::Packet packet;
  packet << action_bits;

  tcp_client_.Send(packet);
}

void Engine::Render() {
  // glob::Submit(font_test_, glm::vec2(100, 200), 73,
  //         "Det här är Comic Sans MS jahoo!", glm::vec4(0, 0, 0, 0.5));
  // glob::Submit(font_test_, glm::vec2(98, 202), 73,
  //           "Det här är Comic Sans MS jahoo!", glm::vec4(1, 1, 1, 1));
  if (show_in_game_menu_buttons_) {
    glob::Submit(in_game_menu_gui_, glm::vec2(491, 152), 1.0f);
  }
  glob::Render();
}

void Engine::SetCurrentRegistry(entt::registry* registry) {
  this->registry_current_ = registry;
}

void Engine::UpdateSystems(float dt) {
  // collision_debug::Update(*reg);

  // Test
  if (Input::IsKeyPressed(GLFW_KEY_ESCAPE) && !show_in_game_menu_buttons_) {
    glob::window::SetMouseLocked(false);
    show_in_game_menu_buttons_ = true;
    UpdateInGameMenu(show_in_game_menu_buttons_);
  } else if (Input::IsKeyPressed(GLFW_KEY_ESCAPE) &&
             show_in_game_menu_buttons_) {
    glob::window::SetMouseLocked(true);
    show_in_game_menu_buttons_ = false;
    UpdateInGameMenu(show_in_game_menu_buttons_);
  }

  player_controller::Update(*registry_current_, dt);
  ability_controller::Update(*registry_current_, dt);

  UpdatePhysics(*registry_current_, dt);
  UpdateCollisions(*registry_current_);

  /*auto view = registry_gameplay_.view<CameraComponent, TransformComponent>();
  for (auto v : view) {
    auto& cam_c = registry_gameplay_.get<CameraComponent>(v);
    auto& trans_c = registry_gameplay_.get<TransformComponent>(v);
    cam_c.cam->SetPosition(trans_c.position +
                           glm::rotate(cam_c.offset, -trans_c.rotation.y,
                                       glm::vec3(0.0f, 1.0f, 0.0f)));
  }*/

  button_system::Update(*registry_current_);
  RenderSystem(*registry_current_);
}

void Engine::CreateMainMenu() {
  glob::window::SetMouseLocked(false);
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  // PLAY BUTTON - change registry to registry_gameplay_
  ButtonComponent* b_c = GenerateButtonEntity(registry_mainmenu_, "PLAY",
                                              glm::vec2(100, 200), font_test_);
  b_c->button_func = [&]() {
    this->SetCurrentRegistry(&registry_gameplay_);
    glob::window::SetMouseLocked(true);
  };

  // SETTINGS BUTTON - change registry to registry_settings_
  b_c = GenerateButtonEntity(registry_mainmenu_, "SETTINGS",
                             glm::vec2(100, 140), font_test_);
  b_c->button_func = [&]() { this->SetCurrentRegistry(&registry_settings_); };

  // EXIT BUTTON - close the game
  b_c = GenerateButtonEntity(registry_mainmenu_, "EXIT", glm::vec2(100, 80),
                             font_test_);
  b_c->button_func = [&]() { exit(0); };
}

void Engine::CreateSettingsMenu() {
  // BACK BUTTON in SETTINGS - go back to main menu
  ButtonComponent* b_c = GenerateButtonEntity(registry_settings_, "BACK",
                                              glm::vec2(100, 200), font_test_);
  b_c->button_func = [&]() { this->SetCurrentRegistry(&registry_mainmenu_); };
}

void Engine::CreateInGameMenu() {
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

void Engine::UpdateInGameMenu(bool show_menu) {
  // Set in_game buttons visibility
  auto view = registry_gameplay_.view<ButtonComponent, TransformComponent>();
  for (auto v : view) {
    auto& button_c = registry_gameplay_.get<ButtonComponent>(v);
    button_c.visible = show_menu;
  }
}
