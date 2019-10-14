#include "state.hpp"

#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "shared/camera_component.hpp"
#include "shared/id_component.hpp"
#include "shared/transform_component.hpp"

#include <shared\pick_up_component.hpp>
#include "ecs/components.hpp"
#include "engine.hpp"
#include "entitycreation.hpp"
#include "util/input.hpp"
#include <physics.hpp>
#include "..//..//..//Server/src/ecs/components/physics_component.hpp"

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

void PlayState::Update(float dt) {
  auto& cli = engine_->GetClient();
  if (!cli.IsConnected()) {
    cli.Disconnect();
    engine_->ChangeState(StateType::MAIN_MENU);
  }
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
    // std::cout << "\n";
    transforms_.clear();
  }
  
  // interpolate
  auto view_entities =
      registry_gameplay_.view<TransformComponent, IDComponent>();

  float f = 0.25f * dt;  // pow(0.75f, dt);
  for (auto entity : view_entities) {
    auto& trans_c = view_entities.get<TransformComponent>(entity);
    auto& id_c = view_entities.get<IDComponent>(entity);
    if (id_c.id == my_id_) {
      auto trans = new_transforms_[id_c.id];
      player_new_pos_ = trans.first;
      player_new_rotation_ = trans.second;
      //if (glm::length(trans_c.position - trans.first) > 3.0f) {
		//trans_c.position = glm::lerp(trans_c.position, trans.first, 1.0f - f);
        //trans_c.position = trans.first;
        //std::cout << "rubber banding\n"; 
	  //}
      trans_c.position = trans.first;
      trans_c.rotation = trans.second;
    } else {
		auto trans = new_transforms_[id_c.id];
		trans_c.position = glm::lerp(trans_c.position, trans.first, 1.0f - f);
		trans_c.rotation = glm::slerp(trans_c.rotation, trans.second, 1.0f - f);  
	}
  }
 
  MovePlayer(dt);

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

