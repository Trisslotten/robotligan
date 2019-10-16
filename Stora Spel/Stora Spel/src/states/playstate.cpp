#include "state.hpp"

#include <GLFW/glfw3.h>
#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include <slob/sound_engine.hpp>

#include "shared/camera_component.hpp"
#include "shared/id_component.hpp"
#include "shared/transform_component.hpp"

#include <shared/physics_component.hpp>
#include <shared/pick_up_component.hpp>
#include "ecs/components.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"
#include "util/global_settings.hpp"
#include "util/input.hpp"

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
  e2D_target_ = glob::GetE2DItem("assets/GUI_elements/target.png");

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
    glob::GetGUIItem("assets/GUI_elements/quickslots_blank.png");
  gui_minimap_ = glob::GetGUIItem("assets/GUI_elements/Minimap_V2.png");
  gui_minimap_goal_red_ = glob::GetGUIItem("assets/GUI_elements/goal_red_icon.png");
  gui_minimap_goal_blue_ = glob::GetGUIItem("assets/GUI_elements/goal_blue_icon.png");
  gui_minimap_player_red_ = glob::GetGUIItem("assets/GUI_elements/player_iconv2_red.png");
  gui_minimap_player_blue_ = glob::GetGUIItem("assets/GUI_elements/player_iconv2_blue.png");
  gui_minimap_ball_ = glob::GetGUIItem("assets/GUI_elements/Ball_Icon.png");

  int num_abilities = (int)AbilityID::NUM_OF_ABILITY_IDS;
  ability_handles_.resize(num_abilities);
  for (int i = 0; i < num_abilities; i++) {
    ability_handles_[i] = glob::GetGUIItem(
      "assets/GUI_Elements/ability_icons/" + std::to_string(i) + ".png");
  }
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  font_scores_ = glob::GetFont("assets/fonts/fonts/OCRAEXT_2.TTF");
  ///////////////////////////////////////////////////////////////
  // \TO BE MOVED
  ///////////////////////////////////////////////////////////////
}

void PlayState::TestParticles() {
  auto e = registry_gameplay_.create();
  auto handle = glob::CreateParticleSystem();

  std::vector handles = {handle};
  std::vector offsets = {glm::vec3(0.f)};
  std::vector directions = {glm::vec3(0.f, 1.f, 0.f)};

  glob::SetParticleSettings(handle, "green_donut.txt");

  registry_gameplay_.assign<ParticleComponent>(e, handles, offsets, directions);
}

