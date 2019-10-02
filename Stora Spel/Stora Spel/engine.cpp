#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <iostream>

#include <button_system.hpp>
#include <glob\window.hpp>
#include <render_system.hpp>
#include "Components/ball_component.hpp"
#include "Components/light_component.hpp"
#include "Components/player_component.hpp"
#include "entitycreation.hpp"
#include "shared/camera_component.hpp"
#include "shared/id_component.hpp"
#include "shared/transform_component.hpp"
#include "util/global_settings.hpp"
#include "util/input.hpp"

Engine::Engine() {}

Engine::~Engine() {}

void Engine::Init() {
  glob::Init();
  Input::Initialize();

  // Tell the GlobalSettings class to do a first read from the settings file
  GlobalSettings::Access()->UpdateValuesFromFile();

  TestCreateLights();

  font_test_ = glob::GetFont("assets/fonts/fonts/comic.ttf");
  glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

  ///////////////////////////////////////////////////////////////
  // TO BE MOVED
  ///////////////////////////////////////////////////////////////
  font_test2_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");
  font_test3_ = glob::GetFont("assets/fonts/fonts/OCRAEXT_2.TTF");
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

  CreateInGameMenu();
  //registry_current_ = &registry_mainmenu_;

  // CreateInitalEntities();

  SetKeybinds();

  main_menu_state_.Init(*this);
  current_state_ = &main_menu_state_;
  wanted_state_type_ = StateType::MAIN_MENU;

  // client.Connect("localhost", 1337);
}

void Engine::CreateInitalEntities() {
  auto arena = registry_gameplay_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  glob::ModelHandle model_arena =
      glob::GetModel("assets/Map_rectangular/map_rextangular.fbx");
  registry_gameplay_.assign<ModelComponent>(arena, model_arena);
  registry_gameplay_.assign<TransformComponent>(arena, zero_vec, zero_vec,
                                                arena_scale);

  auto ball = registry_gameplay_.create();
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  registry_gameplay_.assign<ModelComponent>(ball, model_ball);
  registry_gameplay_.assign<TransformComponent>(ball, zero_vec, zero_vec,
                                                glm::vec3(1.0f));
  registry_gameplay_.assign<BallComponent>(ball);
}

void Engine::Update(float dt) {
  Input::Reset();

  // accumulate key presses
  for (auto const& [key, action] : keybinds_)
    if (Input::IsKeyDown(key)) key_presses_[key]++;
  for (auto const& [button, action] : mousebinds_)
    if (Input::IsMouseButtonDown(button)) mouse_presses_[button]++;

  // accumulate mouse movement
  float mouse_sensitivity = 0.003f;
  glm::vec2 mouse_movement = mouse_sensitivity * Input::MouseMov();
  accum_yaw_ -= mouse_movement.x;
  accum_pitch_ -= mouse_movement.y;

  current_state_->Update(*this);

  UpdateSystems(dt);

  // check if state changed
  if (wanted_state_type_ != current_state_->Type()) {
    // cleanup old state
    current_state_->Cleanup(*this);

    // set new state
    switch (wanted_state_type_) {
      case StateType::MAIN_MENU:
        current_state_ = &main_menu_state_;
        break;
      case StateType::LOBBY:
        current_state_ = &lobby_state_;
        break;
      case StateType::PLAY:
        current_state_ = &play_state_;
        break;
    }

    // init new state
    current_state_->Init(*this);
  }
}

void Engine::UpdateNetwork() {
  {
    // get and send player input
    std::bitset<PlayerAction::NUM_ACTIONS> actions;
    for (auto const& [key, action] : keybinds_) {
      auto& presses = key_presses_[key];
      if (presses > 0) actions.set(action, true);
      presses = 0;
    }
    for (auto const& [button, action] : mousebinds_) {
      auto& presses = mouse_presses_[button];
      if (presses > 0) actions.set(action, true);
      presses = 0;
    }

    uint16_t action_bits = actions.to_ulong();

    NetAPI::Common::Packet packet;

    packet << action_bits;
    packet << accum_pitch_;
    packet << accum_yaw_;
    packet << PacketBlockType::INPUT;

    if (actions[PlayerAction::KICK]) {
      packet << PacketBlockType::CLIENT_READY;
    } else if (actions[PlayerAction::SHOOT]) {
      packet << PacketBlockType::CLIENT_NOT_READY;
    }

    if (client.IsConnected()) {
      client.Send(packet);
    } else {
      // TODO: go to main menu or something
    }
    accum_yaw_ = 0.f;
    accum_pitch_ = 0.f;
  }

  {
    // handle received data
    auto packets = client.Receive();
    // std::cout <<"Num recevied packets: "<< packets.size() << "\n";
    for (auto& packet : packets) {
      while (!packet.IsEmpty()) {
        // std::cout << "Remaining packet size: " << packet.GetPacketSize() <<
        // "\n";
        HandlePacketBlock(packet);
      }
    }

    if (!transforms.empty()) {
      auto view_players =
          registry_gameplay_.view<TransformComponent, PlayerComponent>();
      for (auto player : view_players) {
        auto& trans_c = view_players.get<TransformComponent>(player);
        auto& player_c = view_players.get<PlayerComponent>(player);
        auto trans = transforms[player_c.id];
        trans_c.position = trans.first;
        trans_c.rotation = trans.second;
        /*
        std::cout << trans_c.position.x << ", ";
        std::cout << trans_c.position.y << ", ";
        std::cout << trans_c.position.z << "\n";
        */
      }
      // std::cout << "\n";
      transforms.clear();
    }
  }
}

