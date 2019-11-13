#include "state.hpp"

#include <GLFW/glfw3.h>
#include <boundingboxes.hpp>
#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include <slob/sound_engine.hpp>

#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "shared/camera_component.hpp"
#include "shared/id_component.hpp"
#include "shared/transform_component.hpp"

#include <collision.hpp>
#include <ecs\components\trail_component.hpp>
#include <shared/fail_safe_arena.hpp>
#include <physics.hpp>
#include <shared/physics_component.hpp>
#include <shared/pick_up_component.hpp>
#include "ecs/components.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"
#include "eventdispatcher.hpp"
#include "util/global_settings.hpp"
#include "util/input.hpp"
#include <util/asset_paths.hpp>

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
  e2D_outline_ = glob::GetE2DItem("assets/GUI_elements/wall_outline.png");

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
  gui_minimap_goal_red_ =
      glob::GetGUIItem("assets/GUI_elements/goal_red_icon.png");
  gui_minimap_goal_blue_ =
      glob::GetGUIItem("assets/GUI_elements/goal_blue_icon.png");
  gui_minimap_player_red_ =
      glob::GetGUIItem("assets/GUI_elements/player_iconv2_red.png");
  gui_minimap_player_blue_ =
      glob::GetGUIItem("assets/GUI_elements/player_iconv2_blue.png");
  gui_minimap_ball_ = glob::GetGUIItem("assets/GUI_elements/Ball_Icon.png");
  gui_crosshair_ = glob::GetGUIItem("assets/GUI_elements/Crosshair_V1.png");

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

  test_ball_ = glob::GetTransparentModel("Assets/Ball_new/Ball_Sphere.fbx");
  glob::GetModel("assets/Pickup/Pickup.fbx");
}

void PlayState::CreateGoalParticles(float x) {
  auto e = registry_gameplay_.create();
  auto handle = glob::CreateParticleSystem();

  std::vector handles = {handle};
  std::vector<glm::vec3> offsets;
  //= {glm::vec3(0.f)};
  std::vector<glm::vec3> directions;
  //= {glm::vec3(0.f, 1.f, 0.f)};

  glob::SetParticleSettings(handle, "confetti.txt");
  glob::SetEmitPosition(handle, glm::vec3(x * 0.9f, 1.f, 0.f));
  float x_dir = (x > 0) ? -1 : 1;
  glob::SetParticleDirection(handle, glm::vec3(x_dir, 5.f, 0.f));

  e = registry_gameplay_.create();
  handle = glob::CreateParticleSystem();
  handles.push_back(handle);
  glob::SetParticleSettings(handle, "goal_fire.txt");
  glob::SetEmitPosition(handle, glm::vec3(x * 1.0f, 0.f, 15.f));
  e = registry_gameplay_.create();
  handle = glob::CreateParticleSystem();
  handles.push_back(handle);
  glob::SetParticleSettings(handle, "goal_fire.txt");
  glob::SetEmitPosition(handle, glm::vec3(x * 1.0f, 0.f, -15.f));
  // std::unordered_map<std::string, std::string> map;
  // map["color"] = "1.0 0.0 0.0 0.4";
  // glob::SetParticleSettings(handle, map);
  // auto ball_view = registry_gameplay_.view<

  registry_gameplay_.assign<ParticleComponent>(e, handles, offsets, directions);
  // std::cout << handles.size() << " particle systems" << std::endl;
  // Temp
  registry_gameplay_.assign<int>(e, 0);
}

void PlayState::Init() {
  glob::window::SetMouseLocked(true);
  engine_->SetSendInput(true);
  engine_->SetCurrentRegistry(&registry_gameplay_);
  engine_->SetEnableChat(true);

  // end_game_timer_.Restart();
  game_has_ended_ = false;
  my_target_ = -1;
  primary_cd_ = 0;

  CreateInGameMenu();
  CreateInitialEntities();
  // TestParticles();

  engine_->GetChat()->SetPosition(
      glm::vec2(30, glob::window::GetWindowDimensions().y - 30));

  auto& client = engine_->GetClient();
  NetAPI::Common::Packet to_send;
  bool rec = true;
  to_send << rec;
  to_send << PacketBlockType::CLIENT_RECEIVE_UPDATES;
  client.Send(to_send);

  engine_->GetSoundSystem().PlayAmbientSound(registry_gameplay_);
  goals_swapped_ = false;
  primary_cd_ = 0.f;
  engine_->SetSecondaryAbility(AbilityID::NULL_ABILITY);
  engine_->GetChat()->CloseChat();
  timer_ = 0.f;
  Reset();
}

