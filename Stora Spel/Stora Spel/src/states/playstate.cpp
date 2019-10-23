#include "state.hpp"

#include <GLFW/glfw3.h>
#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include <slob/sound_engine.hpp>

#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "shared/camera_component.hpp"
#include "shared/id_component.hpp"
#include "shared/transform_component.hpp"

#include <physics.hpp>
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
  Reset();
}

void PlayState::Update(float dt) {
  auto& cli = engine_->GetClient();
  if (!cli.IsConnected()) {
    cli.Disconnect();
    engine_->ChangeState(StateType::MAIN_MENU);
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
    // physics_.clear();  //?
  }

  bool move_player = false;
  if (!transforms_.empty()) {
    auto view_entities =
        registry_gameplay_.view<TransformComponent, IDComponent>();
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
    move_player = true;
  }

  auto view_entities =
      registry_gameplay_.view<TransformComponent, IDComponent>();
  if (move_player == true) {
    MovePlayer(1 / 64.0f);
    actions_.clear();
  }

  // interpolate
  float f = 0.5;  // 0.25f * dt;  // pow(0.75f, dt);
  for (auto entity : view_entities) {
    auto& trans_c = view_entities.get<TransformComponent>(entity);
    auto& id_c = view_entities.get<IDComponent>(entity);
    if (id_c.id == my_id_) {
      // auto trans = new_transforms_[id_c.id];
      auto& cam_c = registry_gameplay_.get<CameraComponent>(my_entity_);
      glm::vec3 temp =
          lerp(predicted_state_.position, server_predicted_.position, 0.5f);
      trans_c.position = glm::lerp(trans_c.position, temp, 0.2f);

      // glm::quat pred_rot = glm::quat(glm::vec3(0, predicted_state_.yaw, 0)) *
      //                     glm::quat(glm::vec3(0, 0, predicted_state_.pitch));
      // pred_rot = glm::normalize(pred_rot);
      //
      // glm::quat serv_rot = glm::quat(glm::vec3(0, server_predicted_.yaw, 0))
      // *
      //                     glm::quat(glm::vec3(0, 0,
      //                     server_predicted_.pitch));
      // serv_rot = glm::normalize(serv_rot);
      //
      // glm::quat temp_rot = glm::slerp(pred_rot, serv_rot, 0.5f);
      //
      // cam_c.orientation = glm::slerp(cam_c.orientation, temp_rot, 0.3f);

      glm::quat orientation =
          glm::quat(glm::vec3(0, yaw_, 0)) * glm::quat(glm::vec3(0, 0, pitch_));
      orientation = glm::normalize(orientation);
      cam_c.orientation = orientation;
      trans_c.rotation = glm::quat(glm::vec3(0, yaw_, 0));
    } else {
      auto trans = new_transforms_[id_c.id];
      if (glm::length(trans_c.position - trans.first) > 10) {
        trans_c.position = trans.first;
      } else {
        trans_c.position = glm::lerp(trans_c.position, trans.first, 1.0f - f);
        // trans_c.position = trans.first;
      }
      trans_c.rotation = glm::slerp(trans_c.rotation, trans.second, 1.0f - f);
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
  // glob::Submit(e2D_test_, glm::vec3(10.5f, 1.0f, 0.0f), 2, -90.0f,
  // glm::vec3(0, 1, 0));
  // glob::Submit(e2D_test_, glm::vec3(-10.5f, 1.0f, 0.0f), 2, 90.0f,
  // glm::vec3(0, 1, 0));
  glob::Submit(e2D_test2_, glm::vec3(0.0f, 1.0f, -28.0f), 7, 0.0f,
               glm::vec3(1));

  UpdateGameplayTimer();

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

  predicted_state_.is_airborne = false;
  predicted_state_.velocity.y = 0.f;
}

/*void PlayState::MoveBall(float dt) {
  auto view_ball =
      registry_gameplay_
          .view<BallComponent, TransformComponent, PhysicsComponent>();

  for (auto ball : view_ball) {
    auto& phys_c = view_ball.get<PhysicsComponent>(ball);
    auto& trans_c = view_ball.get<TransformComponent>(ball);
    
        physics::PhysicsObject po;
    po.acceleration =  ;

  }
}*/

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

    float angles = glm::atan(dir.x, dir.z);
    glm::vec3 cross = glm::normalize(glm::cross(my_forward, dir));

    glob::Submit(e2D_target_, target_pos, 1.0f, glm::degrees(angles) + 180.f,
                 glm::vec3(0, 1, 0));
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
    auto& model_c = registry_gameplay_.assign<ModelComponent>(entity);
    model_c.handles.push_back(player_model);
    model_c.offset = glm::vec3(0.f, 0.9f, 0.f);

    registry_gameplay_.assign<AnimationComponent>(
        entity, glob::GetAnimationData(player_model));
    registry_gameplay_.assign<PhysicsComponent>(entity);
    registry_gameplay_.assign<SoundComponent>(entity,
                                              sound_engine.CreatePlayer());

    if (entity_id == my_id_) {
      glm::vec3 camera_offset = glm::vec3(0.5f, 0.7f, 0.f);
      registry_gameplay_.assign<CameraComponent>(entity, camera_offset,
                                                 glm::quat(glm::vec3(0.f)));
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

  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
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
  auto force_object = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  auto& model_c = registry_gameplay_.assign<ModelComponent>(force_object);
  model_c.handles.push_back(model_ball);

  registry_gameplay_.assign<TransformComponent>(force_object, zero_vec,
                                                zero_vec, glm::vec3(0.5f));
  registry_gameplay_.assign<IDComponent>(force_object, id);
}

void PlayState::CreateMissileObject(EntityID id) {
  auto missile_object = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glob::ModelHandle model_ball = glob::GetModel("assets/Rocket/Rocket.fbx");
  auto& model_c = registry_gameplay_.assign<ModelComponent>(missile_object);
  model_c.handles.push_back(model_ball);
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

void PlayState::ReceiveGameEvent(const GameEvent& e) {
  switch (e.type) {
    case GameEvent::RESET: {
      Reset();
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