void Engine::HandlePacketBlock(NetAPI::Common::Packet& packet) {
  int16_t block_type = -1;
  packet >> block_type;
  switch (block_type) {
    case PacketBlockType::CREATE_PLAYER: {
      PlayerID player_id = -1;
      EntityID entity_id = -1;
      // packet >> entity_id;
      packet >> player_id;
      std::cout << "PACKET: CREATE_PLAYER, player_id=" << player_id
                << ", entity_id=" << entity_id << "\n";
      CreatePlayer(player_id, entity_id);
      break;
    }
    case PacketBlockType::SET_CLIENT_PLAYER_ID: {
      packet >> my_id;
      auto view = registry_gameplay_.view<const PlayerComponent>();
      for (auto& player : view) {
        auto& player_c = view.get(player);
        if (player_c.id == my_id) {
          glm::vec3 camera_offset = glm::vec3(0.38f, 0.62f, -0.06f);
          registry_gameplay_.assign<CameraComponent>(player, camera_offset);
          break;
        }
      }
      std::cout << "PACKET: SET_CLIENT_PLAYER_ID id=" << my_id << "\n";
      break;
    }
    case PacketBlockType::TEST_STRING: {
      size_t strsize = 0;
      packet >> strsize;
      std::string str;
      str.resize(strsize);
      std::cout << "Packet Size: " << packet.GetPacketSize() << "\n";
      packet.Remove(str.data(), strsize);
      std::cout << "PACKET: TEST_STRING: '" << str << "'\n";
      break;
    }
    case PacketBlockType::BALL_TRANSFORM: {
      registry_gameplay_.view<TransformComponent, BallComponent>().each(
          [&](auto entity, TransformComponent& trans_c, auto ball) {
            packet >> trans_c.position;
            packet >> trans_c.rotation;
          });
      // std::cout << "Ball pos.y=" << ball_pos.y << "\n";
      break;
    }
    case PacketBlockType::PLAYERS_TRANSFORMS: {
      int size = -1;
      packet >> size;
      for (int i = 0; i < size; i++) {
        PlayerID id;
        glm::vec3 position;
        glm::quat orientation;
        packet >> id;
        packet >> position;
        packet >> orientation;
        transforms[id] = std::make_pair(position, orientation);
      }
      break;
    }
    case PacketBlockType::CAMERA_TRANSFORM: {
      glm::quat orientation;
      packet >> orientation;
      registry_gameplay_.view<CameraComponent>().each(
          [&](auto entity, CameraComponent& cam_c) {
            cam_c.orientation = orientation;
          });
      break;
    }
    case PacketBlockType::GAME_START: {
      SetCurrentRegistry(&registry_gameplay_);
      break;
    }
  }
}

void Engine::Render() { glob::Render(); }

void Engine::SetCurrentRegistry(entt::registry* registry) {
  this->registry_current_ = registry;
}

void Engine::UpdateSystems(float dt) {
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
  if (show_in_game_menu_buttons_) {
    glob::Submit(in_game_menu_gui_, glm::vec2(491, 152), 1.0f);
  }
  // Submit 2D Element TEST
  glob::Submit(e2D_test_, glm::vec3(10.5f, 1.0f, 0.0f), 2, -90.0f,
               glm::vec3(0, 1, 0));
  glob::Submit(e2D_test_, glm::vec3(-10.5f, 1.0f, 0.0f), 2, 90.0f,
               glm::vec3(0, 1, 0));
  glob::Submit(e2D_test2_, glm::vec3(0.0f, 1.0f, -7.0f), 7, 0.0f, glm::vec3(1));

  // Show statistics TEST
  if (Input::IsKeyDown(GLFW_KEY_TAB)) {
    glob::Submit(gui_test_, glm::vec2(285, 177), 0.6, 100);
  }

  // Show GUI TEST
  // Temp Update of stamina bar
  float stam_len = 0.0f;
  /*
  registry_gameplay_.view<PlayerComponent>().each(
      [&](auto entity, PlayerComponent& player_c) {
        stam_len = player_c.energy_current;
      });
          */
  glob::Submit(gui_stamina_base_, glm::vec2(0, 5), 0.85, 100);
  glob::Submit(gui_stamina_fill_, glm::vec2(7, 12), 0.85, stam_len);
  glob::Submit(gui_stamina_icon_, glm::vec2(0, 5), 0.85, 100);
  glob::Submit(gui_quickslots_, glm::vec2(7, 50), 0.3, 100);
  glob::Submit(gui_teamscore_, glm::vec2(497, 648), 1, 100);

  button_system::Update(*registry_current_);

  RenderSystem(*registry_current_);
}

void Engine::SetKeybinds() {
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

void Engine::CreatePlayer(PlayerID player_id, EntityID entity_id) {
  auto entity = registry_gameplay_.create();

  glm::vec3 alter_scale =
      glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f);
  glm::vec3 character_scale = glm::vec3(0.1f);

  glob::ModelHandle player_model =
      glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

  registry_gameplay_.assign<IDComponent>(entity, entity_id);
  registry_gameplay_.assign<PlayerComponent>(entity, player_id);
  registry_gameplay_.assign<TransformComponent>(entity, glm::vec3(),
                                                glm::quat(), character_scale);
  registry_gameplay_.assign<ModelComponent>(entity, player_model,
                                            alter_scale * character_scale);
}

void Engine::TestCreateLights() {
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
}


void Engine::UpdateInGameMenu(bool show_menu) {
  // Set in_game buttons visibility
  auto view = registry_gameplay_.view<ButtonComponent, TransformComponent>();
  for (auto v : view) {
    auto& button_c = registry_gameplay_.get<ButtonComponent>(v);
    button_c.visible = show_menu;
  }
}