void PlayState::Init() {
  glob::window::SetMouseLocked(true);
  engine_->SetSendInput(true);
  engine_->SetCurrentRegistry(&registry_gameplay_);
  engine_->SetEnableChat(true);

  CreateInGameMenu();
  CreateInitialEntities();
  TestParticles();

  engine_->GetChat()->SetPosition(
    glm::vec2(30, glob::window::GetWindowDimensions().y - 30));

  auto& client = engine_->GetClient();
  NetAPI::Common::Packet to_send;
  bool rec = true;
  to_send << rec;
  to_send << PacketBlockType::CLIENT_RECEIVE_UPDATES;
  client.Send(to_send);

  engine_->GetSoundSystem().PlayAmbientSound(registry_gameplay_);
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
  if (!physics_.empty()) {
    auto view_entities =
        registry_gameplay_.view<PhysicsComponent, IDComponent>();
    for (auto entity : view_entities) {
      auto& physics_c = view_entities.get<PhysicsComponent>(entity);
      auto& id_c = view_entities.get<IDComponent>(entity);
      auto phys = physics_[id_c.id];

      physics_c.velocity = phys.first;
      physics_c.is_airborne = phys.second;
    }
  }

  if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
    ToggleInGameMenu();
  }
  if (show_in_game_menu_buttons_) {
    glm::vec2 in_game_menu_pos = glob::window::GetWindowDimensions();
    in_game_menu_pos /= 2;
    in_game_menu_pos.x -= 165;
    in_game_menu_pos.y -= 180;
    glob::Submit(in_game_menu_gui_, in_game_menu_pos, 1.0f);
  }
  // Submit 2D Element TEST
  glob::Submit(e2D_test_, glm::vec3(10.5f, 1.0f, 0.0f), 2, -90.0f,
    glm::vec3(0, 1, 0));
  glob::Submit(e2D_test_, glm::vec3(-10.5f, 1.0f, 0.0f), 2, 90.0f,
    glm::vec3(0, 1, 0));
  glob::Submit(e2D_test2_, glm::vec3(0.0f, 1.0f, -7.0f), 7, 0.0f, glm::vec3(1));

  UpdateGameplayTimer();

  // draw stamina bar
  glob::Submit(gui_stamina_base_, glm::vec2(0, 5), 0.85, 100);
  glob::Submit(gui_stamina_fill_, glm::vec2(7, 12), 0.85, current_stamina_);
  glob::Submit(gui_stamina_icon_, glm::vec2(0, 5), 0.85, 100);

  // draw Minimap
  glob::Submit(gui_minimap_, glm::vec2(glob::window::GetWindowDimensions().x - 250, 10), 0.3);
  // draw Minimap goals
  if (!goals_swapped_) {
    glob::Submit(gui_minimap_goal_red_, glm::vec2(glob::window::GetWindowDimensions().x - 159.2, 10), 0.2);
    glob::Submit(gui_minimap_goal_blue_, glm::vec2(glob::window::GetWindowDimensions().x - 159.2, 367.2), 0.2);
  } else {
    glob::Submit(gui_minimap_goal_red_, glm::vec2(glob::window::GetWindowDimensions().x - 159.2, 367.2), 0.2);
    glob::Submit(gui_minimap_goal_blue_, glm::vec2(glob::window::GetWindowDimensions().x - 159.2, 10), 0.2);
  }

  // Draw Player icons
  auto view_player = registry_gameplay_.view<TransformComponent, PlayerComponent, IDComponent>();
  for (auto entity : view_player) {
    auto& trans_c = view_player.get<TransformComponent>(entity);
    auto& id_c = view_player.get<IDComponent>(entity);

    // Normalize and project player pos to screen space (Z in world space is X in screen space and vice versa)
    float norm_pos_x = trans_c.position.z / 7.1f;
    float norm_pos_y = trans_c.position.x / 10.6f;
    float minimap_pos_x = (norm_pos_x * 120.f) + glob::window::GetWindowDimensions().x - 130.f - 11.f;
    float minimap_pos_y = (norm_pos_y * 190.f) + 190.f - 20.f;

    // Draw the right color icons
    if (engine_->GetPlayerTeam(id_c.id) == TEAM_RED) {
      glob::Submit(gui_minimap_player_red_, glm::vec2(minimap_pos_x, minimap_pos_y), 0.1);    // TODO: CALC REAL POS
    } else {
      glob::Submit(gui_minimap_player_blue_, glm::vec2(minimap_pos_x, minimap_pos_y), 0.1);   // TODO: CALC REAL POS
    }
  }

  // Draw Ball icon
  auto view_ball = registry_gameplay_.view<TransformComponent, BallComponent>();
  for (auto entity : view_ball) {
    auto& trans_c = view_ball.get<TransformComponent>(entity);

    // Normalize and project player pos to screen space (Z in world space is X in screen space and vice versa)
    float norm_pos_x = trans_c.position.z / 7.1f;
    float norm_pos_y = trans_c.position.x / 10.6f;
    float minimap_pos_x = (norm_pos_x * 120.f) + glob::window::GetWindowDimensions().x - 130.f - 20.f;
    float minimap_pos_y = (norm_pos_y * 190.f) + 190.f - 20.f;

    glob::Submit(gui_minimap_ball_, glm::vec2(minimap_pos_x, minimap_pos_y), 0.1);
  }

  // draw quickslot info
  glob::Submit(gui_quickslots_, glm::vec2(7, 50), 0.3, 100);
  glob::Submit(ability_handles_[my_primary_ability_id], glm::vec2(9, 50), 0.75f,
    100);
  glob::Submit(ability_handles_[(int)engine_->GetSecondaryAbility()],
    glm::vec2(66, 50), 0.75f, 100);

  if (game_has_ended_) {
    engine_->DrawScoreboard();

    glm::vec2 pos = glob::window::GetWindowDimensions();
    pos /= 2;
    pos.y -= 160;
    pos.x -= 175;

    std::string best_team = "    BLUE";
    glm::vec4 best_team_color = glm::vec4(0.13f, 0.13f, 1.f, 1.f);

    if (engine_->GetTeamScores()[0] > engine_->GetTeamScores()[1]) {
      best_team = "    RED";
      best_team_color = glm::vec4(1.f, 0.13f, 0.13f, 1.f);
    } else if (engine_->GetTeamScores()[0] == engine_->GetTeamScores()[1]) {
      best_team = "  NO TEAM";
      best_team_color = glm::vec4(.8f, .4f, .4f, 1.f);
    }

    glob::Submit(font_test_, pos + glm::vec2(41, -1), 48, best_team + " wins!",
      glm::vec4(0, 0, 0, 0.7f));

    glob::Submit(font_test_, pos + glm::vec2(40, 0), 48, best_team + " wins!",
      best_team_color);

    int game_end_timeout = 5;
    std::string end_countdown_text =
      std::to_string((int)(game_end_timeout - end_game_timer_.Elapsed()));

    glob::Submit(font_test_, pos + glm::vec2(0, -50), 48,
      "Returning to lobby in: " + end_countdown_text);

    if (end_game_timer_.Elapsed() >= 5.0f) {
      engine_->ChangeState(StateType::LOBBY);
    }
  }
  DrawTopScores();
  DrawTarget();
}
void PlayState::UpdateNetwork() {
  auto& packet = engine_->GetPacket();

  // TEMP: Start recording replay
  bool temp = Input::IsKeyPressed(GLFW_KEY_P);
  packet << temp;
  packet << PacketBlockType::TEST_REPLAY_KEYS;
}

