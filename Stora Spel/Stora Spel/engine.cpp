#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <bitset>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <iostream>

#include <ability_controller_system.hpp>
#include <collision_system.hpp>
#include <physics_system.hpp>
#include <player_controller_system.hpp>
#include <render_system.hpp>
#include "Components/light_component.hpp"
#include "Components/transform_component.hpp"
#include "entitycreation.hpp"
#include "util/global_settings.hpp"
#include "util/input.hpp"

void Engine::Init() {
  glob::Init();
  Input::Initialize();

  tcp_client_.Connect("192.168.1.47", 1337);

  // Tell the GlobalSettings class to do a first read from the settings file
  GlobalSettings::Access()->UpdateValuesFromFile();

  glm::vec3 start_positions[3] = {
      glm::vec3(5.f, 0.f, 0.f),   // Ball
      glm::vec3(-9.f, 4.f, 0.f),  // Player
      glm::vec3(0.f, 0.f, 0.f)    // Others
  };
  CreateEntities(registry_, start_positions, 3);

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
  font_test2_ = glob::GetFont("assets/fonts/fonts/ariblk.ttf");

  keybinds_[GLFW_KEY_UP]    = PlayerAction::WALK_FORWARD;
  keybinds_[GLFW_KEY_DOWN]  = PlayerAction::WALK_BACKWARD;
  keybinds_[GLFW_KEY_LEFT]  = PlayerAction::WALK_LEFT;
  keybinds_[GLFW_KEY_RIGHT] = PlayerAction::WALK_RIGHT;
}

void Engine::Update(float dt) {
  Input::Reset();

  UpdateSystems(dt);
}

void Engine::UpdateNetwork() {
  std::bitset<PlayerAction::NUM_ACTIONS> actions;
  for (auto const& [key, action] : keybinds_) {
    if (Input::IsKeyDown(key)) {
      actions.set(action, true);
    }
  }

  byte action_bits = actions.to_ulong();

  NetAPI::Common::Packet packet;
  packet << action_bits;

  tcp_client_.Send(packet);
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

  player_controller::Update(registry_, dt);
  ability_controller::Update(registry_, dt);

  UpdatePhysics(registry_, dt);
  UpdateCollisions(registry_);

  auto view = registry_.view<CameraComponent, TransformComponent>();
  for (auto v : view) {
    auto& cam_c = registry_.get<CameraComponent>(v);
    auto& trans_c = registry_.get<TransformComponent>(v);
    cam_c.cam->SetPosition(trans_c.position +
                           glm::rotate(cam_c.offset, -trans_c.rotation.y,
                                       glm::vec3(0.0f, 1.0f, 0.0f)));
  }

  RenderSystem(registry_);
}
