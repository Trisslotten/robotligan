#include "gameserver.hpp"

#include <iostream>

#include <collision_system.hpp>
#include <iostream>
#include <physics_system.hpp>
#include <transform_component.hpp>

namespace {}  // namespace

GameServer::~GameServer() { server_.Cleanup(); }

void GameServer::Init() {
  server_.Setup(1337);

  glm::vec3 start_positions[3] = {
      glm::vec3(5.f, 0.f, 0.f),   // Ball
      glm::vec3(-9.f, 4.f, 0.f),  // Player
      glm::vec3(0.f, 0.f, 0.f)    // Others
  };
  CreateEntities(start_positions, 3);
}

void GameServer::Update(float dt) {
  server_.Update();

  UpdateSystems(dt);

  /*
  for (short i = 0; i < server_.GetConnectedPlayers(); i++) {
    auto packet = server_[i];

    glm::vec3 target_vel{0};

    if (!packet.IsEmpty()) {
      int actionflag = 0;
      packet >> actionflag;

      glm::vec3 vel{0};
      if ((actionflag & 1) == 1) vel += glm::vec3(1, 0, 0);
      if ((actionflag & 2) == 2) vel += glm::vec3(-1, 0, 0);
      if ((actionflag & 4) == 4) vel += glm::vec3(0, 0, -1);
      if ((actionflag & 8) == 8) vel += glm::vec3(0, 0, 1);
      if (length(vel) > 0.0001f) vel = normalize(vel);

      target_vel = 10.f * vel;
      // std::cout << "actionflag=" << actionflag << "\n";
    }
    float t = 0.02f;
    vs_[i] = glm::mix(vs_[i], target_vel, 1.f - glm::pow(t, dt));
    positions_[i] += vs_[i] * dt;
  }

  if (positions_.size() > 0) {
    NetAPI::Common::Packet packet;
    packet.Add(positions_.data(), positions_.size());
    packet << (int)positions_.size();

    server_.Send(packet);
  }
  */

  /*
  // Reset positions and velocities
  if (Input::IsKeyPressed(GLFW_KEY_K)) {
    // K as in kickoff
    ResetEntities( start_positions, 3);
  }
  */
}

void GameServer::UpdateSystems(float dt) {
  UpdatePhysics(registry_, dt);
  UpdateCollisions(registry_);
}

void GameServer::CreateEntities(glm::vec3* in_pos_arr,
                                unsigned int in_num_pos) {
  // Create one ball entity and add components
  auto ball_entity = registry_.create();
  AddBallComponents(ball_entity, in_pos_arr[0], glm::vec3(0.0f));

  // Create one map entity and add components
  auto arena_entity = registry_.create();
  AddArenaComponents(arena_entity);

  // Create one player entity and add components
  auto avatar_entity = registry_.create();
  // AddPlayerComponents( avatar_entity);
  AddRobotComponents(avatar_entity, in_pos_arr[1]);

  // Create other robots and add components
  for (unsigned int i = 2; i < in_num_pos; i++) {
    auto other_robot_entity = registry_.create();
    AddRobotComponents(other_robot_entity, in_pos_arr[i]);
  }
}

void GameServer::ResetEntities(glm::vec3* in_pos_arr, unsigned int in_num_pos) {
  // Get everything with a physics component and a transform component
  auto reset_view = registry_.view<PhysicsComponent, TransformComponent>();

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
      // GlobalSettings::Access()->WriteError("main.cpp", "ResetEntities()",
      // "Counter out of scope");
    }
  }
}

void GameServer::AddBallComponents(entt::entity& entity, glm::vec3 in_pos,
                                   glm::vec3 in_vel) {
  // Prepare hard-coded values
  bool ball_is_real = true;
  bool ball_is_airborne = true;
  float ball_friction = 1.0f;
  float ball_radius = 1.0f;
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 ball_scale = glm::vec3(1.0f);
  // glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");

  // Add components for a ball
  registry_.assign<BallComponent>(entity, ball_is_real, ball_is_airborne);
  // registry_.assign<ModelComponent>(entity, model_ball);
  registry_.assign<PhysicsComponent>(entity, in_vel, ball_is_airborne,
                                     ball_friction);
  registry_.assign<TransformComponent>(entity, in_pos, zero_vec, ball_scale);

  // Add a hitbox
  registry_.assign<physics::Sphere>(entity, zero_vec, ball_radius);
}

void GameServer::AddArenaComponents(entt::entity& entity) {
  // Prepare hard-coded values
  // Scale on the hitbox for the map
  float v1 = 7.171f;
  float v2 = 10.6859;  // 13.596f;
  float v3 = 5.723f;
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 arena_scale = glm::vec3(1.0f);
  // glob::ModelHandle model_arena =
  // glob::GetModel("assets/Map_rectangular/map_rextangular.fbx");

  // Add components for an arena
  // registry_.assign<ModelComponent>(entity, model_arena);
  registry_.assign<TransformComponent>(entity, zero_vec, zero_vec, arena_scale);

  // Add a hitbox
  registry_.assign<physics::Arena>(entity, -v2, v2, -v3, v3, -v1, v1);
}

void GameServer::AddRobotComponents(entt::entity& entity, glm::vec3 in_pos) {
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
  // glob::ModelHandle robot_model
  // =glob::GetModel("assets/Mech/Mech_humanoid_posed_unified_AO.fbx");

  // Add components for a robot
  // registry_.assign<ModelComponent>(entity, robot_model, alter_scale *
  // character_scale);
  registry_.assign<PhysicsComponent>(entity, zero_vec, robot_is_airborne,
                                     robot_friction);
  registry_.assign<TransformComponent>(entity, in_pos, zero_vec,
                                       character_scale);

  // Add a hitbox
  registry_.assign<physics::OBB>(
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