void PlayState::Update(float dt) {
  auto& cli = engine_->GetClient();
  if (!cli.IsConnected()) {
    cli.Disconnect();
    engine_->ChangeState(StateType::MAIN_MENU);
  }

  if (!player_look_dirs_.empty()) {
    auto view_entities =
        registry_gameplay_.view<PlayerComponent, IDComponent>();
    for (auto entity : view_entities) {
      auto& player_c = view_entities.get<PlayerComponent>(entity);
      auto& id_c = view_entities.get<IDComponent>(entity);
      auto iter = player_look_dirs_.find(id_c.id);
      if (iter != player_look_dirs_.end()) {
        player_c.look_dir = iter->second;
        // std::cout << "lookdir.x: " << iter->second.x << "\n";
      }
    }
    player_look_dirs_.clear();
  }

  if (!player_move_dirs_.empty()) {
    auto view_entities =
        registry_gameplay_.view<PlayerComponent, IDComponent>();
    for (auto entity : view_entities) {
      auto& player_c = view_entities.get<PlayerComponent>(entity);
      auto& id_c = view_entities.get<IDComponent>(entity);
      auto iter = player_move_dirs_.find(id_c.id);
      if (iter != player_move_dirs_.end()) {
        player_c.move_dir = iter->second;
      }
    }
    player_move_dirs_.clear();
  }

  this->server_state_ = engine_->GetServerState();
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
    // physics_.clear();  //?
  }

  if (!transforms_.empty()) {
    auto view_entities =
        registry_gameplay_.view<TransformComponent, IDComponent>();
    glm::vec3 pos = new_transforms_[ball_id_].first;
    new_transforms_.clear();
    for (auto entity : view_entities) {
      auto& trans_c = view_entities.get<TransformComponent>(entity);
      auto& id_c = view_entities.get<IDComponent>(entity);
      auto trans = transforms_[id_c.id];

      new_transforms_[id_c.id] = std::make_pair(trans.first, trans.second);

      /*
      std::cout << trans_c.position.x << ", ";
      std::cout << trans_c.position.y << ", ";
      std::cout << trans_c.position.z << "\n";
      */
    }
    transforms_.clear();
    OnServerFrame();
    // MovePlayer(1 / 64.0f);
    actions_.clear();
  }
  timer_ += dt;
  if (timer_ > 1.0f / kClientUpdateRate) {
    MoveBall(dt);
    MovePlayer(dt);
    timer_ -= dt;
  }
  // interpolate
  auto view_entities =
      registry_gameplay_.view<TransformComponent, IDComponent>();

  float f = 0.5;  // 0.25f * dt;  // pow(0.75f, dt);
  for (auto entity : view_entities) {
    auto& trans_c = view_entities.get<TransformComponent>(entity);
    auto& id_c = view_entities.get<IDComponent>(entity);
    if (id_c.id == my_id_) {
      auto trans = new_transforms_[id_c.id];
      auto& cam_c = registry_gameplay_.get<CameraComponent>(my_entity_);
      glm::vec3 temp =
          lerp(predicted_state_.position, server_predicted_.position, 0.5f);
      trans_c.position = glm::lerp(trans_c.position, temp, 0.8f);
      //trans_c.position = trans.first;
      glm::quat orientation =
          glm::quat(glm::vec3(0, yaw_, 0)) * glm::quat(glm::vec3(0, 0, pitch_));
      orientation = glm::normalize(orientation);
      if (!show_in_game_menu_buttons_) {
        cam_c.orientation = orientation;
        trans_c.rotation = glm::quat(glm::vec3(0, yaw_, 0));
      }
    } else {
      auto trans = new_transforms_[id_c.id];
      if (glm::length(trans_c.position - trans.first) > 10) {
        trans_c.position = trans.first;
      } else {
        trans_c.position = glm::lerp(trans_c.position, trans.first, 0.5f);
        // trans_c.position = trans.first;
      }
      bool slerp = true;
      if (id_c.id == ball_id_) {
        auto ball_view =
            registry_gameplay_.view<BallComponent, PhysicsComponent>();
        for (auto ball : ball_view) {
          auto& phys_c = ball_view.get<PhysicsComponent>(ball);
          if (phys_c.is_airborne == false) {
            slerp = false;
          }
        }
      }
      if (slerp == true) {
        trans_c.rotation = glm::slerp(trans_c.rotation, trans.second, 0.2f);
      }
    }
  }
  Collision();
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
  // glob::Submit(e2D_test_, glm::vec3(10.5f, 1.0f, 0.0f), 2, -90.0f,
  // glm::vec3(0, 1, 0));
  // glob::Submit(e2D_test_, glm::vec3(-10.5f, 1.0f, 0.0f), 2, 90.0f,
  // glm::vec3(0, 1, 0));
  glob::Submit(e2D_test2_, glm::vec3(0.0f, 1.0f, -28.0f), 7, 0.0f,
               glm::vec3(1));

  UpdateGameplayTimer();
  UpdateSwitchGoalTimer();
  DrawNameOverPlayer();
  DrawWallOutline();

  // draw stamina bar
  glob::Submit(gui_stamina_base_, glm::vec2(0, 5), 0.85, 100);
  glob::Submit(gui_stamina_fill_, glm::vec2(7, 12), 0.85, current_stamina_);
  glob::Submit(gui_stamina_icon_, glm::vec2(0, 5), 0.85, 100);

  // draw crosshair
  glm::vec2 crosshair_pos = glob::window::GetWindowDimensions();
  crosshair_pos /= 2;
  glob::Submit(gui_crosshair_, crosshair_pos - glm::vec2(19, 20), 1.f);

  // draw Minimap
  glob::Submit(gui_minimap_,
               glm::vec2(glob::window::GetWindowDimensions().x - 250, 10), 0.3);
  // draw Minimap goals
  if (!goals_swapped_) {
    glob::Submit(gui_minimap_goal_red_,
                 glm::vec2(glob::window::GetWindowDimensions().x - 159.2, 10),
                 0.2);
    glob::Submit(
        gui_minimap_goal_blue_,
        glm::vec2(glob::window::GetWindowDimensions().x - 159.2, 367.2), 0.2);
  } else {
    glob::Submit(
        gui_minimap_goal_red_,
        glm::vec2(glob::window::GetWindowDimensions().x - 159.2, 367.2), 0.2);
    glob::Submit(gui_minimap_goal_blue_,
                 glm::vec2(glob::window::GetWindowDimensions().x - 159.2, 10),
                 0.2);
  }

  // Draw Player icons
  auto view_player =
      registry_gameplay_
          .view<TransformComponent, PlayerComponent, IDComponent>();
  for (auto entity : view_player) {
    auto& trans_c = view_player.get<TransformComponent>(entity);
    auto& id_c = view_player.get<IDComponent>(entity);
    auto& player_c = view_player.get<PlayerComponent>(entity);

    // Normalize and project player pos to screen space (Z in world space is X
    // in screen space and vice versa)
    float norm_pos_x = trans_c.position.z / 28.1f;
    float norm_pos_y = trans_c.position.x / 40.6f;
    float minimap_pos_x = (norm_pos_x * 120.f) +
                          glob::window::GetWindowDimensions().x - 130.f - 11.f;
    float minimap_pos_y = (norm_pos_y * 190.f) + 190.f - 20.f;

    // Draw the right color icons
    if (engine_->GetPlayerTeam(id_c.id) == TEAM_RED) {
      glob::Submit(gui_minimap_player_red_,
                   glm::vec2(minimap_pos_x, minimap_pos_y),
                   0.1);  // TODO: CALC REAL POS
    } else {
      glob::Submit(gui_minimap_player_blue_,
                   glm::vec2(minimap_pos_x, minimap_pos_y),
                   0.1);  // TODO: CALC REAL POS
    }
  }

  // Draw Ball icon
  auto view_ball = registry_gameplay_.view<TransformComponent, BallComponent>();
  for (auto entity : view_ball) {
    auto& trans_c = view_ball.get<TransformComponent>(entity);

    // Normalize and project player pos to screen space (Z in world space is X
    // in screen space and vice versa)
    float norm_pos_x = trans_c.position.z / 28.1f;
    float norm_pos_y = trans_c.position.x / 40.6f;
    float minimap_pos_x = (norm_pos_x * 120.f) +
                          glob::window::GetWindowDimensions().x - 130.f - 20.f;
    float minimap_pos_y = (norm_pos_y * 190.f) + 190.f - 20.f;

    glob::Submit(gui_minimap_ball_, glm::vec2(minimap_pos_x, minimap_pos_y),
                 0.1);
  }

  if (overtime_has_started_) {
    glm::vec2 pos = glob::window::GetWindowDimensions();
    pos /= 2;
    pos.x -= 225;
    pos.y += 400;

    glob::Submit(font_test_, pos, 175, "OVERTIME");

    if (game_has_ended_) {
      overtime_has_started_ = false;
    }
  }

  if (game_has_ended_) {
    engine_->DrawScoreboard();

    glm::vec2 pos = glob::window::GetWindowDimensions();
    pos /= 2;
    pos.y -= 160;

    std::string best_team = "BLUE";
    glm::vec4 best_team_color = glm::vec4(0.13f, 0.13f, 1.f, 1.f);

    if (engine_->GetTeamScores()[0] > engine_->GetTeamScores()[1]) {
      best_team = "RED";
      best_team_color = glm::vec4(1.f, 0.13f, 0.13f, 1.f);
    }

    std::string winnin_team_text = best_team + " wins!";
    double width = glob::GetWidthOfText(font_test_, winnin_team_text, 48);

    pos.x -= width / 2;

    glob::Submit(font_test_, pos + glm::vec2(1, -1), 48, winnin_team_text,
                 glm::vec4(0, 0, 0, 0.7f));

    glob::Submit(font_test_, pos, 48, winnin_team_text, best_team_color);

    int game_end_timeout = 5;
    std::string end_countdown_text =
        std::to_string((int)(game_end_timeout - end_game_timer_.Elapsed()));

    std::string return_to_lobby_test =
        "Returning to lobby in: " + end_countdown_text;
    width = glob::GetWidthOfText(font_test_, return_to_lobby_test, 48);
    pos.x = (glob::window::GetWindowDimensions().x / 2) - (width / 2);
    glob::Submit(font_test_, pos + glm::vec2(0, -40), 48, return_to_lobby_test);

    if (end_game_timer_.Elapsed() >= 5.0f) {
      engine_->ChangeState(StateType::LOBBY);
    }
  }
  if (primary_cd_ > 0) {
    primary_cd_ -= dt;
  }
  DrawTopScores();
  DrawTarget();
  DrawQuickslots();

  glob::Submit(test_ball_, glm::mat4());
}

void PlayState::UpdateNetwork() {
  auto& packet = engine_->GetPacket();

  frame_id++;
  packet << frame_id;
  packet << PacketBlockType::FRAME_ID;

  // TEMP: Start recording replay
  //bool temp = Input::IsKeyPressed(GLFW_KEY_P);
  //packet << temp;
  //packet << PacketBlockType::TEST_REPLAY_KEYS;
}

void PlayState::Cleanup() {
  registry_gameplay_.reset();
  game_has_ended_ = false;
}

