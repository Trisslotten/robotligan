#include "entitycreation.hpp"

#include <ability_component.hpp>
#include <ball_component.hpp>
#include <boundingboxes.hpp>
#include <camera_component.hpp>
#include <glob/graphics.hpp>
#include <glob\camera.hpp>
#include <model_component.hpp>
#include <physics_component.hpp>
#include <transform_component.hpp>
#include "engine.hpp"

ButtonComponent* GenerateButtonEntity(
    entt::registry& reg, std::string text, glm::vec2 pos,
    glob::Font2DHandle f_handle, unsigned int font_size,
    glm::vec4 normal_color,
    glm::vec4 hover_color) {
  auto button = reg.create();
  reg.assign<ButtonComponent>(button);
  reg.assign<TransformComponent>(button, glm::vec3(pos.x, pos.y, 0));
  ButtonComponent& b_c = reg.get<ButtonComponent>(button);
  b_c.text = text;
  b_c.text_current_color = b_c.text_normal_color = normal_color;
  b_c.text_hover_color = hover_color;
  b_c.font_size = font_size;  // menu_settings::font_size;
  b_c.bounds = glm::vec2(b_c.font_size * b_c.text.size() / 2, b_c.font_size);
  b_c.f_handle = f_handle;
  return &reg.get<ButtonComponent>(button);
}

void CreateEntities(entt::registry& registry, glm::vec3* in_pos_arr,
                    unsigned int in_num_pos) {
  // Create one ball entity and add components
  auto ball_entity = registry.create();
  AddBallComponents(registry, ball_entity, in_pos_arr[0], glm::vec3(0.0f));

  // Create one map entity and add components
  auto arena_entity = registry.create();
  AddArenaComponents(registry, arena_entity);

  // Create one player entity and add components
  auto avatar_entity = registry.create();
  // AddPlayerComponents(registry, avatar_entity);
  AddRobotComponents(registry, avatar_entity, in_pos_arr[1]);

  // Create other robots and add components
  for (unsigned int i = 2; i < in_num_pos; i++) {
    auto other_robot_entity = registry.create();
    AddRobotComponents(registry, other_robot_entity, in_pos_arr[i]);
  }
}

void ResetEntities(entt::registry& registry, glm::vec3* in_pos_arr,
                   unsigned int in_num_pos) {
  // Get everything with a physics component and a transform component
  auto reset_view = registry.view<PhysicsComponent, TransformComponent>();

  unsigned int pos_counter = 0;

  for (auto entity : reset_view) {
    PhysicsComponent& physics_component =
        reset_view.get<PhysicsComponent>(entity);
    TransformComponent& transform_component =
        reset_view.get<TransformComponent>(entity);

    physics_component.velocity = glm::vec3(0.0f);
    physics_component.is_airborne = true;

    transform_component.position = in_pos_arr[pos_counter];
    pos_counter++;

    if (pos_counter >= in_num_pos) {
      GlobalSettings::Access()->WriteError("main.cpp", "ResetEntities()",
                                           "Counter out of scope");
    }
  }
}

void AddBallComponents(entt::registry& registry, entt::entity& entity,
                       glm::vec3 in_pos, glm::vec3 in_vel) {
  // Prepare hard-coded values
  bool ball_is_real = true;
  bool ball_is_airborne = true;
  float ball_friction = 1.0f;
  float ball_radius = 1.0f;
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 ball_scale = glm::vec3(1.0f);
  glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");

  // Add components for a ball
  registry.assign<BallComponent>(entity, ball_is_real, ball_is_airborne);
  registry.assign<ModelComponent>(entity, model_ball);
  registry.assign<PhysicsComponent>(entity, in_vel, ball_is_airborne,
                                    ball_friction);
  registry.assign<TransformComponent>(entity, in_pos, zero_vec, ball_scale);

  // Add a hitbox
  registry.assign<physics::Sphere>(entity, zero_vec, ball_radius);
}

void AddArenaComponents(entt::registry& registry, entt::entity& entity) {
  // Prepare hard-coded values
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
  registry.assign<TransformComponent>(entity, zero_vec, zero_vec, arena_scale);

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
  // registry.assign<PlayerComponent>(entity);
}

void AddRobotComponents(entt::registry& registry, entt::entity& entity,
                        glm::vec3 in_pos) {
  // Prepare hard-coded values
  bool robot_is_airborne = true;
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
  registry.assign<ModelComponent>(entity, robot_model,
                                  alter_scale * character_scale);
  registry.assign<PhysicsComponent>(entity, zero_vec, robot_is_airborne,
                                    robot_friction);
  registry.assign<TransformComponent>(entity, in_pos, zero_vec,
                                      character_scale);

  // Add a hitbox
  registry.assign<physics::OBB>(
      entity,
      alter_scale * character_scale,            // Center
      glm::vec3(1.f, 0.f, 0.f),                 //
      glm::vec3(0.f, 1.f, 0.f),                 // Normals
      glm::vec3(0.f, 0.f, 1.f),                 //
      coeff_x_side * character_scale.x * 0.5f,  //
      coeff_y_side * character_scale.y * 0.5f,  // Length of each plane
      coeff_z_side * character_scale.z * 0.5f   //
  );
}