void PlayState::Cleanup() {
  registry_gameplay_.reset();
  game_has_ended_ = false;
}

void PlayState::ToggleInGameMenu() {
  show_in_game_menu_buttons_ = !show_in_game_menu_buttons_;
  glob::window::SetMouseLocked(!show_in_game_menu_buttons_);
  engine_->SetSendInput(!show_in_game_menu_buttons_);
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
  int temp = match_time_ - engine_->GetGameplayTimer();
  int sec = 0;
  int min = 5;

  min = temp / 60;
  sec = temp % 60;

  // Countdown timer
  int count = countdown_time_ - engine_->GetCountdownTimer();

  glm::vec2 pos = glob::window::GetWindowDimensions();
  pos.x /= 2;
  pos.x += 4;
  pos.y -= 6;

  std::string min_string = std::to_string(min);
  std::string sec_string = std::to_string(sec);

  // min_string = "0" + min_string;
  if (sec < 10) sec_string = "0" + sec_string;

  // --------------------------------------
  glob::Submit(font_test_, pos, 40, min_string, glm::vec4(1));
  glob::Submit(font_test_, pos + glm::vec2(-8, -12), 40, "----",
    glm::vec4(1, 1, 1, 1));
  glob::Submit(font_test_, pos + glm::vec2(-7, -26), 40, sec_string,
    glm::vec4(1));

  glm::vec2 countdown_pos = glob::window::GetWindowDimensions();
  countdown_pos /= 2;
  countdown_pos.x += 75;
  countdown_pos.y += 100;
  if (count > 0) {
    glob::Submit(font_test_, countdown_pos, 500, std::to_string(count),
      glm::vec4(1));
  }
}