void PlayState::ToggleInGameMenu() {
  show_in_game_menu_buttons_ = !show_in_game_menu_buttons_;
  glob::window::SetMouseLocked(!show_in_game_menu_buttons_);
  engine_->SetSendInput(!show_in_game_menu_buttons_);
  engine_->SetTakeInput(!show_in_game_menu_buttons_);
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
  int temp = 0;
  if (!overtime_has_started_) {
    temp = match_time_ - engine_->GetGameplayTimer();
  } else {
    temp = engine_->GetGameplayTimer() - match_time_;
  }
  // Gameplay timer
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

void PlayState::DrawNameOverPlayer() {
  auto player_view =
      registry_gameplay_.view<PlayerComponent, TransformComponent,
                              ModelComponent, IDComponent>();

  auto& my_transform = registry_gameplay_.get<TransformComponent>(my_entity_);
  for (auto entity : player_view) {
    if (my_entity_ == entity) {
      continue;
    }

    auto& id_c = player_view.get<IDComponent>(entity);
    auto& transform = player_view.get<TransformComponent>(entity);
    auto& model = player_view.get<ModelComponent>(entity);

    if (!model.invisible) {
      for (auto& [id, name] : engine_->player_names_) {
        EntityID e_id = ClientIDToEntityID(id);
        if (e_id == id_c.id) {
          glm::vec3 look =
              glm::vec3(transform.position - my_transform.position);
          float distance = glm::length(look);
          glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
          look.y = 0;
          look = glm::normalize(look);

          glm::vec3 right = glm::cross(look, up);
          glm::mat4 matrix(1.0f);
          matrix[0] = glm::vec4(right, 0.f);
          matrix[1] = glm::vec4(up, 0.f);
          matrix[2] = glm::vec4(look, 0.f);

          float size = 1.f;
          float offset = 0.f;
          glm::vec4 color = glm::vec4(1.0f);
          if (distance < 10.f) {
            offset = 1.3f;
            size = 0.3f;
          } else if (distance > 40.f) {
            break;
          } else {
            auto factor = (distance - 10.f) / 60.f;
            offset = glm::lerp(1.3f, 3.0f, factor);
            size = glm::lerp(0.3f, 1.5f, factor);
          }

          auto team = engine_->GetPlayerTeam(e_id);
          if (team == TEAM_BLUE)
            color = glm::vec4(0.f, 0.f, 1.f, 1.f);
          else if (team == TEAM_RED)
            color = color = glm::vec4(1.f, 0.f, 0.f, 1.f);

          glob::Submit(font_test_, transform.position + up * offset, size, name,
                       color, matrix);

          break;
        }
      }
    }
    break;
  }
}

void PlayState::DrawWallOutline() {
  if (my_primary_ability_id == (int)AbilityID::BUILD_WALL && primary_cd_ <= 0.f || engine_->GetSecondaryAbility() == AbilityID::BUILD_WALL) {
    auto& trans = registry_gameplay_.get<TransformComponent>(my_entity_);
    auto& camera = registry_gameplay_.get<CameraComponent>(my_entity_);

    glm::vec3 pos =
        camera.GetLookDir() * 4.5f + trans.position + camera.offset;
    pos.y = 0.05f;

   
    if (glm::distance(pos, glm::vec3(-40.f, -3.9f, 0.f)) < 20.f ||
        glm::distance(pos, glm::vec3(40.f, -3.9f, 0.f)) < 20.f) {
      return;
    }

    glm::mat4 mat =
        glm::rotate(glm::pi<float>() * 0.5f, glm::vec3(-1.f, 0.f, 0.f));

    mat = glm::rotate(glm::pi<float>() * 0.5f, glm::vec3(0.f, 1.f, 0.f)) * mat;
    mat = glm::scale(glm::vec3(5, 0.f, 5)) * mat;
    mat = glm::toMat4(trans.rotation) * mat;
    //*glm::toMat4(trans.rotation) *
    //                    glm::scale(glm::vec3(1, 0.f, 5));
    glob::Submit(
        e2D_outline_, pos,
        mat);

    //glob::SubmitCube(glm::translate(pos) *
    //                 glm::toMat4(trans.rotation) *
    //    glm::scale(glm::vec3(1, 0.f, 5)));
  }
}

void PlayState::UpdateSwitchGoalTimer() {
  // Countdown timer
  int temp_time = engine_->GetSwitchGoalTime();
  float time = engine_->GetSwitchGoalCountdownTimer();
  int count = (int)time;

  // Start countdown
  if (time < temp_time) {
    countdown_in_progress_ = true;
  }

#define LIGHT_EFFECT 2
#if LIGHT_EFFECT == 0
  if (countdown_in_progress_) {
    float temp = time;
    int temp1 = (int)temp;
    float dim = temp - temp1;

    if (dim < 0.5f) {
      auto& blue_light =
          registry_gameplay_.get<LightComponent>(blue_goal_light_);
      blue_light.color = glm::vec3(0.f);

      auto& red_light = registry_gameplay_.get<LightComponent>(red_goal_light_);
      red_light.color = glm::vec3(0.f);
    } else {
      auto& blue_light =
          registry_gameplay_.get<LightComponent>(blue_goal_light_);
      blue_light.color = glm::vec3(0.1f, 0.1f, 1.f);

      auto& red_light = registry_gameplay_.get<LightComponent>(red_goal_light_);
      red_light.color = glm::vec3(1.f, 0.1f, 0.1f);
    }
  }
#elif LIGHT_EFFECT == 1
  if (countdown_in_progress_) {
    float temp = time / 2.f;
    int temp1 = (int)temp;
    float dim = temp - temp1;

    if (dim < 0.5f) {
      dim *= 2.f;
    } else {
      dim -= 0.5f;
      dim *= 2.f;
      dim = 1 - dim;
    }

    auto& blue_light = registry_gameplay_.get<LightComponent>(blue_goal_light_);
    blue_light.color = glm::vec3(0.1f, 0.1f, 1.f) * (0.0f + 1.0f * dim);

    auto& red_light = registry_gameplay_.get<LightComponent>(red_goal_light_);
    red_light.color = glm::vec3(1.f, 0.1f, 0.1f) * (0.0f + 1.f * dim);
  }
#elif LIGHT_EFFECT == 2
  if (countdown_in_progress_) {
    float temp = time / 2.f;
    int temp1 = (int)temp;
    float dim = temp - temp1;

    if (dim < 0.5f) {
      dim *= 2.f;
    } else {
      dim -= 0.5f;
      dim *= 2.f;
      dim = 1 - dim;
    }

    auto& blue_light = registry_gameplay_.get<LightComponent>(blue_goal_light_);
    blue_light.radius = 30 * dim;

    auto& red_light = registry_gameplay_.get<LightComponent>(red_goal_light_);
    red_light.radius = 30 * dim;
  }
#endif

  // Write out timer
  if (countdown_in_progress_) {
    glm::vec2 countdown_pos = glob::window::GetWindowDimensions();
    countdown_pos /= 2;
    countdown_pos.x += 10;
    countdown_pos.y += 265;

    glm::vec2 countdown_text_pos = glob::window::GetWindowDimensions();
    countdown_text_pos /= 2;
    countdown_text_pos.x -= 170;
    countdown_text_pos.y += 300;
    glob::Submit(font_test_, countdown_text_pos, 50,
                 std::string("SWITCHING GOALS IN..."), glm::vec4(0.8));
    glob::Submit(font_test_, countdown_pos, 100,
                 std::to_string(temp_time - count), glm::vec4(0.8));

    // After timer reaches zero swap goals
    if (temp_time - time <= 0) {
      auto& blue_light =
          registry_gameplay_.get<LightComponent>(blue_goal_light_);
      blue_light.color = glm::vec3(0.1f, 0.1f, 1.f);
      blue_light.radius = 30;

      auto& red_light = registry_gameplay_.get<LightComponent>(red_goal_light_);
      red_light.color = glm::vec3(1.f, 0.1f, 0.1f);
      red_light.radius = 30;
      SwitchGoals();

      // Save game event
      GameEvent switch_goals_event_done;
      switch_goals_event_done.type = GameEvent::SWITCH_GOALS_DONE;
      // dispatcher.trigger(switch_goals_event_done);

      countdown_in_progress_ = false;
    }
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

FrameState PlayState::SimulateMovement(std::vector<int>& action,
                                       FrameState& state, float dt) {
  auto view_controller =
      registry_gameplay_
          .view<CameraComponent, PlayerComponent, TransformComponent>();

  glm::vec3 final_velocity = glm::vec3(0.f, state.velocity.y, 0.f);
  FrameState new_state;
  new_state.is_airborne = state.is_airborne;
  for (auto entity : view_controller) {
    CameraComponent& cam_c = view_controller.get<CameraComponent>(entity);
    // PlayerComponent& player_c = view_controller.get<PlayerComponent>(entity);
    TransformComponent& trans_c =
        view_controller.get<TransformComponent>(entity);
    // PhysicsComponent& physics_c =
    // view_controller.get<PhysicsComponent>(entity);

    // Caputre keyboard input and apply velocity

    // auto& phys_c = registry_gameplay_.get<PhysicsComponent>(my_entity_);
    glm::vec3 accum_velocity = glm::vec3(0.f);

    constexpr float pi = glm::pi<float>();
    // state.pitch = glm::clamp(state.pitch, -0.49f * pi, 0.49f * pi);
    //// base movement direction on camera orientation.
    ////glm::vec3 frwd = cam_c.GetLookDir();
    glm::quat orientation = glm::quat(glm::vec3(0, state.yaw, 0)) *
                            glm::quat(glm::vec3(0, 0, state.pitch));
    // orientation = glm::normalize(orientation);
    glm::vec3 frwd = orientation * glm::vec3(1, 0, 0);
    frwd.y = 0;
    frwd = glm::normalize(frwd);
    glm::vec3 up(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(frwd, up));
    bool sprint = false;
    for (int a : action) {
      if (a == PlayerAction::WALK_FORWARD) {
        accum_velocity += frwd;
      }
      if (a == PlayerAction::WALK_BACKWARD) {
        accum_velocity -= frwd;
      }
      if (a == PlayerAction::WALK_RIGHT) {
        accum_velocity += right;
      }
      if (a == PlayerAction::WALK_LEFT) {
        accum_velocity -= right;
      }
      if (a == PlayerAction::SPRINT && sprinting_) {
        sprint = true;
      }
      auto& player_c = registry_gameplay_.get<PlayerComponent>(my_entity_);
      if (a == PlayerAction::JUMP && player_c.can_jump) {
        player_c.can_jump = false;
        // Add velocity upwards
        final_velocity += up * 8.0f;
        // Set them to be airborne
        new_state.is_airborne = true;
        // Subtract energy cost from resources
        // player_c.energy_current -= player_c.cost_jump;
      }
    }

    if (glm::length(accum_velocity) > 0.f)
      accum_velocity = glm::normalize(accum_velocity) * 12.0f;
    if (sprint) {
      accum_velocity *= 2.f;
    }

    // physics stuff

    final_velocity += accum_velocity;

    // physics_c.velocity = final_velocity;

    // glm::vec3 cur_move_dir = glm::normalize(physics_c.velocity);

    // IF player is pressing space
    // AND is not airborne
    // AND has more enery than the cost for jumping

    // slowdown
    glm::vec3 sidemov = glm::vec3(final_velocity.x, 0, final_velocity.z);
    float cur_move_speed = glm::length(sidemov);
    // if (cur_move_speed > 0.f) {
    // movement "floatiness", lower value = less floaty
    float t = 0.0005f;
    new_state.velocity.x =
        glm::mix(state.velocity.x, final_velocity.x, 1.f - glm::pow(t, dt));
    new_state.velocity.z =
        glm::mix(state.velocity.z, final_velocity.z, 1.f - glm::pow(t, dt));
    new_state.velocity.y = final_velocity.y;

    physics::PhysicsObject po;
    po.acceleration = glm::vec3(0.f);
    po.airborne = new_state.is_airborne;
    po.friction = 0.f;
    po.max_speed = 1000;
    po.position = state.position;
    po.velocity = new_state.velocity;
    physics::Update(&po, dt);
    new_state.velocity = po.velocity;
    new_state.position = po.position;
    new_state.pitch = state.pitch;
    new_state.yaw = state.yaw;
    return new_state;
  }
}

void PlayState::MovePlayer(float dt) {
  auto view_controller =
      registry_gameplay_
          .view<CameraComponent, PlayerComponent, TransformComponent>();

  PlayerData new_frame;
  new_frame.delta_time = dt;
  new_frame.id = frame_id;
  new_frame.pitch = predicted_state_.pitch;
  new_frame.yaw = predicted_state_.yaw;

  for (auto& a : actions_) {
    new_frame.actions.push_back(a);
  }
  auto& cam_o = registry_gameplay_.get<CameraComponent>(my_entity_).orientation;

  auto& trans_c = view_controller.get<TransformComponent>(my_entity_);
  predicted_state_.position = trans_c.position;
  FrameState new_state =
      SimulateMovement(new_frame.actions, predicted_state_, dt);
  predicted_state_ = new_state;
  // std::cout << "y: " << predicted_state_.velocity.y << std::endl;
  // std::cout << new_state.velocity.y << std::endl;
  history_.push_back(new_frame);

  server_predicted_.pitch = predicted_state_.pitch;
  server_predicted_.yaw = predicted_state_.yaw;
  new_state = SimulateMovement(new_frame.actions, server_predicted_, dt);
  server_predicted_ = new_state;
}

void PlayState::OnServerFrame() {
  auto& trans_c = registry_gameplay_.get<TransformComponent>(my_entity_);

  server_predicted_.position = new_transforms_[my_id_].first;
  server_predicted_.velocity =
      registry_gameplay_.get<PhysicsComponent>(my_entity_).velocity;
  server_predicted_.is_airborne =
      registry_gameplay_.get<PhysicsComponent>(my_entity_).is_airborne;

  auto& cam_o = registry_gameplay_.get<CameraComponent>(my_entity_).orientation;
  predicted_state_.pitch = pitch_;
  predicted_state_.yaw = yaw_;

  for (auto& frame : history_) {
    server_predicted_.pitch = frame.pitch;
    server_predicted_.yaw = frame.yaw;
    FrameState new_state =
        SimulateMovement(frame.actions, server_predicted_, frame.delta_time);
    server_predicted_ = new_state;
  }

  if (glm::length(predicted_state_.position - server_predicted_.position) >
      5.0f) {
    trans_c.position = server_predicted_.position;
    return;
  }
}

void PlayState::MoveBall(float dt) {
  auto view_ball =
      registry_gameplay_.view<BallComponent, TransformComponent,
                              PhysicsComponent, IDComponent, physics::Sphere>();

  for (auto ball : view_ball) {
    auto& phys_c = view_ball.get<PhysicsComponent>(ball);
    auto& trans_c = view_ball.get<TransformComponent>(ball);
    auto& id_c = view_ball.get<IDComponent>(ball);

    physics::PhysicsObject po;
    po.acceleration = glm::vec3(0.0f);
    po.airborne = phys_c.is_airborne;
    po.friction = 1.0f;
    po.max_speed = phys_c.max_speed;
    po.position = new_transforms_[id_c.id].first;
    po.velocity = phys_c.velocity;

    physics::Update(&po, dt);

    new_transforms_[id_c.id].first = po.position;
    phys_c.velocity = po.velocity;

    // Rotate the ball
    // ===========================================================================

    if (phys_c.is_airborne == false) {
      if (phys_c.velocity.x == 0.f && phys_c.velocity.z == 0.f) continue;

      physics::Sphere& sphere_c = view_ball.get<physics::Sphere>(ball);
      float distance = glm::length(phys_c.velocity);
      float radians = distance / sphere_c.radius;

      if (radians == 0.f) break;

      glm::vec3 direction =
          glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), phys_c.velocity));

      // glm::quat r = glm::angleAxis(radians, direction);
      direction = radians * direction;
      glm::quat r(0, direction);
      glm::quat spin = 0.5f * r * trans_c.rotation;

      trans_c.rotation += spin * dt;
      trans_c.rotation = glm::normalize(trans_c.rotation);
    } else {
      auto& ball_c = view_ball.get<BallComponent>(ball);
      glm::quat spin = 0.5f * ball_c.rotation * trans_c.rotation;
      trans_c.rotation += spin * dt;
      trans_c.rotation = glm::normalize(trans_c.rotation);
    }
  }
}

