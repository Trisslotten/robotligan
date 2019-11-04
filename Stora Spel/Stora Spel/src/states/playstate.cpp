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
#include <physics.hpp>
#include <shared/physics_component.hpp>
#include <shared/pick_up_component.hpp>
#include "ecs/components.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"
#include "eventdispatcher.hpp"
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
  gui_minimap_goal_red_ =
      glob::GetGUIItem("assets/GUI_elements/goal_red_icon.png");
  gui_minimap_goal_blue_ =
      glob::GetGUIItem("assets/GUI_elements/goal_blue_icon.png");
  gui_minimap_player_red_ =
      glob::GetGUIItem("assets/GUI_elements/player_iconv2_red.png");
  gui_minimap_player_blue_ =
      glob::GetGUIItem("assets/GUI_elements/player_iconv2_blue.png");
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

  test_ball_ = glob::GetTransparentModel("Assets/Ball_new/Ball_Sphere.fbx");
}

void PlayState::TestParticles() {
  auto e = registry_gameplay_.create();
  auto handle = glob::CreateParticleSystem();

  std::vector handles = {handle};
  std::vector<glm::vec3> offsets;
  //= {glm::vec3(0.f)};
  std::vector<glm::vec3> directions;
  //= {glm::vec3(0.f, 1.f, 0.f)};

  glob::SetParticleSettings(handle, "green_donut.txt");
  glob::SetEmitPosition(handle, glm::vec3(30.f, 0.f, 0.f));

  e = registry_gameplay_.create();
  handle = glob::CreateParticleSystem();
  handles.push_back(handle);
  glob::SetParticleSettings(handle, "green_donut.txt");
  glob::SetEmitPosition(handle, glm::vec3(-30.f, 0.f, 0.f));
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
    MovePlayer(1 / 64.0f);
    actions_.clear();
  }
  timer_ += dt;
  if (timer_ > 1.0f / 64.0f) {
    MoveBall(dt);
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
      auto& cam_c = registry_gameplay_.get<CameraComponent>(my_entity_);
      glm::vec3 temp =
          lerp(predicted_state_.position, server_predicted_.position, 0.5f);
      trans_c.position = glm::lerp(trans_c.position, temp, 0.2f);
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

  // draw stamina bar
  glob::Submit(gui_stamina_base_, glm::vec2(0, 5), 0.85, 100);
  glob::Submit(gui_stamina_fill_, glm::vec2(7, 12), 0.85, current_stamina_);
  glob::Submit(gui_stamina_icon_, glm::vec2(0, 5), 0.85, 100);

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

void PlayState::DrawNameOverPlayer() {
  auto player_view =
      registry_gameplay_
          .view<PlayerComponent, TransformComponent, IDComponent>();

  auto& my_transform = registry_gameplay_.get<TransformComponent>(my_entity_);
  for (auto entity : player_view) {
    if (my_entity_ == entity) {
      continue;
    }

    auto& id_c = player_view.get<IDComponent>(entity);
    auto& transform = player_view.get<TransformComponent>(entity);

    for (auto& [id, name] : engine_->player_names_) {
      EntityID e_id = ClientIDToEntityID(id);
      if (e_id == id_c.id) {
        glm::vec3 look = glm::vec3(transform.position - my_transform.position);
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
}

void PlayState::UpdateSwitchGoalTimer() {
  // Countdown timer
  int temp_time = engine_->GetSwitchGoalTime();
  int count = engine_->GetSwitchGoalCountdownTimer();

  // Start countdown
  if (count == 1) {
    countdown_in_progress_ = true;
  }

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
    if (temp_time - count <= 0) {
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
      if (a == PlayerAction::SPRINT && current_stamina_ > 60.0f * dt) {
        sprint = true;
      }
      if (a == PlayerAction::JUMP && !state.is_airborne) {
        // Add velocity upwards
        final_velocity += up * 6.0f;
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

  for (auto& frame : history_) {
    for (auto a : frame.actions) {
      if (a == PlayerAction::JUMP) {
        return;
      }
    }
  }
  // predicted_state_.is_airborne = false;
  // predicted_state_.velocity.y = 0.0f;
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
  auto& arena_hitbox = registry_gameplay_.get<physics::Arena>(arena_entity_);

  physics::IntersectData data = Intersect(arena_hitbox, my_obb);
  if (data.collision == true) {
    my_obb.center += data.move_vector;
    if (data.move_vector.y > 0.0f) {
      // my_phys_c.is_airborne = false;
      predicted_state_.is_airborne = false;
      predicted_state_.velocity.y = 0.f;
    } else if (data.move_vector.y < 0.0f) {
      my_phys_c.velocity.y = 0.f;
    }
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
    }
  }
  // collision with ball
  auto view_ball = registry_gameplay_.view<BallComponent, physics::Sphere>();
  for (auto ball : view_ball) {
    auto& hitbox = view_ball.get<physics::Sphere>(ball);
    physics::IntersectData data = Intersect(hitbox, my_obb);
    if (data.collision == true) {
      hitbox.center += data.move_vector;
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

    registry_gameplay_.assign<IDComponent>(entity, entity_id);
    auto& pc = registry_gameplay_.assign<PlayerComponent>(entity);
    registry_gameplay_.assign<TransformComponent>(entity, glm::vec3(0.f),
                                                  glm::quat(), character_scale);

    glob::ModelHandle player_model = glob::GetModel("Assets/Mech/Mech.fbx");
    auto& model_c = registry_gameplay_.assign<ModelComponent>(entity);
    model_c.handles.push_back(player_model);
    model_c.offset = glm::vec3(0.f, 0.9f, 0.f);
    if (engine_->GetPlayerTeam(entity_id) == TEAM_BLUE) {
      model_c.material_index = 1;
    } else {
      model_c.material_index = 0;
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
  glm::vec3 arena_scale = glm::vec3(4.0f, 4.0f, 4.0f);
  glob::ModelHandle model_arena =
      glob::GetModel("assets/Map/Map_singular_TMP.fbx");
  auto& model_c = registry_gameplay_.assign<ModelComponent>(arena);
  model_c.handles.push_back(model_arena);

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
  arena_entity_ = arena;
}

void AddLightToBall(entt::registry& registry, entt::entity& ball) {
  registry.assign<LightComponent>(ball, glm::vec3(0.f, 1.f, 0.f),
                                            20.f, 0.f);
}

void PlayState::CreateBallEntity() {
  auto& sound_engine = engine_->GetSoundEngine();

  // Ball
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  auto ball = registry_gameplay_.create();
  glob::ModelHandle model_ball_projectors_p =
      glob::GetModel("Assets/Ball_new/Ball_projectors.fbx");
  glob::ModelHandle model_ball_sphere_p =
      glob::GetTransparentModel("Assets/Ball_new/Ball_Sphere.fbx");
  // glob::GetModel("assets/Ball_new/Ball_Comb_tmp.fbx");
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
      blue_goal_light_, glm::vec3(48.f, -6.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  red_goal_light_ = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(
      red_goal_light_, glm::vec3(1.f, 0.1f, 0.1f), 30.f, 0.f);
  registry_gameplay_.assign<TransformComponent>(
      red_goal_light_, glm::vec3(-48.f, -6.f, 0.f), glm::vec3(0.f, 0.f, 1.f),
      glm::vec3(1.f));

  auto light = registry_gameplay_.create();
  registry_gameplay_.assign<LightComponent>(light, glm::vec3(0.4f, 0.4f, 0.4f),
                                            90.f, 0.1f);
  registry_gameplay_.assign<TransformComponent>(
      light, glm::vec3(0, 4.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f));
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

  GameEvent wall_event;
  wall_event.type = GameEvent::BUILD_WALL;
  wall_event.build_wall.wall_id = id;
  dispatcher.trigger(wall_event);
}

void PlayState::CreatePickUp(EntityID id, glm::vec3 position) {
  auto pick_up = registry_gameplay_.create();

  registry_gameplay_.assign<IDComponent>(pick_up, id);

  glob::ModelHandle model_pick_up = glob::GetModel("assets/Pickup/Pickup.fbx");
  auto& model_c = registry_gameplay_.assign<ModelComponent>(pick_up);
  model_c.handles.push_back(model_pick_up);

  registry_gameplay_.assign<TransformComponent>(
      pick_up, position, glm::vec3(0.0f, 0.0f, 0.f), glm::vec3(0.4f));
  registry_gameplay_.assign<PickUpComponent>(pick_up);
}

void PlayState::CreateCannonBall(EntityID id) {
  auto cannonball = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);

  glob::ModelHandle model_ball = glob::GetModel("assets/Rocket/Rocket.fbx");
  auto& model_c = registry_gameplay_.assign<ModelComponent>(cannonball);
  model_c.handles.push_back(model_ball);

  registry_gameplay_.assign<TransformComponent>(cannonball, zero_vec, zero_vec,
                                                glm::vec3(0.3f));
  registry_gameplay_.assign<IDComponent>(cannonball, id);
}

void PlayState::CreateTeleportProjectile(EntityID id) {
  auto teleport_projectile = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);

  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  auto& model_c =
      registry_gameplay_.assign<ModelComponent>(teleport_projectile);
  model_c.handles.push_back(model_ball);

  registry_gameplay_.assign<TransformComponent>(teleport_projectile, zero_vec,
                                                zero_vec, glm::vec3(0.3f));
  registry_gameplay_.assign<IDComponent>(teleport_projectile, id);
}

void PlayState::CreateForcePushObject(EntityID id) {
  auto& sound_engine = engine_->GetSoundEngine();

  auto force_object = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  auto& model_c = registry_gameplay_.assign<ModelComponent>(force_object);
  model_c.handles.push_back(model_ball);

  registry_gameplay_.assign<TransformComponent>(force_object, zero_vec,
                                                zero_vec, glm::vec3(0.5f));
  registry_gameplay_.assign<IDComponent>(force_object, id);
  registry_gameplay_.assign<SoundComponent>(force_object,
                                            sound_engine.CreatePlayer());
}

void PlayState::CreateMissileObject(EntityID id) {
  auto& sound_engine = engine_->GetSoundEngine();

  auto missile_object = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glob::ModelHandle model_ball = glob::GetModel("assets/Rocket/Rocket.fbx");
  auto& model_c = registry_gameplay_.assign<ModelComponent>(missile_object);
  model_c.handles.push_back(model_ball);
  registry_gameplay_.assign<TransformComponent>(missile_object, zero_vec,
                                                zero_vec, glm::vec3(0.5f));
  registry_gameplay_.assign<IDComponent>(missile_object, id);
  registry_gameplay_.assign<SoundComponent>(missile_object,
                                            sound_engine.CreatePlayer());
}

void PlayState::DestroyEntity(EntityID id) {
  auto id_view = registry_gameplay_.view<IDComponent>();

  glm::vec3 pos(0);
  bool was_ball = false;

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
      if (registry_gameplay_.has<TransformComponent>(entity)) {
        pos = registry_gameplay_.get<TransformComponent>(entity).position;
      }
      registry_gameplay_.destroy(entity);
      break;
    }
  }

  if (was_ball) {
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
    registry_gameplay_.assign<int>(e, 0);
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
  }
}

void PlayState::Reset() {
  auto view_particle = registry_gameplay_.view<ParticleComponent>();
  for (auto& entity : view_particle) {
    auto& particle_c = view_particle.get(entity);

    for (int i = 0; i < particle_c.handles.size(); ++i) {
      glob::ResetParticles(particle_c.handles[i]);
    }
  }

  auto view_delete = registry_gameplay_.view<ParticleComponent, int>();
  for (auto& entity : view_delete) {
    auto& particle_c = view_delete.get<ParticleComponent>(entity);

    for (int i = 0; i < particle_c.handles.size(); ++i) {
      glob::DestroyParticleSystem(particle_c.handles[i]);
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
  predicted_state_.is_airborne = true;
  predicted_state_.velocity = glm::vec3(0.0f);
}

void PlayState::EndGame() {
  end_game_timer_.Restart();
  game_has_ended_ = true;
}

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