void PlayState::DrawTopScores() {
  glm::vec2 team_score_pos = glob::window::GetWindowDimensions();
  team_score_pos.x /= 2;
  team_score_pos.x -= 145;
  team_score_pos.y -= 60;
  glob::Submit(gui_teamscore_, team_score_pos, 1, 100);

  glob::Submit(font_scores_, team_score_pos + glm::vec2(90, 55), 72,
    std::to_string(engine_->GetTeamScores()[1]),
    glm::vec4(0, 0.26, 1, 1));
  glob::Submit(font_scores_, team_score_pos + glm::vec2(217, 55), 72,
    std::to_string(engine_->GetTeamScores()[0]),
    glm::vec4(1, 0, 0, 1));
}

void PlayState::DrawTarget() {
  if (my_target_ != -1) {
    auto view_players =
        registry_gameplay_
            .view<PlayerComponent, TransformComponent, IDComponent>();
    glm::vec3 target_pos;
    glm::vec3 my_pos;
    glm::vec3 my_forward;

    for (auto player : view_players) {
      auto& p_c = registry_gameplay_.get<PlayerComponent>(player);
      auto& t_c = registry_gameplay_.get<TransformComponent>(player);
      auto& id_c = registry_gameplay_.get<IDComponent>(player);

      if (id_c.id == my_target_) {
        target_pos = t_c.position;
      }
      if (id_c.id == my_id_) {
        my_pos = t_c.position;
        my_forward = t_c.Forward();
        if (registry_gameplay_.has<CameraComponent>(player)) {
          auto& c_c = registry_gameplay_.get<CameraComponent>(player);
          my_forward = c_c.GetLookDir();
        }
      }
    }
    glm::vec3 diff = target_pos - my_pos;
    glm::vec3 dir = glm::normalize(diff);
    float dot = glm::dot(my_forward, dir);

    float angles = glm::atan(dir.x,dir.z);
    glm::vec3 cross = glm::normalize(glm::cross(my_forward, dir));

    glob::Submit(e2D_target_, target_pos, 1.0f, glm::degrees(angles)+180.f,
                 glm::vec3(0,1,0));
  }
}

void PlayState::SetEntityTransform(EntityID player_id, glm::vec3 pos,
  glm::quat orientation) {
  transforms_[player_id] = std::make_pair(pos, orientation);
}