void PlayState::Collision() {
  auto view_moveable =
      registry_gameplay_.view<TransformComponent, physics::OBB>();
  for (auto object : view_moveable) {
    auto& hitbox = view_moveable.get<physics::OBB>(object);
    auto& transform = view_moveable.get<TransformComponent>(object);
    hitbox.center = transform.position;

    // Rotate OBB
    auto mat_rot = glm::toMat4(transform.rotation);
    hitbox.normals[0] = mat_rot * glm::vec4(1.0f, 0.f, 0.f, 0.f);
    hitbox.normals[2] = mat_rot * glm::vec4(0.0f, 0.f, 1.f, 0.f);
  }
  auto view_moveable1 =
      registry_gameplay_.view<TransformComponent, physics::Sphere>();
  for (auto object : view_moveable1) {
    auto& hitbox = view_moveable1.get<physics::Sphere>(object);
    auto& transform = view_moveable1.get<TransformComponent>(object);
    hitbox.center = transform.position;
  }

  auto& my_obb = registry_gameplay_.get<physics::OBB>(my_entity_);
  auto& my_phys_c = registry_gameplay_.get<PhysicsComponent>(my_entity_);
  auto& arena_hitbox =
      registry_gameplay_.get<physics::MeshHitbox>(arena_entity_);
  auto& arena_hitbox2 =
      registry_gameplay_.get<FailSafeArenaComponent>(arena_entity_);

  physics::IntersectData data =
      Intersect(arena_hitbox, my_obb, -my_phys_c.velocity);
  if (data.collision) {
    my_obb.center += data.move_vector;
    if (data.normal.y > 0.25) {
      auto& player_c = registry_gameplay_.get<PlayerComponent>(my_entity_);
      my_phys_c.velocity.y = 0.f;
      predicted_state_.velocity.y = 0.f;
      server_predicted_.velocity.y = 0.f;
      if (player_c.can_jump == false) {
        player_c.can_jump = true;
      }
    } else if (data.move_vector.y < 0.0f) {
      my_phys_c.velocity.y = 0.f;
      predicted_state_.velocity.y = 0.f;
      server_predicted_.velocity.y = 0.f;
    }
  }
  if (my_obb.center.x > arena_hitbox2.arena.xmax - 0.9f) {
    my_obb.center.x = arena_hitbox2.arena.xmax - 0.9f;
  } else if (my_obb.center.x < arena_hitbox2.arena.xmin + 0.9f) {
    my_obb.center.x = arena_hitbox2.arena.xmin + 0.9f;
  }
  if (my_obb.center.y - my_obb.extents[1] <= arena_hitbox2.arena.ymin) {
    my_phys_c.velocity.y = 0.0f;
    predicted_state_.velocity.y = 0.f;
    server_predicted_.velocity.y = 0.f;
    my_obb.center.y = arena_hitbox2.arena.ymin + my_obb.extents[1];
  }
  
  // collision with walls
  auto view_walls = registry_gameplay_.view<physics::OBB>();
  for (auto wall : view_walls) {
    if (registry_gameplay_.has<PlayerComponent>(wall)) {
      continue;
    }
    auto& hitbox = view_walls.get(wall);
    physics::IntersectData data = Intersect(hitbox, my_obb);
    if (data.collision == true) {
      my_obb.center -= data.move_vector;
      my_phys_c.velocity.y = 0.0f;
      predicted_state_.velocity.y = 0.f;
      server_predicted_.velocity.y = 0.f;
    }
  }
  // collision with ball
  auto view_ball = registry_gameplay_.view<BallComponent, physics::Sphere>();
  for (auto ball : view_ball) {
    auto& hitbox = view_ball.get<physics::Sphere>(ball);
    physics::IntersectData data = Intersect(hitbox, my_obb);
    if (data.collision == true) {
      hitbox.center += data.move_vector;
      my_phys_c.velocity.y = 0.0f;
      predicted_state_.velocity.y = 0.f;
      server_predicted_.velocity.y = 0.f;
    }
  }
  // update positions
  auto view_sphere =
      registry_gameplay_.view<TransformComponent, physics::Sphere>();
  for (auto sphere : view_sphere) {
    auto& transform = view_sphere.get<TransformComponent>(sphere);
    auto& hitbox = view_sphere.get<physics::Sphere>(sphere);

    transform.position = hitbox.center;
  }

  auto view_obb = registry_gameplay_.view<TransformComponent, physics::OBB>();
  for (auto obb : view_obb) {
    auto& transform = view_obb.get<TransformComponent>(obb);
    auto& hitbox = view_obb.get<physics::OBB>(obb);
    // std::cout << transform.position.y << " before\n";

    transform.position = hitbox.center;
    // std::cout << transform.position.y << " after\n";
  }
}

