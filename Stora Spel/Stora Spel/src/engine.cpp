#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <iostream>

#include <glob\window.hpp>
#include "ecs/components.hpp"
#include "ecs/systems/button_system.hpp"
#include "ecs/systems/render_system.hpp"
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

  // glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

  SetKeybinds();

  main_menu_state_.SetEngine(this);
  lobby_state_.SetEngine(this);
  play_state_.SetEngine(this);

  main_menu_state_.Startup();
  lobby_state_.Startup();
  play_state_.Startup();

  main_menu_state_.Init();
  current_state_ = &main_menu_state_;
  wanted_state_type_ = StateType::MAIN_MENU;
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

  current_state_->Update();

  UpdateSystems(dt);

  // check if state changed
  if (wanted_state_type_ != current_state_->Type()) {
    // cleanup old state
    current_state_->Cleanup();

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
    current_state_->Init();
  }
}

void Engine::UpdateNetwork() {
  current_state_->UpdateNetwork();

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

  NetAPI::Common::Packet to_send;

  if(!packet_.IsEmpty()) {
      to_send << packet_;
  }

  if(should_send_input_) {
    to_send << action_bits;
    to_send << accum_pitch_;
    to_send << accum_yaw_;
    to_send << PacketBlockType::INPUT;
  }
  if (client_.IsConnected() && !to_send.IsEmpty()) {
    client_.Send(to_send);
  }
  accum_yaw_ = 0.f;
  accum_pitch_ = 0.f;
  packet_ = NetAPI::Common::Packet();

  // handle received data
  if (client_.IsConnected()) {
    auto packets = client_.Receive();
    // std::cout <<"Num recevied packets: "<< packets.size() << "\n";
    for (auto& packet : packets) {
      while (!packet.IsEmpty()) {
        // std::cout << "Remaining packet size: " << packet.GetPacketSize() <<
        // "\n";
        HandlePacketBlock(packet);
      }
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
      // CreatePlayer(player_id, entity_id);
      break;
    }
    /*
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
    */
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
    case PacketBlockType::ENTITY_TRANSFORMS: {
      int size = -1;
      packet >> size;
      for (int i = 0; i < size; i++) {
        EntityID id;
        glm::vec3 position;
        glm::quat orientation;
        packet >> id;
        packet >> position;
        packet >> orientation;
        play_state_.AddSetEntityTransform(id, position, orientation);
      }
      break;
    }
    case PacketBlockType::CAMERA_TRANSFORM: {
      glm::quat orientation;
      packet >> orientation;
      play_state_.SetCameraOrientation(orientation);
      break;
    }
    case PacketBlockType::GAME_START: {
      std::cout << "PACKET: GAME_START\n";
      ChangeState(StateType::PLAY);
      break;
    }
  }
}

void Engine::Render() { glob::Render(); }

void Engine::SetCurrentRegistry(entt::registry* registry) {
  this->registry_current_ = registry;
}

void Engine::UpdateSystems(float dt) {
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
