#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <iostream>

#include <render_system.hpp>
#include "Components/light_component.hpp"
#include "shared/camera_component.hpp"
#include "shared/transform_component.hpp"
#include "util/global_settings.hpp"
#include "util/input.hpp"

Engine::Engine() {}

Engine::~Engine() {}

void Engine::Init() {
  glob::Init();
  Input::Initialize();

  client.Connect("localhost", 1337);

  // Tell the GlobalSettings class to do a first read from the settings file
  GlobalSettings::Access()->UpdateValuesFromFile();

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

  font_test_ = glob::GetFont("assets/fonts/fonts/comic.ttf");

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

  for (auto const& [key, action] : keybinds_)
    if (Input::IsKeyDown(key)) key_presses_[key]++;
  for (auto const& [button, action] : mousebinds_)
    if (Input::IsMouseButtonDown(button)) mouse_presses_[button]++;

  glm::vec2 mouse_movement = Input::MouseMov();
  accum_yaw += mouse_movement.x;
  accum_pitch += mouse_movement.y;

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
    packet << accum_pitch;
    packet << accum_yaw;
    packet << PacketBlockType::INPUT;

    client.Send(packet);

	accum_yaw = 0.f;
    accum_pitch = 0.f;
  }

  {
    // handle received data
    auto packet = client.Receive();
    while (!packet.IsEmpty()) {
      HandlePacketBlock(packet);
    }
  }
}

void Engine::HandlePacketBlock(NetAPI::Common::Packet& packet) {
  int16_t block_type = -1;
  packet >> block_type;
  size_t strsize = 0;
  switch (block_type) {
    case PacketBlockType::CREATE_PLAYER: {
      PlayerID id = -1;
      packet >> id;
      //std::cout << "PACKET: CREATE_PLAYER, id=" << id << "\n";
      CreatePlayer(id);
      break;
    }
    case PacketBlockType::SET_CLIENT_PLAYER_ID: {
      packet >> my_id;
      std::cout << "PACKET: SET_CLIENT_PLAYER_ID id=" << my_id << "\n";
      break;
    }
    case PacketBlockType::TEST_STRING: {
      packet >> strsize;
      std::string str;
      str.resize(strsize);
      packet.Remove(str.data(), strsize);
      //std::cout << "PACKET: TEST_STRING: '" << str << "'\n";
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

  auto view = registry_.view<CameraComponent, TransformComponent>();
  for (auto v : view) {
    auto& cam_c = registry_.get<CameraComponent>(v);
    auto& trans_c = registry_.get<TransformComponent>(v);
    /*
    cam_c.cam->SetPosition(trans_c.position +
                           glm::rotate(cam_c.offset, -trans_c.rotation.y,
                                       glm::vec3(0.0f, 1.0f, 0.0f)));
    */
  }

  RenderSystem(registry_);
}

void Engine::CreatePlayer(PlayerID id) {
  auto entity = registry_.create();
  registry_.assign<TransformComponent>(entity);

  glob::ModelHandle player_model =
      glob::GetModel("Assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

  registry_.assign<ModelComponent>(entity, player_model);
}
