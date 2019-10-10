#include "state.hpp"

#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include <GLFW/glfw3.h>
#include "shared/camera_component.hpp"
#include "shared/id_component.hpp"
#include "shared/transform_component.hpp"

#include <shared\pick_up_component.hpp>
#include "ecs/components.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"
#include "util/input.hpp"
#include "util/global_settings.hpp"

void PlayState::Startup() {
  ///////////////////////////////////////////////////////////////
  // TO BE MOVED
  ///////////////////////////////////////////////////////////////
  // Create in-game menu background
  in_game_menu_gui_ =
      glob::GetGUIItem("Assets/GUI_elements/ingame_menu_V1.png");
  // Create 2D element
  e2D_test_ = glob::GetE2DItem("assets/GUI_elements/point_table.png");
  e2D_test2_ = glob::GetE2DItem("assets/GUI_elements/Scoreboard_V1.png");

  // Create GUI elementds
  gui_test_ = glob::GetGUIItem("assets/GUI_elements/Scoreboard_V1.png");
  gui_teamscore_ = glob::GetGUIItem("assets/GUI_elements/point_table.png");
  gui_stamina_base_ =
      glob::GetGUIItem("assets/GUI_elements/stamina_bar_base.png");
  gui_stamina_fill_ =
      glob::GetGUIItem("assets/GUI_elements/stamina_bar_fill.png");
  gui_stamina_icon_ =
      glob::GetGUIItem("assets/GUI_elements/stamina_bar_icon.png");
  gui_quickslots_ =
      glob::GetGUIItem("assets/GUI_elements/koncept_abilities.png");
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  ///////////////////////////////////////////////////////////////
  // \TO BE MOVED
  ///////////////////////////////////////////////////////////////
}

void PlayState::Init() {
  glob::window::SetMouseLocked(true);
  engine_->SetSendInput(true);
  engine_->SetCurrentRegistry(&registry_gameplay_);
  engine_->SetEnableChat(true);

  CreateInGameMenu();
  CreateInitialEntities();

  auto& client = engine_->GetClient();
  NetAPI::Common::Packet to_send;
  bool rec = true;
  to_send << rec;
  to_send << PacketBlockType::CLIENT_RECEIVE_UPDATES;
  client.Send(to_send);
}

void PlayState::Update() {
  auto& cli = engine_->GetClient();
  if (!cli.IsConnected()) {
    cli.Disconnect();
    engine_->ChangeState(StateType::MAIN_MENU);
  }
  if (!transforms_.empty()) {
    auto view_entities =
        registry_gameplay_.view<TransformComponent, IDComponent>();
    for (auto entity : view_entities) {
      auto& trans_c = view_entities.get<TransformComponent>(entity);
      auto& id_c = view_entities.get<IDComponent>(entity);
      auto trans = transforms_[id_c.id];
      trans_c.position = trans.first;
      trans_c.rotation = trans.second;
      /*
      std::cout << trans_c.position.x << ", ";
      std::cout << trans_c.position.y << ", ";
      std::cout << trans_c.position.z << "\n";
      */
    }
    // std::cout << "\n";
    transforms_.clear();
  }

  if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
    ToggleInGameMenu();
  }
  if (show_in_game_menu_buttons_) {
    glob::Submit(in_game_menu_gui_, glm::vec2(491, 152), 1.0f);
  }
  // Submit 2D Element TEST
  glob::Submit(e2D_test_, glm::vec3(10.5f, 1.0f, 0.0f), 2, -90.0f,
               glm::vec3(0, 1, 0));
  glob::Submit(e2D_test_, glm::vec3(-10.5f, 1.0f, 0.0f), 2, 90.0f,
               glm::vec3(0, 1, 0));
  glob::Submit(e2D_test2_, glm::vec3(0.0f, 1.0f, -7.0f), 7, 0.0f, glm::vec3(1));

  UpdateGameplayTimer();
  /*
  registry_gameplay_.view<PlayerComponent>().each(
      [&](auto entity, PlayerComponent& player_c) {
        stam_len = player_c.energy_current;
      });
  */
  glob::Submit(gui_stamina_base_, glm::vec2(0, 5), 0.85, 100);
  glob::Submit(gui_stamina_fill_, glm::vec2(7, 12), 0.85, current_stamina_);
  glob::Submit(gui_stamina_icon_, glm::vec2(0, 5), 0.85, 100);
  glob::Submit(gui_quickslots_, glm::vec2(7, 50), 0.3, 100);
  glob::Submit(gui_teamscore_, glm::vec2(497, 648), 1, 100);
}