void PlayState::SetEntityPhysics(EntityID player_id, glm::vec3 vel,
                                 bool is_airborne) {
  physics_[player_id] = std::make_pair(vel, is_airborne);
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
  auto& sound_engine = engine_->GetSoundEngine();

  std::cout << "DEBUG: playstate.cpp: Created " << player_ids_.size()
    << " players\n";

  for (auto entity_id : player_ids_) {
    auto entity = registry_gameplay_.create();

    glm::vec3 alter_scale =
        glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f);
    glm::vec3 character_scale = glm::vec3(0.0033f);

    glob::ModelHandle player_model = glob::GetModel("Assets/Mech/Mech.fbx");

    registry_gameplay_.assign<IDComponent>(entity, entity_id);
    registry_gameplay_.assign<PlayerComponent>(entity);
    registry_gameplay_.assign<TransformComponent>(entity, glm::vec3(0.f),
                                                  glm::quat(), character_scale);
    registry_gameplay_.assign<ModelComponent>(entity, player_model,
                                              glm::vec3(0.f, 0.9f, 0.f));
    registry_gameplay_.assign<AnimationComponent>(
        entity, glob::GetAnimationData(player_model));
    registry_gameplay_.assign<PhysicsComponent>(entity);
    registry_gameplay_.assign<SoundComponent>(entity,
                                              sound_engine.CreatePlayer());

    if (entity_id == my_id_) {
      glm::vec3 camera_offset = glm::vec3(0.1f, 0.7f, 0.f);
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
  auto& sound_engine = engine_->GetSoundEngine();

  // Ball
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  auto ball = registry_gameplay_.create();
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/TestBall.fbx");
  registry_gameplay_.assign<ModelComponent>(ball, model_ball);
  registry_gameplay_.assign<TransformComponent>(ball, zero_vec, zero_vec,
    glm::vec3(1.0f));
  registry_gameplay_.assign<PhysicsComponent>(ball);
  registry_gameplay_.assign<BallComponent>(ball);
  registry_gameplay_.assign<IDComponent>(ball, ball_id_);
  registry_gameplay_.assign<SoundComponent>(ball, sound_engine.CreatePlayer());
}

void PlayState::CreateInGameMenu() {
  font_test_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  glm::vec2 in_game_menu_pos = glob::window::GetWindowDimensions();
  in_game_menu_pos /= 2;
  in_game_menu_pos.x -= 110;
  in_game_menu_pos.y += 110;

  // CONTINUE BUTTON -- change registry to registry_gameplay_
  ButtonComponent* in_game_buttons_ = GenerateButtonEntity(
    registry_gameplay_, "CONTINUE", in_game_menu_pos + glm::vec2(0, 0),
    font_test_, false);
  in_game_buttons_->button_func = [&]() { ToggleInGameMenu(); };
  // SETTINGS BUTTON -- change registry to registry_settings_
  in_game_buttons_ = GenerateButtonEntity(registry_gameplay_, "SETTINGS",
    in_game_menu_pos + glm::vec2(0, -70),
    font_test_, false);

  in_game_buttons_->button_func = [&] {
    engine_->ChangeState(StateType::SETTINGS);
    ToggleInGameMenu();
  };

  // END GAME -- change registry to registry_mainmenu_
  in_game_buttons_ = GenerateButtonEntity(registry_gameplay_, "MAINMENU",
    in_game_menu_pos + glm::vec2(0, -140),
    font_test_, false);
  in_game_buttons_->button_func = [&] {
    engine_->ChangeState(StateType::MAIN_MENU);
  };

  in_game_buttons_ = GenerateButtonEntity(registry_gameplay_, "EXIT",
    in_game_menu_pos + glm::vec2(0, -210),
    font_test_, false);
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
    glob::GetModel("assets/Pickup/Pickup.fbx");
  registry_gameplay_.assign<ModelComponent>(pick_up, model_pick_up);
  registry_gameplay_.assign<TransformComponent>(
    pick_up, position, glm::vec3(0.0f, 0.0f, 0.f), glm::vec3(0.4f));
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

void PlayState::CreateTeleportProjectile(EntityID id) {
  auto teleport_projectile = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  /*glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  registry_gameplay_.assign<ModelComponent>(teleport_projectile, model_ball);*/
  registry_gameplay_.assign<TransformComponent>(teleport_projectile, zero_vec, zero_vec,
    glm::vec3(0.3f));
  registry_gameplay_.assign<IDComponent>(teleport_projectile, id);
}

void PlayState::CreateForcePushObject(EntityID id) {
  auto force_object = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  registry_gameplay_.assign<ModelComponent>(force_object, model_ball);
  registry_gameplay_.assign<TransformComponent>(force_object, zero_vec,
    zero_vec, glm::vec3(0.5f));
  registry_gameplay_.assign<IDComponent>(force_object, id);
}

void PlayState::CreateMissileObject(EntityID id) {
  auto missile_object = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glob::ModelHandle model_ball = glob::GetModel("assets/Rocket/Rocket.fbx");
  registry_gameplay_.assign<ModelComponent>(missile_object, model_ball);
  registry_gameplay_.assign<TransformComponent>(missile_object, zero_vec,
                                                zero_vec, glm::vec3(0.5f));
  registry_gameplay_.assign<IDComponent>(missile_object, id);
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
  if (!goals_swapped_) {
    goals_swapped_ = true;
  } else {
    goals_swapped_ = false;
  }
  TransformComponent& blue_light_trans_c =
    registry_gameplay_.get<TransformComponent>(blue_goal_light_);
  TransformComponent& red_light_trans_c =
    registry_gameplay_.get<TransformComponent>(red_goal_light_);

  glm::vec3 blue_light_pos = blue_light_trans_c.position;
  blue_light_trans_c.position = red_light_trans_c.position;
  red_light_trans_c.position = blue_light_pos;
}

void PlayState::EndGame() {
  end_game_timer_.Restart();
  game_has_ended_ = true;
}