#include <iostream>
#include <entt.hpp>
#include <NetAPI/networkTest.hpp>
#include <NetAPI/socket/server.hpp>
#include <NetAPI/socket/tcpclient.hpp>
#include <glm/gtx/transform.hpp>
#include <glob/graphics.hpp>
#include <glob/window.hpp>
#include <entity/registry.hpp>

#include <entity/registry.hpp>
#include "ability_controller_system.hpp"
#include "ball_component.hpp"
#include "collision.hpp"
#include "ability_component.hpp"
#include "collision_system.hpp"
#include "model_component.hpp"
#include "physics_system.hpp"
#include "player_controller_system.hpp"
#include "print_position_system.hpp"
#include "render_system.hpp"
#include "transform_component.hpp"

#include "collision_temp_debug_system.h"
#include <GLFW/glfw3.h>
#include "util/input.hpp"
#include "util/meminfo.hpp"
#include "util/timer.hpp"
#include "util/meminfo.hpp"

#include <thread>
#include <chrono>

void init() {
  glob::window::Create();
  glob::Init();
  Input::Initialize();
}

void updateSystems(entt::registry *reg, float dt) {
  //collision_debug::Update(*reg);
  player_controller::Update(*reg, dt);
  ability_controller::Update(*reg, dt);
 
  UpdatePhysics(*reg, dt);
  UpdateCollisions(*reg);
  Render(*reg);
}

int main(unsigned argc, char **argv) {
  glob::window::Create();
  glob::Init();
  init();  // Initialize everything
  Timer timer;

  std::cout << "Hello World!*!!!111\n";

  std::cout << "Test från development\n";

  glob::ModelHandle model_h =
      glob::GetModel("assets/Mech/Mech_humanoid_posed_unified_AO.fbx");
  glob::ModelHandle model_h2 = glob::GetModel("assets/Ball/Ball.fbx");
  glob::ModelHandle model_h3 =
      glob::GetModel("assets/Map_rectangular/map_rextangular.fbx");
  

  entt::registry registry;

  // Create ball
  auto entity = registry.create();
  registry.assign<BallComponent>(entity, true, true);
  registry.assign<PhysicsComponent>(entity, glm::vec3(1.0f, 0.0f, 0.0f), true, 0.0f);
  registry.assign<physics::Sphere>(entity, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
  registry.assign<ModelComponent>(entity, model_h2);
  registry.assign<TransformComponent>(entity, glm::vec3(5.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(1.f));

  // Create the map
  entity = registry.create();
  // Scale on the hitbox for the map
  float v1 = 7.171f;
  float v2 = 10.6859;  // 13.596f;
  float v3 = 5.723f;
  registry.assign<physics::Arena>(entity, -v2, v2, -v3, v3, -v1, v1);
  registry.assign<ModelComponent>(entity, model_h3);
  registry.assign<TransformComponent>(entity, glm::vec3(0.f), glm::vec3(0.f),
                                      glm::vec3(1.f));
 

  glm::vec3 scale_character = glm::vec3(.1f, .1f, .1f);

  auto avatar = registry.create();  // this is the player avatar
  registry.assign<ModelComponent>(
      avatar, model_h,
      glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f) *
          scale_character);
  registry.assign<CameraComponent>(
      avatar, (Camera *)glob::GetCamera(),
      glm::vec3(0.38f, 0.62f, -0.06f));  // get the camera pointer from glob renderer
  registry.assign<PlayerComponent>(avatar);
  registry.assign<TransformComponent>(avatar, glm::vec3(-9.f, 4.f, 0.f),
                                      glm::vec3(0, 0, 0), scale_character);
  registry.assign<PhysicsComponent>(avatar, glm::vec3(.0f, .0f, .0f), true, 0.f);
 
  registry.assign<physics::OBB>(
      avatar,
      glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f) *
          scale_character,
      glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f),
      glm::vec3(0.f, 0.f, 1.f), (11.223f - (-0.205f)) * scale_character.x / 2.f,
      (8.159f - (-10.316f)) * scale_character.y / 2.f,
      (10.206f - (-1.196f)) * scale_character.z / 2.f);
  registry.assign<AbilityComponent>(avatar, NULL_ABILITY, false, 0.0f, NULL_ABILITY, false, false, 0.0f);
  
  // opponent
  entity = registry.create();
  registry.assign<ModelComponent>(
      entity, model_h,
      glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f) *
          scale_character);
  registry.assign<PhysicsComponent>(entity, glm::vec3(0), true, 0.f);
  registry.assign<physics::OBB>(
      entity, glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f) * scale_character,
      glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f),
       glm::vec3(0.f, 0.f, 1.f), (11.223f - (-0.205f)) * scale_character.x / 2.f,
      (8.159f - (-10.316f)) * scale_character.y / 2.f,
      (10.206f - (-1.196f)) * scale_character.z / 2.f);
  registry.assign<TransformComponent>(
      entity, glm::vec3(0.f,0.f,0.f),
                                      glm::vec3(0, 0, 0), scale_character);


  timer.Restart();
  float dt = 0.0f;
  while (!glob::window::ShouldClose()) {
    dt = timer.Restart();
    Input::Reset();
    // tick
    /*if (Input::IsKeyDown(GLFW_KEY_K)) {
      auto& c = registry.get<CameraComponent>(avatar);
      c.offset.x += 0.01f;
      std::cout << "Camera: " << c.offset.x << std::endl;
    }
    if (Input::IsKeyDown(GLFW_KEY_L)) {
      auto &c = registry.get<CameraComponent>(avatar);
      c.offset.x -= 0.01f;
      std::cout << "Camera: " << c.offset.x << std::endl;
    }
    if (Input::IsKeyDown(GLFW_KEY_O)) {
      auto &c = registry.get<CameraComponent>(avatar);
      c.offset.y += 0.01f;
      std::cout << "Camera y: " << c.offset.y << std::endl;
    }
    if (Input::IsKeyDown(GLFW_KEY_P)) {
      auto &c = registry.get<CameraComponent>(avatar);
      c.offset.y -= 0.01f;
      std::cout << "Camera y: " << c.offset.y << std::endl;
    }
    if (Input::IsKeyDown(GLFW_KEY_U)) {
      auto &c = registry.get<CameraComponent>(avatar);
      c.offset.z += 0.01f;
      std::cout << "Camera z: " << c.offset.z << std::endl;
    }
    if (Input::IsKeyDown(GLFW_KEY_I)) {
      auto &c = registry.get<CameraComponent>(avatar);
      c.offset.z -= 0.01f;
      std::cout << "Camera z: " << c.offset.z << std::endl;
    }*/
    // render
    updateSystems(&registry, dt);

    glob::Render();
    glob::window::Update();
  }

  glob::window::Cleanup();
  std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM()
            << " MB\n";
  std::cout << "VRAM usage: " << util::MemoryInfo::GetInstance().GetUsedVRAM()
            << " MB\n";

  std::cout << "WSA is initialized? " << std::boolalpha << NetAPI::Initialization::WinsockInitialized() << std::endl;

  std::cin.ignore();
  return EXIT_SUCCESS;
}