void PlayState::UpdateNetwork() {
  auto& packet = engine_->GetPacket();

  // TEMP: Start recording replay
  bool temp = Input::IsKeyPressed(GLFW_KEY_P);
  packet << temp;
  packet << PacketBlockType::TEST_REPLAY_KEYS;
}

void PlayState::Cleanup() {
  //
}

void PlayState::ToggleInGameMenu() {
  show_in_game_menu_buttons_ = !show_in_game_menu_buttons_;
  glob::window::SetMouseLocked(!show_in_game_menu_buttons_);
  UpdateInGameMenu(show_in_game_menu_buttons_);
}

void PlayState::UpdateInGameMenu(bool show_menu) {
  // Set in_game buttons visibility
  auto view = registry_gameplay_.view<ButtonComponent, TransformComponent>();
  for (auto v : view) {
    auto& button_c = registry_gameplay_.get<ButtonComponent>(v);
    button_c.visible = show_menu;
  }
}

void PlayState::UpdateGameplayTimer() {
  // Gameplay timer
  int temp = (int)GlobalSettings::Access()->ValueOf("MATCH_TIME") - engine_->GetGameplayTimer();
  int sec = 0;
  int min = 5;

  min = temp / 60;
  sec = temp % 60;

  // Countdown timer
  int count = (int)GlobalSettings::Access()->ValueOf("COUNTDOWN_TIME") - engine_->GetCountdownTimer();

  // --------------------------------------
  glob::Submit(font_test_, glm::vec2(645, 705), 40, std::to_string(min),
               glm::vec4(1));
  glob::Submit(font_test_, glm::vec2(638, 695), 40, "----",
               glm::vec4(1, 1, 1, 1));
  glob::Submit(font_test_, glm::vec2(638, 675), 40, std::to_string(sec),
               glm::vec4(1));
  if (count > 0) {
    glob::Submit(font_test_, glm::vec2(735, 450), 500, std::to_string(count),
                 glm::vec4(1));  // Visible = true
  } else {
    glob::Submit(font_test_, glm::vec2(735, 450), 500, std::to_string(count),
                 glm::vec4(1), false);
  }
}

void PlayState::SetEntityTransform(EntityID player_id, glm::vec3 pos,
                                   glm::quat orientation) {
  transforms_[player_id] = std::make_pair(pos, orientation);
}

void PlayState::SetCameraOrientation(glm::quat orientation) {
  registry_gameplay_.view<CameraComponent>().each(
      [&](auto entity, CameraComponent& cam_c) {
        cam_c.orientation = orientation;
      });
}

void PlayState::CreateInitialEntities() {
  CreatePlayerEntities();
  CreateArenaEntity();
  CreateBallEntity();
  TestCreateLights();
}

void PlayState::CreatePlayerEntities() {
  std::cout << "DEBUG: playstate.cpp: Created " << player_ids_.size()
            << " players\n";

  for (auto entity_id : player_ids_) {
    auto entity = registry_gameplay_.create();

    glm::vec3 alter_scale =
        glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f);
    glm::vec3 character_scale = glm::vec3(0.1f);

    glob::ModelHandle player_model =
        glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

    registry_gameplay_.assign<IDComponent>(entity, entity_id);
    registry_gameplay_.assign<PlayerComponent>(entity);
    registry_gameplay_.assign<TransformComponent>(entity, glm::vec3(),
                                                  glm::quat(), character_scale);
    registry_gameplay_.assign<ModelComponent>(entity, player_model,
                                              alter_scale * character_scale);

    if (entity_id == my_id_) {
      glm::vec3 camera_offset = glm::vec3(0.38f, 0.62f, -0.06f);
      registry_gameplay_.assign<CameraComponent>(entity, camera_offset);
    }
  }
}

void PlayState::CreateArenaEntity() {
  auto arena = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  glob::ModelHandle model_arena =
      glob::GetModel("assets/Map_rectangular/map_rextangular.fbx");
  registry_gameplay_.assign<ModelComponent>(arena, model_arena);
  registry_gameplay_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                arena_scale);
}