EntityID PlayState::ClientIDToEntityID(long client_id) {
  return engine_->GetPlayerScores()[client_id].enttity_id;
}

void PlayState::DrawTarget() {
  if (my_target_ != -1 &&
      (engine_->GetSecondaryAbility() == AbilityID::MISSILE ||
       my_primary_ability_id == (int)AbilityID::MISSILE)) {
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

    float angles = glm::atan(dir.x, dir.z);
    glm::vec3 cross = glm::normalize(glm::cross(my_forward, dir));

    glob::Submit(e2D_target_, target_pos, 1.0f, glm::degrees(angles) + 180.f,
                 glm::vec3(0, 1, 0));
  }
}

void PlayState::DrawQuickslots() {
  // draw quickslot info
  glob::Submit(gui_quickslots_, glm::vec2(7, 50), 0.3, 100);
  float opacity = 1.0f;
  if (primary_cd_ > 0.0f) {
    opacity = 0.33f;
  }
  glob::Submit(ability_handles_[my_primary_ability_id], glm::vec2(9, 50), 0.75f,
               100, opacity);
  glob::Submit(ability_handles_[(int)engine_->GetSecondaryAbility()],
               glm::vec2(66, 50), 0.75f, 100);
  if (primary_cd_ > 0.0f) {
    std::string cd_string = std::to_string((int)primary_cd_);
    float jump_left = (cd_string.length() - 1) * 16;
    glob::Submit(font_test_, glm::vec2(45 - jump_left, 89), 72, cd_string,
                 glm::vec4(0, 0, 0, 0.7f));
    glob::Submit(font_test_, glm::vec2(46 - jump_left, 90), 72, cd_string);
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
  CreateMapEntity();
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

    //glob::ModelHandle player_model = glob::GetModel(kModelPathMech);

    registry_gameplay_.assign<IDComponent>(entity, entity_id);
    auto& pc = registry_gameplay_.assign<PlayerComponent>(entity);
    registry_gameplay_.assign<TransformComponent>(entity, glm::vec3(0.f),
                                                  glm::quat(), character_scale);

    glob::ModelHandle player_model = glob::GetModel("Assets/Mech/Mech.fbx");
    auto& model_c = registry_gameplay_.assign<ModelComponent>(entity);
    model_c.handles.push_back(player_model);
    model_c.offset = glm::vec3(0.f, 0.9f, 0.f);
    if (engine_->GetPlayerTeam(entity_id) == TEAM_BLUE) {
      model_c.diffuse_index = 1;
    } else {
      model_c.diffuse_index = 0;
    }

    registry_gameplay_.assign<AnimationComponent>(
        entity, glob::GetAnimationData(player_model));
    registry_gameplay_.assign<PhysicsComponent>(entity);
    registry_gameplay_.assign<SoundComponent>(entity,
                                              sound_engine.CreatePlayer());

    if (entity_id == my_id_) {
      glm::vec3 camera_offset = glm::vec3(0.5f, 0.7f, 0.f);
      registry_gameplay_.assign<CameraComponent>(entity, camera_offset,
                                                 glm::quat(glm::vec3(0.f)));
      character_scale = glm::vec3(0.1f);
      float coeff_x_side = (11.223f - (-0.205f));
      float coeff_y_side = (8.159f - (-10.316f));
      float coeff_z_side = (10.206f - (-1.196f));

      registry_gameplay_.assign<physics::OBB>(
          entity,
          alter_scale * character_scale,            // Center
          glm::vec3(1.f, 0.f, 0.f),                 //
          glm::vec3(0.f, 1.f, 0.f),                 // Normals
          glm::vec3(0.f, 0.f, 1.f),                 //
          coeff_x_side * character_scale.x * 0.5f,  //
          coeff_y_side * character_scale.y * 0.5f,  // Length of each plane
          coeff_z_side * character_scale.z * 0.5f   //
      );
      my_entity_ = entity;
    }
  }
}

void PlayState::CreateArenaEntity() {
  auto arena = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  glob::ModelHandle model_arena =
      glob::GetModel("assets/Arena/Map_V3_ARENA.fbx");
  glob::ModelHandle model_arena_banner =
      glob::GetModel("assets/Arena/Map_V3_ARENA_SIGNS.fbx");
  glob::ModelHandle model_map = glob::GetModel("assets/MapV3/Map_Walls.fbx");
  glob::ModelHandle model_map_floor =
      glob::GetModel("assets/MapV3/Map_Floor.fbx");
  glob::ModelHandle model_map_projectors =
      glob::GetModel("assets/MapV3/Map_Projectors.fbx");

    //glob::GetModel(kModelPathMapSingular);
  auto& model_c = registry_gameplay_.assign<ModelComponent>(arena);
  model_c.handles.push_back(model_arena);
  model_c.handles.push_back(model_arena_banner);
  model_c.handles.push_back(model_map);
  model_c.handles.push_back(model_map_floor);
  model_c.handles.push_back(model_map_projectors);

  registry_gameplay_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                arena_scale);
}

void PlayState::CreateMapEntity() {
  auto arena = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(2.6f);
  glob::ModelHandle model_hitbox =
      glob::GetModel("assets/MapV3/Map_Hitbox.fbx");
  glob::ModelHandle model_map_walls =
      glob::GetTransparentModel("assets/MapV3/Map_EnergyWall.fbx");

  auto& model_c = registry_gameplay_.assign<ModelComponent>(arena);
  model_c.handles.push_back(model_map_walls);

  registry_gameplay_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                arena_scale);
  // Prepare hard-coded values
  // Scale on the hitbox for the map
  float v1 = 6.8f * arena_scale.z;
  float v2 = 10.67f * arena_scale.x;  // 13.596f;
  float v3 = 2.723f * arena_scale.y;
  float v4 = 5.723f * arena_scale.y;

  // Add a hitbox
  registry_gameplay_.assign<physics::Arena>(arena, -v2, v2, -v3, v4, -v1, v1);

  auto md = glob::GetMeshData(model_hitbox);
  glm::mat4 matrix =
      glm::rotate(-90.f * glm::pi<float>() / 180.f, glm::vec3(1.f, 0.f, 0.f)) *
      glm::rotate(90.f * glm::pi<float>() / 180.f, glm::vec3(0.f, 0.f, 1.f));

  for (auto& v : md.pos) v = matrix * glm::vec4(v, 1.f);
  for (auto& v : md.pos) v *= arena_scale;
  

   physics::Arena a;
  a.xmax = -1;
  a.xmin = 1;
  a.ymax = -1;
  a.ymin = 1;
  a.zmax = -1;
  a.zmin = 1;
  for (auto& v : md.pos) {
    if (v.x > a.xmax) a.xmax = v.x;
    if (v.x < a.xmin) a.xmin = v.x;
    if (v.y > a.ymax) a.ymax = v.y;
    if (v.y < a.ymin) a.ymin = v.y;
    if (v.z > a.zmax) a.zmax = v.z;
    if (v.z < a.zmin) a.zmin = v.z;
  }

  registry_gameplay_.assign<FailSafeArenaComponent>(arena, a);
  auto& mh = registry_gameplay_.assign<physics::MeshHitbox>(
      arena, std::move(md.pos), std::move(md.indices));
  arena_entity_ = arena;
}

void AddLightToBall(entt::registry& registry, entt::entity& ball) {
  registry.assign<LightComponent>(ball, glm::vec3(0.f, 1.f, 0.f), 20.f, 0.f,
                                  false);
}

void PlayState::CreateBallEntity() {
  auto& sound_engine = engine_->GetSoundEngine();

  // Ball
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  auto ball = registry_gameplay_.create();
  glob::ModelHandle model_ball_projectors_p = glob::GetModel(kModelPathBallProjectors);
  glob::ModelHandle model_ball_sphere_p = glob::GetTransparentModel(kModelPathBallSphere);
  //glob::GetModel("assets/Ball_new/Ball_Comb_tmp.fbx");
  auto& model_c = registry_gameplay_.assign<ModelComponent>(ball);
  model_c.handles.push_back(model_ball_sphere_p);
  model_c.handles.push_back(model_ball_projectors_p);

  registry_gameplay_.assign<TransformComponent>(ball, zero_vec, zero_vec,
                                                glm::vec3(0.95f));
  registry_gameplay_.assign<PhysicsComponent>(ball);
  registry_gameplay_.assign<BallComponent>(ball);
  registry_gameplay_.assign<IDComponent>(ball, ball_id_);
  registry_gameplay_.assign<SoundComponent>(ball, sound_engine.CreatePlayer());
  registry_gameplay_.assign<physics::Sphere>(ball, glm::vec3(0.0f), 1.0f);
  AddLightToBall(registry_gameplay_, ball);

  registry_gameplay_.assign<TrailComponent>(ball);
}

