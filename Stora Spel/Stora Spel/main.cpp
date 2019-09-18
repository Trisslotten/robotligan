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
#include <GLFW/glfw3.h> //NTS: This one must be included after certain other things
#include "util/input.hpp"
#include "util/meminfo.hpp"
#include "util/timer.hpp"
#include "util/meminfo.hpp"

#include "util/global_settings.hpp"

#include <thread>
#include <chrono>

// NTS: Move into game engine class once that exists
void CreateEntities(entt::registry& registry);
void AddBallComponents(entt::registry& registry, entt::entity& entity,
                       glm::vec3 in_pos, glm::vec3 in_vel);
void AddArenaComponents(entt::registry& registry, entt::entity& entity);
void AddPlayerComponents(entt::registry& registry, entt::entity& entity);
void AddRobotComponents(entt::registry& registry, entt::entity& entity,
                        glm::vec3 in_pos);

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

  //Tell the GlobalSettings class to do a first read from the settings file
  GlobalSettings::Access()->UpdateValuesFromFile();

  //Create a registry and create some entities in it
  entt::registry registry;
  CreateEntities(registry);

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

	//Check if the keys for global settings are pressed
    if (Input::IsKeyPressed(GLFW_KEY_U)) {
      // Update contents of GlobalSettings from file
      GlobalSettings::Access()->UpdateValuesFromFile();
      // Write contents of GlobalSettings to console
      GlobalSettings::Access()->WriteMapToConsole();
    }

    
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

void CreateEntities(entt::registry& registry) {
  // Create one ball entity and add components
  auto ball_entity = registry.create();
  AddBallComponents(registry, ball_entity, glm::vec3(5.f, 0.f, 0.f), glm::vec3(0.0f));

  // Create one map entity and add components
  auto arena_entity = registry.create();
  AddArenaComponents(registry, arena_entity);

  // Create one player entity and add components
  auto avatar_entity = registry.create();
  AddPlayerComponents(registry, avatar_entity);
  AddRobotComponents(registry, avatar_entity, glm::vec3(-9.f, 4.f, 0.f));

  // Create one opponent with the default values
  auto opponent_entity = registry.create();
  AddRobotComponents(registry, opponent_entity, glm::vec3(0.f, 0.f, 0.f));


}

void AddBallComponents(entt::registry& registry, entt::entity& entity,
                       glm::vec3 in_pos, glm::vec3 in_vel) {
  // Prepare hard-coded values
  float ball_friction = 0.0f;
  float ball_radius = 1.0f;
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 ball_scale = glm::vec3(1.0f);
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");

  // Add components for a ball
  registry.assign<BallComponent>(entity, true, true);
  registry.assign<ModelComponent>(entity, model_ball);
  registry.assign<PhysicsComponent>(entity, in_vel, true, ball_friction);
  registry.assign<TransformComponent>(entity, in_pos, zero_vec,
                                      ball_scale);

  // Add a hitbox
  registry.assign<physics::Sphere>(entity, zero_vec, ball_radius);

}

void AddArenaComponents(entt::registry& registry, entt::entity& entity) {
  //Prepare hard-coded values
  // Scale on the hitbox for the map
  float v1 = 7.171f;
  float v2 = 10.6859;  // 13.596f;
  float v3 = 5.723f;
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  glob::ModelHandle model_arena =
      glob::GetModel("assets/Map_rectangular/map_rextangular.fbx");

  // Add components for an arena
  registry.assign<ModelComponent>(entity, model_arena);
  registry.assign<TransformComponent>(entity, zero_vec, zero_vec,
                                      arena_scale);

  // Add a hitbox
  registry.assign<physics::Arena>(entity, -v2, v2, -v3, v3, -v1, v1);
}

void AddPlayerComponents(entt::registry& registry, entt::entity& entity) {
  // Prepare hard-coded values
  AbilityID primary_id = SUPER_STRIKE;
  AbilityID secondary_id = NULL_ABILITY;
  float primary_cooldown =
      GlobalSettings::Access()->ValueOf("ABILITY_SUPER_STRIKE_COOLDOWN");
  glm::vec3 camera_offset = glm::vec3(0.38f, 0.62f, -0.06f);

  // Add components for a player
  registry.assign<AbilityComponent>(
      entity,            // Entity
      primary_id,        // Primary abiliy id
      false,             // Use primary ability
      primary_cooldown,  // Primary ability cooldown
      0.0f,              // Remaining cooldown
      secondary_id,      // Secondary ability
      false,             // Use secondary ability
      false,             // Shoot
      0.0f               // Remaining shoot cooldown
  );
  registry.assign<CameraComponent>(entity, (Camera*)glob::GetCamera(),
                                   camera_offset);
  registry.assign<PlayerComponent>(entity);
}

void AddRobotComponents(entt::registry& registry, entt::entity& entity,
                        glm::vec3 in_pos) {
  // Prepare hard-coded values
  float robot_friction = 0.0f;
  float coeff_x_side = (11.223f - (-0.205f));
  float coeff_y_side = (8.159f - (-10.316f));
  float coeff_z_side = (10.206f - (-1.196f));
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 alter_scale =
      glm::vec3(5.509f - 5.714f * 2.f, -1.0785f, 4.505f - 5.701f * 1.5f);
  glm::vec3 character_scale = glm::vec3(0.1f);
  glob::ModelHandle robot_model =
      glob::GetModel("assets/Mech/Mech_humanoid_posed_unified_AO.fbx");
	
  // Add components for a robot
  registry.assign<ModelComponent>(
      entity, robot_model, alter_scale * character_scale);
  registry.assign<PhysicsComponent>(entity, zero_vec, true,
                                    robot_friction);
  registry.assign<TransformComponent>(entity, in_pos,
                                      zero_vec, character_scale);
  
  // Add a hitbox
  registry.assign<physics::OBB>(
      entity,
      alter_scale * character_scale,	// Center
      glm::vec3(1.f, 0.f, 0.f),			//
	  glm::vec3(0.f, 1.f, 0.f),			// Normals
      glm::vec3(0.f, 0.f, 1.f),			//
	  coeff_x_side * character_scale.x * 0.5f,	//
      coeff_y_side * character_scale.y * 0.5f,  // Length of each plane
      coeff_z_side * character_scale.z * 0.5f	//
	  );
  
}