FrameState PlayState::SimulateMovement(std::vector<int>* action, float dt) {
  auto view_controller = registry_gameplay_.view<CameraComponent, PlayerComponent, TransformComponent>();

  glm::vec3 final_velocity = glm::vec3(0.f, 0.f, 0.f);
  for (auto entity : view_controller) {
    CameraComponent& cam_c = view_controller.get<CameraComponent>(entity);
    // PlayerComponent& player_c = view_controller.get<PlayerComponent>(entity);
    TransformComponent& trans_c =
        view_controller.get<TransformComponent>(entity);
    // PhysicsComponent& physics_c =
    // view_controller.get<PhysicsComponent>(entity);

    // Caputre keyboard input and apply velocity

    glm::vec3 accum_velocity = glm::vec3(0.f);

    // base movement direction on camera orientation.
    glm::vec3 frwd = cam_c.GetLookDir();
    frwd.y = 0;
    frwd = glm::normalize(frwd);
    glm::vec3 up(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(frwd, up));
    bool sprint = false;
    for (int a : *action) {
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
    // if (player_c.actions[PlayerAction::JUMP] && !physics_c.is_airborne &&
    //    player_c.energy_current > player_c.cost_jump && !player_c.no_clip) {
    //  // Add velocity upwards
    //  final_velocity += up * player_c.jump_speed;
    //  // Set them to be airborne
    //  physics_c.is_airborne = true;
    //  // Subtract energy cost from resources
    //  player_c.energy_current -= player_c.cost_jump;
    //}

    // slowdown
    glm::vec3 sidemov = glm::vec3(final_velocity.x, 0, final_velocity.z);
    float cur_move_speed = glm::length(sidemov);
    // if (cur_move_speed > 0.f) {
    // movement "floatiness", lower value = less floaty
    float t = 0.0005f;
    predicted_state_.velocity.x = glm::mix(
        predicted_state_.velocity.x, final_velocity.x, 1.f - glm::pow(t, dt));
    predicted_state_.velocity.z = glm::mix(
        predicted_state_.velocity.z, final_velocity.z, 1.f - glm::pow(t, dt));
    predicted_state_.velocity.y = final_velocity.y;
    physics::PhysicsObject po;
    po.acceleration = glm::vec3(0.f);
    po.airborne = false;
    po.friction = 0.f;
    po.max_speed = 1000;
    po.position = predicted_state_.position;
    po.velocity = predicted_state_.velocity;

    physics::Update(&po, dt);
    FrameState new_state;
    new_state.velocity = po.velocity;
    new_state.position = po.position;
    return new_state;
  }
}

void PlayState::MovePlayer(float dt) {
  auto view_controller =
      registry_gameplay_
          .view<CameraComponent, PlayerComponent, TransformComponent>();

  PlayerData new_frame;
  new_frame.delta_time = dt;

  for (auto const& [key, action] : *keybinds_) {
    if (action == PlayerAction::WALK_FORWARD && Input::IsKeyDown(key)) {
      new_frame.actions.push_back(PlayerAction::WALK_FORWARD);
    }
    if (action == PlayerAction::WALK_BACKWARD && Input::IsKeyDown(key)) {
      new_frame.actions.push_back(PlayerAction::WALK_BACKWARD);
    }
    if (action == PlayerAction::WALK_RIGHT && Input::IsKeyDown(key)) {
      new_frame.actions.push_back(PlayerAction::WALK_RIGHT);
    }
    if (action == PlayerAction::WALK_LEFT && Input::IsKeyDown(key)) {
      new_frame.actions.push_back(PlayerAction::WALK_LEFT);
    }
    if (action == PlayerAction::SPRINT && current_stamina_ > 60.0f * dt) {
      new_frame.actions.push_back(PlayerAction::SPRINT);
    }
  }
  FrameState new_state = SimulateMovement(&new_frame.actions, dt);
    new_frame.delta_pos = new_state.position - predicted_state_.position;
    new_frame.velocity = new_state.velocity;

	history_.push_back(new_frame);
    history_duration_ += dt;

	//float converge_multiplier = 0.05f;
	//
	//glm::vec3 extrapolated_position = predicted_state_.position + po.velocity * LATENCY * converge_multiplier;
	//
	//t = dt / (LATENCY * (1 + converge_multiplier));
	//
	//trans_c.position = (extrapolated_position - trans_c.position) * t;
    auto& trans_c = registry_gameplay_.get<TransformComponent>(my_entity_);
    trans_c.position = new_state.position;
    predicted_state_ = new_state;
}

void PlayState::OnServerFrame() { 
	float dt = 0;
  if (history_duration_ - LATENCY > 0) {
    dt = history_duration_ - LATENCY;
  }
  history_duration_ -= dt;
  while (history_.size() > 0 && dt > 0) {
    if (dt >= history_.front().delta_time) {
        dt -= history_.front().delta_time;
        history_.pop_front();
    } else {
        float t = 1 - dt / history_.front().delta_time;
        history_.front().delta_time -= dt;
        history_.front().delta_pos *= dt;
        break;
	}
  }
  glm::vec3 velocity = history_.front().velocity;
  if (registry_gameplay_.has<PhysicsComponent>(my_entity_)) {
    auto& phys_c = registry_gameplay_.get<PhysicsComponent>(my_entity_);
    velocity = phys_c.velocity;
  }

  float velocity_tolerance = 0.3f;
  if (glm::length(velocity - history_.front().velocity) > velocity_tolerance) {
    //auto& trans_c = registry_gameplay_.get<TransformComponent>(my_entity_);
    predicted_state_.position = player_new_pos_;
    predicted_state_.rotation = player_new_rotation_;
    predicted_state_.velocity = velocity;

	for (auto& frame : history_) {
      FrameState new_state = SimulateMovement(&frame.actions, frame.delta_time);
      frame.delta_pos = new_state.position - predicted_state_.position;
      frame.velocity = predicted_state_.velocity;
      predicted_state_ = new_state;
	}
  } else {
    auto& trans_c = registry_gameplay_.get<TransformComponent>(my_entity_);
    predicted_state_.position = player_new_pos_;
    predicted_state_.rotation = player_new_rotation_;

	for (auto& frame : history_) {
      predicted_state_.position += frame.delta_pos;
      predicted_state_.rotation = frame.rotation;
	}
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
      my_entity_ = entity;
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