void PlayState::CreateBallEntity() {
  // Ball
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  auto ball = registry_gameplay_.create();
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  registry_gameplay_.assign<ModelComponent>(ball, model_ball);
  registry_gameplay_.assign<TransformComponent>(ball, zero_vec, zero_vec,
                                                glm::vec3(1.0f));
  registry_gameplay_.assign<BallComponent>(ball);
  registry_gameplay_.assign<IDComponent>(ball, ball_id_);
}

void PlayState::CreateInGameMenu() {
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  // CONTINUE BUTTON -- change registry to registry_gameplay_
  ButtonComponent* in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "CONTINUE", glm::vec2(550, 430), font_test_, false);
  in_game_buttons_->button_func = [&]() { ToggleInGameMenu(); };
  // SETTINGS BUTTON -- change registry to registry_settings_
  in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "SETTINGS", glm::vec2(550, 360), font_test_, false);

  in_game_buttons_->button_func = [&] {
    // All the logic here
  };

  // END GAME -- change registry to registry_mainmenu_
  in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "MAINMENU", glm::vec2(550, 290), font_test_, false);
  in_game_buttons_->button_func = [&] {
    engine_->ChangeState(StateType::MAIN_MENU);
  };

  in_game_buttons_ = GenerateButtonEntity(
      registry_gameplay_, "EXIT", glm::vec2(550, 220), font_test_, false);
  in_game_buttons_->button_func = [&] { exit(0); };
}

void PlayState::TestCreateLights() {
  // Create lights
  blue_goal_light_ = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(
      blue_goal_light_, glm::vec3(0.1f, 0.1f, 1.0f), 15.f, 0.0f);
  registry_gameplay_.assign<TransformComponent>(
      blue_goal_light_, glm::vec3(12.f, -4.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  red_goal_light_ = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(
      red_goal_light_, glm::vec3(1.f, 0.1f, 0.1f), 15.f, 0.f);
  registry_gameplay_.assign<TransformComponent>(
      red_goal_light_, glm::vec3(-12.f, -4.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  auto light = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(light, glm::vec3(0.4f, 0.4f, 0.4f),
                                            30.f, 0.1f);
  registry_gameplay_.assign<TransformComponent>(
      light, glm::vec3(0, 4.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f));
}

void PlayState::CreatePickUp(glm::vec3 position) {
  auto pick_up = registry_gameplay_.create();
  glob::ModelHandle model_pick_up =
      glob::GetModel("assets/lowpolydeer/deer.fbx");  // Replace with real model
  registry_gameplay_.assign<ModelComponent>(pick_up, model_pick_up);
  registry_gameplay_.assign<TransformComponent>(
      pick_up, position, glm::vec3(0.0f, 0.0f, -1.6f), glm::vec3(0.002f));
  registry_gameplay_.assign<PickUpComponent>(pick_up);
}

void PlayState::CreateCannonBall(EntityID id) {
  auto cannonball = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  registry_gameplay_.assign<ModelComponent>(cannonball, model_ball);
  registry_gameplay_.assign<TransformComponent>(cannonball, zero_vec, zero_vec,
                                                glm::vec3(0.3f));
  registry_gameplay_.assign<IDComponent>(cannonball, id);
}

void PlayState::CreateForcePushObject(EntityID id) {
  auto force_object = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  registry_gameplay_.assign<ModelComponent>(force_object, model_ball);
  registry_gameplay_.assign<TransformComponent>(force_object, zero_vec, zero_vec,
                                                glm::vec3(0.5f));
  registry_gameplay_.assign<IDComponent>(force_object, id);
}

void PlayState::DestroyEntity(EntityID id) {
  auto id_view = registry_gameplay_.view<IDComponent>();
  for (auto entity : id_view) {
    auto& e_id = id_view.get(entity);

	if (e_id.id == id) {
      registry_gameplay_.destroy(entity);
      return;
	}
  }
}

void PlayState::SwitchGoals() {
  TransformComponent& blue_light_trans_c =
      registry_gameplay_.get<TransformComponent>(blue_goal_light_);
  TransformComponent& red_light_trans_c =
      registry_gameplay_.get<TransformComponent>(red_goal_light_);

  glm::vec3 blue_light_pos = blue_light_trans_c.position;
  blue_light_trans_c.position = red_light_trans_c.position;
  red_light_trans_c.position = blue_light_pos;
}
