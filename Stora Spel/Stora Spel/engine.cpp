#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <iostream>

#include <render_system.hpp>
#include "Components/ball_component.hpp"
#include "Components/light_component.hpp"
#include "Components/player_component.hpp"
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

  CreateInitalEntities();

  SetKeybinds();

  client.Connect("192.168.1.47", 1337);
}

void Engine::CreateInitalEntities() {
  auto arena = registry_.create();
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  glob::ModelHandle model_arena =
      glob::GetModel("assets/Map_rectangular/map_rextangular.fbx");
  registry_.assign<ModelComponent>(arena, model_arena);
  registry_.assign<TransformComponent>(arena, zero_vec, zero_vec, arena_scale);

  auto ball = registry_.create();
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");
  registry_.assign<ModelComponent>(ball, model_ball);
  registry_.assign<TransformComponent>(ball, zero_vec, zero_vec,
                                       glm::vec3(1.0f));
  registry_.assign<BallComponent>(ball);
}

void Engine::Update(float dt) {
  Input::Reset();

  for (auto const& [key, action] : keybinds_)
    if (Input::IsKeyDown(key)) key_presses_[key]++;
  for (auto const& [button, action] : mousebinds_)
    if (Input::IsMouseButtonDown(button)) mouse_presses_[button]++;

  float mouse_sensitivity = 0.003f;
  glm::vec2 mouse_movement = mouse_sensitivity * Input::MouseMov();
  accum_yaw_ -= mouse_movement.x;
  accum_pitch_ -= mouse_movement.y;

  UpdateSystems(dt);
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
    for (auto& packet : client.Receive()) {
      while (!packet.IsEmpty()) {
        HandlePacketBlock(packet);
      }
    }

    if (!transforms.empty()) {
      auto view_players = registry_.view<TransformComponent, PlayerComponent>();
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
      packet >> entity_id;
      packet >> player_id;
      std::cout << "PACKET: CREATE_PLAYER, player_id=" << player_id
                << ", entity_id=" << entity_id << "\n";
      CreatePlayer(player_id, entity_id);
      break;
    }
    case PacketBlockType::SET_CLIENT_PLAYER_ID: {
      packet >> my_id;
      auto view = registry_.view<const PlayerComponent>();
      for (auto& player : view) {
        auto& player_c = view.get(player);
        if (player_c.id == my_id) {
          glm::vec3 camera_offset = glm::vec3(0.38f, 0.62f, -0.06f);
          registry_.assign<CameraComponent>(player, camera_offset);
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
      packet.Remove(str.data(), strsize);
      std::cout << "PACKET: TEST_STRING: '" << str << "'\n";
    }
    case PacketBlockType::BALL_TRANSFORM: {
      registry_.view<TransformComponent, BallComponent>().each(
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
      registry_.view<CameraComponent>().each(
          [&](auto entity, CameraComponent& cam_c) {
            cam_c.orientation = orientation;
          });
      break;
    }
  }
}

void Engine::Render() {
  glob::Submit(font_test_, glm::vec2(100, 200), 73,
               "Det här är Comic Sans MS jahoo!", glm::vec4(0, 0, 0, 0.5));
  glob::Submit(font_test_, glm::vec2(98, 202), 73,
               "Det här är Comic Sans MS jahoo!", glm::vec4(1, 1, 1, 1));

  glob::Render();
}

void Engine::UpdateSystems(float dt) {
  // collision_debug::Update(*reg);

  // player_controller::Update(registry_, dt);
  // ability_controller::Update(registry_, dt);

  // UpdatePhysics(registry_, dt);
  // UpdateCollisions(registry_);

  /*
  auto view = registry_.view<CameraComponent, TransformComponent>();
  for (auto v : view) {
    auto& cam_c = registry_.get<CameraComponent>(v);
    auto& trans_c = registry_.get<TransformComponent>(v);
    cam_c.cam->SetPosition(trans_c.position +
                           glm::rotate(cam_c.offset, -trans_c.rotation.y,
                                       glm::vec3(0.0f, 1.0f, 0.0f)));
  }
  */

  RenderSystem(registry_);
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
  auto entity = registry_.create();

  glm::vec3 alter_scale =
      glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f);
  glm::vec3 character_scale = glm::vec3(0.1f);

  glob::ModelHandle player_model =
      glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

  registry_.assign<IDComponent>(entity, entity_id);
  registry_.assign<PlayerComponent>(entity, player_id);
  registry_.assign<TransformComponent>(entity, glm::vec3(), glm::quat(),
                                       character_scale);
  registry_.assign<ModelComponent>(entity, player_model,
                                   alter_scale * character_scale);
}

void Engine::TestCreateLights() {
  // Create light
  auto light = registry_.create();
  registry_.assign<LightComponent>(light, glm::vec3(0.3f, 0.3f, 1.0f), 15.f,
                                   0.2f);
  registry_.assign<TransformComponent>(light, glm::vec3(12.f, -4.f, 0.f),
                                       glm::vec3(0.f, 0.f, 1.f),
                                       glm::vec3(1.f));

  light = registry_.create();
  registry_.assign<LightComponent>(light, glm::vec3(1.f, 0.3f, 0.3f), 15.f,
                                   0.f);
  registry_.assign<TransformComponent>(light, glm::vec3(-12.f, -4.f, 0.f),
                                       glm::vec3(0.f, 0.f, 1.f),
                                       glm::vec3(1.f));
}