void PlayState::CreateNewBallEntity(bool fake, EntityID id) {
  auto& sound_engine = engine_->GetSoundEngine();

  // Ball
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  auto ball = registry_gameplay_.create();
  glob::ModelHandle model_ball_projectors_p =
      glob::GetModel("Assets/Ball_new/Ball_projectors.fbx");

  std::string model_path = "Assets/Ball_new/Ball_Sphere.fbx";
  if (fake) {
    model_path = "Assets/Ball_new/fake/Ball_Sphere.fbx";
  } else {
    AddLightToBall(registry_gameplay_, ball);
  }
  glob::ModelHandle model_ball_sphere_p = glob::GetTransparentModel(model_path);
  // glob::GetModel("assets/Ball_new/Ball_Comb_tmp.fbx");
  auto& model_c = registry_gameplay_.assign<ModelComponent>(ball);
  model_c.handles.push_back(model_ball_sphere_p);
  model_c.handles.push_back(model_ball_projectors_p);

  registry_gameplay_.assign<TransformComponent>(ball, zero_vec, zero_vec,
                                                glm::vec3(0.95f));
  registry_gameplay_.assign<PhysicsComponent>(ball);
  registry_gameplay_.assign<BallComponent>(ball);
  registry_gameplay_.assign<IDComponent>(ball, id);
  registry_gameplay_.assign<SoundComponent>(ball, sound_engine.CreatePlayer());

  registry_gameplay_.assign<TrailComponent>(ball);
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
      blue_goal_light_, glm::vec3(0.1f, 0.1f, 1.0f), 30.f, 0.0f);
  registry_gameplay_.assign<TransformComponent>(
      blue_goal_light_, glm::vec3(45.f, -8.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  red_goal_light_ = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(
      red_goal_light_, glm::vec3(1.f, 0.1f, 0.1f), 30.f, 0.f);
  registry_gameplay_.assign<TransformComponent>(
      red_goal_light_, glm::vec3(-45.f, -8.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  auto light = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(light, glm::vec3(0.4f, 0.4f, 0.4f),
                                            90.f, 0.2f);
  registry_gameplay_.assign<TransformComponent>(
      light, glm::vec3(0, 4.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f));
}

void PlayState::AddPlayer() {
  player_ids_.clear();
  auto& sound_engine = engine_->GetSoundEngine();
  for (auto entity_id : player_ids_) {
    auto entity = registry_gameplay_.create();

    glm::vec3 alter_scale =
        glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f);
    glm::vec3 character_scale = glm::vec3(0.0033f);

    registry_gameplay_.assign<IDComponent>(entity, entity_id);
    auto& pc = registry_gameplay_.assign<PlayerComponent>(entity);
    registry_gameplay_.assign<TransformComponent>(entity, glm::vec3(0.f),
                                                  glm::quat(), character_scale);

    glob::ModelHandle player_model = glob::GetModel("Assets/Mech/Mech.fbx");
    auto& model_c = registry_gameplay_.assign<ModelComponent>(entity);
    model_c.handles.push_back(player_model);
    model_c.offset = glm::vec3(0.f, 0.9f, 0.f);
    if (engine_->GetPlayerTeam(entity_id) == TEAM_BLUE) {
      model_c.diffuse_index = 1;
    } else {
      model_c.diffuse_index = 0;
    }

    registry_gameplay_.assign<AnimationComponent>(
        entity, glob::GetAnimationData(player_model));
    registry_gameplay_.assign<PhysicsComponent>(entity);
    registry_gameplay_.assign<SoundComponent>(entity,
                                              sound_engine.CreatePlayer());

    if (entity_id == my_id_) {
      glm::vec3 camera_offset = glm::vec3(0.5f, 0.7f, 0.f);
      registry_gameplay_.assign<CameraComponent>(entity, camera_offset,
                                                 glm::quat(glm::vec3(0.f)));
      character_scale = glm::vec3(0.1f);
      float coeff_x_side = (11.223f - (-0.205f));
      float coeff_y_side = (8.159f - (-10.316f));
      float coeff_z_side = (10.206f - (-1.196f));
      registry_gameplay_.assign<physics::OBB>(
          entity,
          alter_scale * character_scale,            // Center
          glm::vec3(1.f, 0.f, 0.f),                 //
          glm::vec3(0.f, 1.f, 0.f),                 // Normals
          glm::vec3(0.f, 0.f, 1.f),                 //
          coeff_x_side * character_scale.x * 0.5f,  //
          coeff_y_side * character_scale.y * 0.5f,  // Length of each plane
          coeff_z_side * character_scale.z * 0.5f   //
      );
      my_entity_ = entity;
    }
  }
}

void PlayState::CreateWall(EntityID id, glm::vec3 position,
                           glm::quat rotation) {
  auto wall = registry_gameplay_.create();
  auto& sound_engine = engine_->GetSoundEngine();
  registry_gameplay_.assign<SoundComponent>(wall, sound_engine.CreatePlayer());
  registry_gameplay_.assign<IDComponent>(wall, id);
  registry_gameplay_.assign<TransformComponent>(wall, position, rotation,
                                                glm::vec3(1.f, 4.f, 5.f));
  auto& obb = registry_gameplay_.assign<physics::OBB>(wall);
  obb.extents[0] = 1.f;
  obb.extents[1] = 8.3f;
  obb.extents[2] = 5.f;

  glob::ModelHandle model = glob::GetModel("assets/Pickup/Pickup.fbx");
  int a = 10;
  std::vector<glob::ModelHandle> hs;
  hs.push_back(model);
  registry_gameplay_.assign<ModelComponent>(wall, hs);
  registry_gameplay_.assign<WallComponent>(wall);

  GameEvent wall_event;
  wall_event.type = GameEvent::BUILD_WALL;
  wall_event.build_wall.wall_id = id;
  dispatcher.trigger(wall_event);
}

void PlayState::CreatePickUp(EntityID id, glm::vec3 position) {
  auto pick_up = registry_gameplay_.create();

  registry_gameplay_.assign<IDComponent>(pick_up, id);

  glob::ModelHandle model_pick_up =
    glob::GetModel(kModelPathPickup);
  auto& model_c = registry_gameplay_.assign<ModelComponent>(pick_up);
  model_c.handles.push_back(model_pick_up);

  registry_gameplay_.assign<TransformComponent>(
      pick_up, position, glm::vec3(0.0f, 0.0f, 0.f), glm::vec3(0.4f));
  registry_gameplay_.assign<PickUpComponent>(pick_up);
  auto& snd = registry_gameplay_.assign<SoundComponent>(pick_up);
  snd.sound_player = engine_->GetSoundEngine().CreatePlayer();

  GameEvent e;
  e.pickup_spawned.pickup_id = id;
  e.type = GameEvent::PICKUP_SPAWNED;
  dispatcher.trigger(e);
}

void PlayState::CreateCannonBall(EntityID id, glm::vec3 pos, glm::quat ori) {
  auto cannonball = registry_gameplay_.create();

  glob::ModelHandle model_shot = glob::GetModel(kModelPathRocket);
  auto& model_c = registry_gameplay_.assign<ModelComponent>(cannonball);
  model_c.handles.push_back(model_shot);

  registry_gameplay_.assign<TransformComponent>(cannonball, pos, ori,
                                                glm::vec3(0.3f));
  registry_gameplay_.assign<ProjectileComponent>(cannonball, ProjectileID::CANNON_BALL);
  registry_gameplay_.assign<IDComponent>(cannonball, id);
}

void PlayState::CreateTeleportProjectile(EntityID id, glm::vec3 pos,
                                         glm::quat ori) {
  auto teleport_projectile = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);

  registry_gameplay_.assign<TrailComponent>(teleport_projectile, 0.5f,
                                            glm::vec4(1, 1, 1, 1));
  registry_gameplay_.assign<TransformComponent>(teleport_projectile, pos, ori,
                                                glm::vec3(0.3f));
  registry_gameplay_.assign<ProjectileComponent>(teleport_projectile,
                                                 ProjectileID::TELEPORT_PROJECTILE);
  registry_gameplay_.assign<IDComponent>(teleport_projectile, id);
}

void PlayState::CreateForcePushObject(EntityID id, glm::vec3 pos,
                                      glm::quat ori) {
  auto& sound_engine = engine_->GetSoundEngine();

  auto force_object = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glob::ModelHandle model_ball = glob::GetModel(kModelPathBall);
  //"assets/Ball/force_push_ball.fbx"	TODO: Swap path to this one
  auto& model_c = registry_gameplay_.assign<ModelComponent>(force_object);
  model_c.handles.push_back(model_ball);

  registry_gameplay_.assign<TransformComponent>(force_object, pos, ori,
                                                glm::vec3(0.5f));
  registry_gameplay_.assign<IDComponent>(force_object, id);
  registry_gameplay_.assign<SoundComponent>(force_object,
                                            sound_engine.CreatePlayer());
  registry_gameplay_.assign<TrailComponent>(force_object, 1.f,
                                            glm::vec4(0.4, 0.4, 1, 1));
}

void PlayState::CreateMissileObject(EntityID id, glm::vec3 pos, glm::quat ori) {
  auto& sound_engine = engine_->GetSoundEngine();

  auto missile_object = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glob::ModelHandle model_missile = glob::GetModel(kModelPathRocket);
  auto& model_c = registry_gameplay_.assign<ModelComponent>(missile_object);
  model_c.handles.push_back(model_missile);
  registry_gameplay_.assign<TransformComponent>(missile_object, pos, ori,
                                                glm::vec3(0.5f));
  registry_gameplay_.assign<IDComponent>(missile_object, id);
  registry_gameplay_.assign<SoundComponent>(missile_object,
                                            sound_engine.CreatePlayer());
  registry_gameplay_.assign<ProjectileComponent>(missile_object, ProjectileID::MISSILE_OBJECT);
  registry_gameplay_.assign<TrailComponent>(missile_object, 0.2f,
                                            glm::vec4(1.0f, 0.6f, 0.2f, 1.0f));
}

void PlayState::DestroyEntity(EntityID id) {
  auto id_view = registry_gameplay_.view<IDComponent>();

  glm::vec3 pos(0);
  bool was_ball = false;
  bool was_wall = false;

  for (auto entity : id_view) {
    auto& e_id = id_view.get(entity);

    if (e_id.id == id) {
      if (registry_gameplay_.has<BallComponent>(entity)) {
        was_ball = true;

        if (registry_gameplay_.has<SoundComponent>(entity)) {
          auto& sound_c = registry_gameplay_.get<SoundComponent>(entity);
          auto ent = registry_gameplay_.create();

          GameEvent ge;
          ge.type = GameEvent::FAKE_BALL_POOF;
          ge.fake_ball_poofed.ball_id = id;
          dispatcher.trigger(ge);
        }
      }
      if (registry_gameplay_.has<WallComponent>(entity)) {
        was_wall = true;
      }
      if (registry_gameplay_.has<TransformComponent>(entity)) {
        pos = registry_gameplay_.get<TransformComponent>(entity).position;
      }
      registry_gameplay_.destroy(entity);
      break;
    }
  }

  if (was_ball || was_wall) {
    auto e = registry_gameplay_.create();
    auto handle = glob::CreateParticleSystem();

    std::vector<glob::ParticleSystemHandle> handles;
    std::vector<glm::vec3> offsets;
    //= {glm::vec3(0.f)};
    std::vector<glm::vec3> directions;
    //= {glm::vec3(0.f, 1.f, 0.f)};

    glob::SetParticleSettings(handle, "ball_destroy.txt");
    glob::SetEmitPosition(handle, pos);
    handles.push_back(handle);

    // auto ball_view = registry_gameplay_.view<

    auto& p_c = registry_gameplay_.assign<ParticleComponent>(
        e, handles, offsets, directions);
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

void PlayState::SetPlayerLookDir(EntityID id, glm::vec3 look_dir) {
  player_look_dirs_[id] = look_dir;
}

void PlayState::SetPlayerMoveDir(EntityID id, glm::vec3 move_dir) {
  player_move_dirs_[id] = move_dir;
}

void PlayState::ReceiveGameEvent(const GameEvent& e) {
  switch (e.type) {
    case GameEvent::GOAL: {
      CreateGoalParticles(e.goal.x);
      break;
    }
    case GameEvent::RESET: {
      Reset();
      break;
    }
    case GameEvent::PRIMARY_USED: {
      if (e.primary_used.player_id == my_id_) {
        primary_cd_ = e.primary_used.cd;
      }
      break;
    }
    case GameEvent::SECONDARY_USED: {
      if (e.primary_used.player_id == my_id_) {
        engine_->SetSecondaryAbility(AbilityID::NULL_ABILITY);
      }
      break;
    }
    case GameEvent::INVISIBILITY_CAST: {
      auto registry = engine_->GetCurrentRegistry();
      auto view_controller =
          registry->view<IDComponent, PlayerComponent, ModelComponent,
                         TransformComponent>();
      for (auto entity : view_controller) {
        IDComponent& id_c = view_controller.get<IDComponent>(entity);
        PlayerComponent& p_c = view_controller.get<PlayerComponent>(entity);
        ModelComponent& m_c = view_controller.get<ModelComponent>(entity);
        TransformComponent& t_c =
            view_controller.get<TransformComponent>(entity);

        if (id_c.id == e.invisibility_cast.player_id) {
          m_c.invisible = true;

          // Particles
          entt::entity particle_entity = registry_gameplay_.create();
          glob::ParticleSystemHandle handle = glob::CreateParticleSystem();
          std::vector<glob::ParticleSystemHandle> in_handles;
          std::vector<glm::vec3> in_offsets;
          std::vector<glm::vec3> in_directions;

          glob::SetParticleSettings(handle, "ball_destroy.txt");
          glob::SetEmitPosition(handle, t_c.position);
          in_handles.push_back(handle);

          ParticleComponent& par_c =
              registry_gameplay_.assign<ParticleComponent>(
                  particle_entity, in_handles, in_offsets, in_directions);
          registry_gameplay_.assign<int>(particle_entity, 0);

          if (e.invisibility_cast.player_id == my_id_) {
            // TODO: Add effect to let player know it's invisible
            glob::SetInvisibleEffect(m_c.invisible);
          }
        }
      }
      break;
    }
    case GameEvent::INVISIBILITY_END: {
      auto registry = engine_->GetCurrentRegistry();
      auto view_controller =
          registry->view<IDComponent, PlayerComponent, ModelComponent,
                         TransformComponent>();
      for (auto entity : view_controller) {
        IDComponent& id_c = view_controller.get<IDComponent>(entity);
        PlayerComponent& p_c = view_controller.get<PlayerComponent>(entity);
        ModelComponent& m_c = view_controller.get<ModelComponent>(entity);
        TransformComponent& t_c =
            view_controller.get<TransformComponent>(entity);

        if (id_c.id == e.invisibility_end.player_id) {
          m_c.invisible = false;

          // Particles
          entt::entity particle_entity = registry_gameplay_.create();
          glob::ParticleSystemHandle handle = glob::CreateParticleSystem();
          std::vector<glob::ParticleSystemHandle> in_handles;
          std::vector<glm::vec3> in_offsets;
          std::vector<glm::vec3> in_directions;

          glob::SetParticleSettings(handle, "ball_destroy.txt");
          glob::SetEmitPosition(handle, t_c.position);
          in_handles.push_back(handle);

          ParticleComponent& par_c =
              registry_gameplay_.assign<ParticleComponent>(
                  particle_entity, in_handles, in_offsets, in_directions);
          registry_gameplay_.assign<int>(particle_entity, 0);

          if (e.invisibility_end.player_id == my_id_) {
            // TODO: Remove effect to let player know it's visible again
            glob::SetInvisibleEffect(m_c.invisible);
          }
        }
      }
      break;
    }
    case GameEvent::FORCE_PUSH_IMPACT: {
      auto ent = registry_gameplay_.create();
      auto handle = glob::CreateParticleSystem();

      std::vector handles = {handle};
      std::vector<glm::vec3> offsets;
      std::vector<glm::vec3> directions;

      glob::SetParticleSettings(handle, "force_push.txt");

      auto registry = engine_->GetCurrentRegistry();
      auto view_controller = registry->view<IDComponent, TransformComponent>();

      for (auto proj_ent : view_controller) {
        auto& id_c = view_controller.get<IDComponent>(proj_ent);
        auto& trans_c = view_controller.get<TransformComponent>(proj_ent);

        if (id_c.id == e.force_push_impact.projectile_id) {
          glob::SetEmitPosition(handle, trans_c.position);
          break;
        }
      }

      registry_gameplay_.assign<ParticleComponent>(ent, handles, offsets,
                                                   directions);
      registry_gameplay_.assign<int>(ent, 0);
      break;
    }
    case GameEvent::MISSILE_IMPACT: {
      auto ent = registry_gameplay_.create();
      auto handle = glob::CreateParticleSystem();

      std::vector handles = {handle};
      std::vector<glm::vec3> offsets;
      std::vector<glm::vec3> directions;

      glob::SetParticleSettings(handle, "missile_impact.txt");

      auto registry = engine_->GetCurrentRegistry();
      auto view_controller = registry->view<IDComponent, TransformComponent>();

      for (auto proj_ent : view_controller) {
        auto& id_c = view_controller.get<IDComponent>(proj_ent);
        auto& trans_c = view_controller.get<TransformComponent>(proj_ent);

        if (id_c.id == e.force_push_impact.projectile_id) {
          glob::SetEmitPosition(handle, trans_c.position);
          break;
        }
      }

      registry_gameplay_.assign<ParticleComponent>(ent, handles, offsets,
                                                   directions);
      registry_gameplay_.assign<int>(ent, 0);
      break;
    }
    case GameEvent::TELEPORT_CAST: {
      auto registry = engine_->GetCurrentRegistry();
      auto view_controller =
          registry->view<IDComponent, PlayerComponent, TransformComponent>();
      for (auto entity : view_controller) {
        IDComponent& id_c = view_controller.get<IDComponent>(entity);
        PlayerComponent& p_c = view_controller.get<PlayerComponent>(entity);
        TransformComponent& t_c =
            view_controller.get<TransformComponent>(entity);

        if (id_c.id == e.teleport_cast.player_id) {
          // Particles
          entt::entity particle_entity = registry_gameplay_.create();
          glob::ParticleSystemHandle handle = glob::CreateParticleSystem();
          std::vector<glob::ParticleSystemHandle> in_handles;
          std::vector<glm::vec3> in_offsets;
          std::vector<glm::vec3> in_directions;
          glob::SetParticleSettings(handle, "teleport.txt");
          glob::SetEmitPosition(handle, t_c.position);
          in_handles.push_back(handle);
          ParticleComponent& par_c =
              registry_gameplay_.assign<ParticleComponent>(
                  particle_entity, in_handles, in_offsets, in_directions);
          registry_gameplay_.assign<int>(particle_entity, 0);

          break;
        }
      }
      break;
    }
    case GameEvent::TELEPORT_IMPACT: {
      auto registry = engine_->GetCurrentRegistry();
      auto view_controller =
          registry->view<IDComponent, PlayerComponent, TransformComponent>();
      for (auto entity : view_controller) {
        IDComponent& id_c = view_controller.get<IDComponent>(entity);
        PlayerComponent& p_c = view_controller.get<PlayerComponent>(entity);
        TransformComponent& t_c =
            view_controller.get<TransformComponent>(entity);

        if (id_c.id == e.teleport_impact.player_id) {
          // Particles
          entt::entity particle_entity = registry_gameplay_.create();
          glob::ParticleSystemHandle handle = glob::CreateParticleSystem();
          std::vector<glob::ParticleSystemHandle> in_handles;
          std::vector<glm::vec3> in_offsets;
          std::vector<glm::vec3> in_directions;
          glob::SetParticleSettings(handle, "teleport.txt");
          glob::SetEmitPosition(handle, e.teleport_impact.hit_pos);
          in_handles.push_back(handle);
          ParticleComponent& par_c =
              registry_gameplay_.assign<ParticleComponent>(
                  particle_entity, in_handles, in_offsets, in_directions);
          registry_gameplay_.assign<int>(particle_entity, 0);
          break;
        }
      }
      break;
    }
    case GameEvent::HOMING_BALL: {
      auto registry = engine_->GetCurrentRegistry();
      auto view_controller =
          registry->view<BallComponent, TrailComponent, IDComponent>();
      for (auto entity : view_controller) {
        BallComponent& ball_c = view_controller.get<BallComponent>(entity);
        TrailComponent& trail_c = view_controller.get<TrailComponent>(entity);
        IDComponent& id_c = view_controller.get<IDComponent>(entity);

        if (id_c.id == e.homing_ball.ball_id) {
          trail_c.color = glm::vec4(0.74f, 0.19f, 1.0f, 1.0f);
          break;
        }
      }
      break;
    }
    case GameEvent::HOMING_BALL_END: {
      auto registry = engine_->GetCurrentRegistry();
      auto view_controller =
          registry->view<BallComponent, TrailComponent, IDComponent>();
      for (auto entity : view_controller) {
        BallComponent& ball_c = view_controller.get<BallComponent>(entity);
        TrailComponent& trail_c = view_controller.get<TrailComponent>(entity);
        IDComponent& id_c = view_controller.get<IDComponent>(entity);

        if (id_c.id == e.homing_ball_end.ball_id) {
          trail_c.color = glm::vec4(0, 1, 0, 1);
          break;
        }
      }
      break;
    }
    case GameEvent::GRAVITY_DROP: {
      auto entity = registry_gameplay_.create();
      auto handle = glob::CreateParticleSystem();

      std::vector handles = {handle};
      std::vector<glm::vec3> offsets;
      std::vector<glm::vec3> directions;
      glob::SetParticleSettings(handle, "dust.txt");

      registry_gameplay_.assign<ParticleComponent>(entity, handles, offsets,
                                                   directions);
      registry_gameplay_.assign<int>(entity, 0);
      break;
    }
    case GameEvent::BLACKOUT_TRIGGER: {
      glob::SetBlackout(true);
      auto registry = engine_->GetCurrentRegistry();
      auto view_controller = registry->view<LightComponent>();
      for (auto entity : view_controller) {
        LightComponent& light_c = view_controller.get(entity);

        // Turn off all light sources affected by blackout
        if (!registry->has<BallComponent>(entity) &&
            (entity != red_goal_light_ && entity != blue_goal_light_)) {
          light_c.blackout = true;
        }
      }
      break;
    }
    case GameEvent::BLACKOUT_END: {
      glob::SetBlackout(false);
      auto registry = engine_->GetCurrentRegistry();
      auto view_controller = registry->view<LightComponent>();
      for (auto entity : view_controller) {
        LightComponent& light_c = view_controller.get(entity);

        light_c.blackout = false;
      }
    }
    case GameEvent::SUPER_KICK: {
      auto registry = engine_->GetCurrentRegistry();
      auto view_controller =
          registry->view<IDComponent, PlayerComponent, TransformComponent>();
      for (auto entity : view_controller) {
        IDComponent& id_c = view_controller.get<IDComponent>(entity);
        PlayerComponent& p_c = view_controller.get<PlayerComponent>(entity);
        TransformComponent& t_c =
            view_controller.get<TransformComponent>(entity);

        if (id_c.id == e.super_kick.player_id) {
          // Particles
          entt::entity particle_entity = registry_gameplay_.create();
          glob::ParticleSystemHandle handle = glob::CreateParticleSystem();
          std::vector<glob::ParticleSystemHandle> in_handles;
          std::vector<glm::vec3> in_offsets;
          std::vector<glm::vec3> in_directions;
          glob::SetParticleSettings(handle, "superkick.txt");
          glob::SetParticleDirection(handle, t_c.Forward());
          glob::SetEmitPosition(handle, t_c.position);
          in_handles.push_back(handle);
          ParticleComponent& par_c =
              registry_gameplay_.assign<ParticleComponent>(
                  particle_entity, in_handles, in_offsets, in_directions);
          registry_gameplay_.assign<int>(particle_entity, 0);

          break;
        }
      }
    }
    case GameEvent::SPRINT_START: {
      sprinting_ = true;
      break;
    }
    case GameEvent::SPRINT_END: {
      sprinting_ = false;
      break;
    }
  }
}

void PlayState::Reset() {
  auto view_particle = registry_gameplay_.view<ParticleComponent>();
  for (auto& entity : view_particle) {
                                      auto& particle_c =
                                          view_particle.get(entity);

                                      for (int i = 0;
                                           i < particle_c.handles.size(); ++i) {
                                        glob::ResetParticles(
                                            particle_c.handles[i]);
                                      }
                                    }

                                    auto view_delete =
                                        registry_gameplay_
                                            .view<ParticleComponent, int>();
                                    for (auto& entity : view_delete) {
                                      auto& particle_c =
                                          view_delete.get<ParticleComponent>(
                                              entity);

                                      for (int i = 0;
                                           i < particle_c.handles.size(); ++i) {
                                        glob::DestroyParticleSystem(
                                            particle_c.handles[i]);
                                      }

    registry_gameplay_.destroy(entity);
  }
  if ((my_team_ == TEAM_BLUE && goals_swapped_ == false) ||
      (my_team_ == TEAM_RED && goals_swapped_ == true)) {
    yaw_ = glm::pi<float>();
  } else {
    yaw_ = 0.0f;
  }
  pitch_ = 0.0f;

  auto& player_c = registry_gameplay_.get<PlayerComponent>(my_entity_);
  player_c.can_jump = false;
  server_predicted_.velocity = glm::vec3(0.0f);
  predicted_state_.velocity = glm::vec3(0.0f);
}

  void PlayState::EndGame() {
    end_game_timer_.Restart();
    game_has_ended_ = true;
  }

  void PlayState::OverTime() { overtime_has_started_ = true; }

  void PlayState::AddPitchYaw(float pitch, float yaw) {
    pitch_ += pitch;
    yaw_ += yaw;
    constexpr float pi = glm::pi<float>();
    pitch_ = glm::clamp(pitch_, -0.49f * pi, 0.49f * pi);
  }

  void PlayState::SetPitchYaw(float pitch, float yaw) {
    pitch_ = pitch;
    yaw_ = yaw;
    constexpr float pi = glm::pi<float>();
    pitch_ = glm::clamp(pitch_, -0.49f * pi, 0.49f * pi);
  }