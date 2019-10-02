#include "gameserver.hpp"

#include <iostream>
#include <algorithm>
#include <bitset>
#include <glob/graphics.hpp>
#include <iostream>
#include <transform_component.hpp>

#include "ecs/components.hpp"
#include "ecs/systems/ability_controller_system.hpp"
#include "ecs/systems/collision_system.hpp"
#include "ecs/systems/goal_system.hpp"
#include "ecs/systems/physics_system.hpp"
#include "ecs/systems/player_controller_system.hpp"

#include "shared.hpp";

namespace {}  // namespace

GameServer::~GameServer() {}

void GameServer::Init() {
  glob::SetModelUseGL(false);

  GlobalSettings::Access()->UpdateValuesFromFile();

  server_.Setup(1337);

  CreateEntities();
  CreateGoals();
}

void GameServer::Update(float dt) {
  server_.Update();

  for (auto client_data : server_.GetNewlyConnected()) {
    std::cout << "DEBUG: Creating a player\n";
    CreatePlayer(client_data->ID);
  }

  for (auto& [id, client_data] : server_.GetClients()) {
    for (auto& packet : client_data->packets) {
      while (!packet.IsEmpty()) {
        HandlePacketBlock(packet, id);
      }
    }
    client_data->packets.clear();
  }
  auto player_view = registry_.view<PlayerComponent>();
  registry_.view<PlayerComponent>().each(
      [&](auto entity, PlayerComponent& player_c) {
        auto inputs = players_inputs_[player_c.id];
        player_c.actions = inputs.first;
        player_c.pitch += inputs.second.x;
        player_c.yaw += inputs.second.y;
        /*
        std::cout << "Pitch: " << player_c.pitch << "\n";
        std::cout << "Yaw:   " << player_c.yaw << "\n\n";
                */
      });
  players_inputs_.clear();

  /*
  registry_.view<TransformComponent>().each([&](auto entity, auto& trans_c) {
    glm::vec3 p = trans_c.position;
    //std::cout << (int)p.x << ", " << (int)p.y << ", " << (int)p.z << "\n";
  });
  */

  UpdateSystems(dt);

  for (auto& [id, client_data] : server_.GetClients()) {
    NetAPI::Common::Packet to_send;
    auto header = to_send.GetHeader();
    header->receiver = id;

    /*
    glm::vec3 ball_pos{0};
    registry_.view<TransformComponent, BallComponent>().each(
        [&](auto entity, auto& trans_c, auto& ball) {
          ball_pos = trans_c.position;
        });
    to_send << ball_pos;
    to_send << PacketBlockType::TEST_BALL_P;
    */
    auto view_cam = registry_.view<CameraComponent, PlayerComponent>();
    for (auto cam : view_cam) {
      auto& cam_c = view_cam.get<CameraComponent>(cam);
      auto& player_c = view_cam.get<PlayerComponent>(cam);
      if (id == player_c.id) {
        to_send << cam_c.orientation;
        break;
      }
    }
    to_send << PacketBlockType::CAMERA_TRANSFORM;

    auto view_player = registry_.view<PlayerComponent>();
    for (auto player : view_player) {
      auto& player_c = view_player.get(player);
      if (id == player_c.id) {
        to_send << player_c.energy_current;
        break;
      }
    }
    to_send << PacketBlockType::PLAYER_STAMINA;

    auto view_ball = registry_.view<BallComponent, TransformComponent>();
    for (auto ball : view_ball) {
      // auto& ball_c = view_cam.get<BallComponent>(ball);
      auto& trans_c = view_ball.get<TransformComponent>(ball);

      to_send << trans_c.rotation;
      to_send << trans_c.position;
      break;
    }
    to_send << PacketBlockType::BALL_TRANSFORM;

    auto view_players = registry_.view<TransformComponent, PlayerComponent>();
    int num_players = view_players.size();
    for (auto player : view_players) {
      auto& trans_c = view_players.get<TransformComponent>(player);
      auto& player_c = view_players.get<PlayerComponent>(player);
      to_send << trans_c.rotation;
      to_send << trans_c.position;
      to_send << player_c.id;
    }
    to_send << num_players;
    to_send << PacketBlockType::PLAYERS_TRANSFORMS;

    bool is_created = false;
    for (auto& created_id : created_players_) {
      if (id == created_id) {
        to_send << id;
        to_send << PacketBlockType::SET_CLIENT_PLAYER_ID;
        is_created = true;
        break;
      }
    }

    if (is_created) {
      for (auto& [id, client_data] : server_.GetClients()) {
        to_send << id;
        to_send << PacketBlockType::CREATE_PLAYER;
      }
    } else {
      for (auto& created_id : created_players_) {
        to_send << created_id;
        to_send << PacketBlockType::CREATE_PLAYER;
      }
    }

    auto view_goals = registry_.view<GoalComponenet, TeamComponent>();

    entt::entity blue_goal;

    bool sent_switch = false;
    for (auto goal : view_goals) {
      GoalComponenet& goal_goal_c = registry_.get<GoalComponenet>(goal);
      TeamComponent& goal_team_c = registry_.get<TeamComponent>(goal);
      to_send << goal_team_c;
      to_send << goal_goal_c.goals;
      to_send << PacketBlockType::TEAM_SCORE;
      if (goal_goal_c.switched_this_tick) {
        if (!sent_switch) {
          to_send << PacketBlockType::SWITCH_GOALS;
          sent_switch = true;
        }
        goal_goal_c.switched_this_tick = false;
      }
    }

	//send messages
    for (auto m : messages) {
      to_send.Add(m.c_str(), m.size());
      to_send << m.size();
      to_send << PacketBlockType::MESSAGE;
	  }
    // send new teams
    for (auto& p : new_teams_) {
      to_send << p.second;
      to_send << p.first;
      to_send << PacketBlockType::CHOOSE_TEAM;
    }

    server_.Send(to_send);
  }

  created_players_.clear();
  messages.clear();
  new_teams_.clear();
}

void GameServer::UpdateSystems(float dt) {
  player_controller::Update(registry_, dt);
  ability_controller::Update(registry_, dt);

  UpdatePhysics(registry_, dt);
  UpdateCollisions(registry_);
  if (goal_system::Update(registry_)) {
    ResetEntities();
  }
}

void GameServer::HandlePacketBlock(NetAPI::Common::Packet& packet,
                                   unsigned short id) {
  int16_t block_type = -1;
  packet >> block_type;
  switch (block_type) {
    case PacketBlockType::INPUT: {
      uint16_t actions = 0;
      // TODO: put in player_actions then in PlayerComponent
      float pitch = 0.f;
      float yaw = 0.f;
      packet >> yaw;
      packet >> pitch;
      packet >> actions;
      players_inputs_[id] = std::make_pair(actions, glm::vec2(pitch, yaw));
      // std::cout << "PACKET: INPUT, " << actions << ", " << yaw << ", " <<
      // pitch << "\n";
      break;
    }
    case PacketBlockType::MESSAGE: {
      size_t strsize = 0;
      packet >> strsize;
      std::string str;
      str.resize(strsize);
      packet.Remove(str.data(), strsize);
      messages.push_back(str);
      break;
    }
    case PacketBlockType::CHOOSE_TEAM: {
      PlayerID pid;
      unsigned int team;
      packet >> pid;
      packet >> team;

      new_teams_.push_back({pid, team});
    }
  }
}

void GameServer::HandleNewTeam() {
  // TODO: logic for swapping team
  new_teams_.erase(std::remove_if(new_teams_.begin(), new_teams_.end(),
                                  [](std::pair<PlayerID, unsigned int> pair) {
                                    if (pair.second < 2) return false;
                                    return true;
    }));
}

void GameServer::CreatePlayer(PlayerID id) {
  auto entity = registry_.create();

  // TODO: change with lobby and starting game
  glm::vec3 start_pos{-2.f, 4.f, 0.f};
  start_pos.x += 3.f * glm::sin(20.f * float(test_player_guid_));
  start_pos.z += 3.f * glm::sin(30.f * float(test_player_guid_) + 2.f);

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
  registry_.assign<TransformComponent>(entity, start_pos, zero_vec,
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

  // Prepare hard-coded values
  AbilityID primary_id = SWITCH_GOALS;
  AbilityID secondary_id = SWITCH_GOALS;
  float primary_cooldown =
      GlobalSettings::Access()->ValueOf("ABILITY_SUPER_STRIKE_COOLDOWN");
  glm::vec3 camera_offset = glm::vec3(0.38f, 0.62f, -0.06f);

  // Add components for a player
  registry_.assign<AbilityComponent>(
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
  registry_.assign<CameraComponent>(entity, camera_offset);

  auto& player_component = registry_.assign<PlayerComponent>(entity);
  player_component.id = id;
  created_players_.push_back(id);

  std::cout << "DEBUG: Created player id: " << player_component.id << "\n";
  ResetEntities();
}

void GameServer::CreateEntities() {
  // Create one ball entity and add components
  auto ball_entity = registry_.create();
  AddBallComponents(ball_entity, glm::vec3(0.f), glm::vec3(0.0f));

  // Create one map entity and add components
  auto arena_entity = registry_.create();
  AddArenaComponents(arena_entity);

  // Create one player entity and add components
  // CreatePlayer();

  ResetEntities();
}

void GameServer::ResetEntities() {
  // Reset Rotation
  auto rotation_view = registry_.view<TransformComponent>();
  for (auto entity : rotation_view) {
    auto& t = rotation_view.get(entity);
    t.rotation = glm::vec3(0.f);
  }

  // Reset Players
  glm::vec3 player_pos[3];
  for (int i = 0; i < 3; ++i) {
    player_pos[i].x = GlobalSettings::Access()->ValueOf(
      std::string("PLAYERPOSITION") + std::to_string(i) + "X");
    player_pos[i].y = GlobalSettings::Access()->ValueOf(
        std::string("PLAYERPOSITION") + std::to_string(i) + "Y");
    player_pos[i].z = GlobalSettings::Access()->ValueOf(
        std::string("PLAYERPOSITION") + std::to_string(i) + "Z");
  }

  unsigned int blue_counter = 0;
  unsigned int red_counter = 0;
  bool blue_team = true;
  auto player_view = registry_.view<PlayerComponent, PhysicsComponent, TransformComponent, CameraComponent>();
  for (auto entity : player_view) {
    PhysicsComponent& physics_component =
        player_view.get<PhysicsComponent>(entity);
    TransformComponent& transform_component =
        player_view.get<TransformComponent>(entity);
    PlayerComponent& player_component =
        player_view.get<PlayerComponent>(entity);
    CameraComponent& cam = player_view.get<CameraComponent>(entity);

    if (registry_.has<TeamComponent>(entity)) {
      TeamComponent& team = registry_.get<TeamComponent>(entity);
      if (team.team == TEAM_RED) blue_team = false;
    }
    
    float orientation_value = 0.f;
    if (blue_team) orientation_value = glm::pi<float>();


    cam.orientation = glm::vec3(0.f, orientation_value, 0.f);

    player_component.pitch = 0.f;
    player_component.yaw = orientation_value;
    physics_component.velocity = glm::vec3(0.0f);
    physics_component.is_airborne = true;

    transform_component.rotation = glm::vec3(0.f, orientation_value, 0.f);

    if (blue_team) {
      transform_component.position = player_pos[blue_counter];
      blue_counter++;

      blue_counter %= 3; 
    } else {
      transform_component.position = player_pos[red_counter];
      transform_component.position.x *= -1.f;
      red_counter++;

      red_counter %= 3;
    }
  }
  
  // TODO: reset pick-up


  // Reset Balls
  auto ball_view =
      registry_.view<BallComponent, PhysicsComponent, TransformComponent>();
  for (auto entity : ball_view) {
    BallComponent& ball_component = ball_view.get<BallComponent>(entity);

    if (ball_component.is_real == false) {
      registry_.destroy(entity);
      continue;
    }

    PhysicsComponent& physics_component =
        ball_view.get<PhysicsComponent>(entity);
    TransformComponent& transform_component =
        ball_view.get<TransformComponent>(entity);

    physics_component.velocity = glm::vec3(0.0f);
    physics_component.is_airborne = true;

    glm::vec3 pos;
    pos.x = GlobalSettings::Access()->ValueOf("BALLPOSITIONX");
    pos.y = GlobalSettings::Access()->ValueOf("BALLPOSITIONY");
    pos.z = GlobalSettings::Access()->ValueOf("BALLPOSITIONZ");
    transform_component.position = pos;

    ball_component.rotation = glm::vec3(0.f);
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
  glob::ModelHandle model_arena =
      glob::GetModel("assets/Map_rectangular/map_rextangular.fbx");

  // Add components for an arena
  // registry_.assign<ModelComponent>(entity, model_arena);
  registry_.assign<TransformComponent>(entity, zero_vec, zero_vec, arena_scale);

  // Add a hitbox
  registry_.assign<physics::Arena>(entity, -v2, v2, -v3, v3, -v1, v1);
  auto md = glob::GetMeshData(model_arena);
  glm::mat4 matrix =
      glm::rotate(-90.f * glm::pi<float>() / 180.f, glm::vec3(1.f, 0.f, 0.f)) *
      glm::rotate(90.f * glm::pi<float>() / 180.f, glm::vec3(0.f, 0.f, 1.f));

  for (auto& v : md.pos) v = matrix * glm::vec4(v, 1.f);
  auto& mh = registry_.assign<physics::MeshHitbox>(entity, std::move(md.pos),
                                                   std::move(md.indices));
  // glob::LoadWireframeMesh(model_arena, mh.pos, mh.indices);
}

void GameServer::CreateGoals() {
  // blue team's goal, place at red goal in world
  auto entity_blue = registry_.create();
  registry_.assign<physics::OBB>(entity_blue, glm::vec3(0.f, 0.f, 0.f),
                                 glm::vec3(1, 0, 0), glm::vec3(0, 1, 0),
                                 glm::vec3(0, 0, 1), 1.f, 1.f, 2.f);
  registry_.assign<TeamComponent>(entity_blue, TEAM_BLUE);
  registry_.assign<GoalComponenet>(entity_blue);
  auto& trans_comp = registry_.assign<TransformComponent>(entity_blue);
  trans_comp.position = glm::vec3(-12.f, -4.f, 0.f);

  // red team's goal, place at blue goal in world
  auto entity_red = registry_.create();
  registry_.assign<physics::OBB>(entity_red, glm::vec3(0.f, 0.f, 0.f),
                                 glm::vec3(1, 0, 0), glm::vec3(0, 1, 0),
                                 glm::vec3(0, 0, 1), 1.f, 1.f, 2.f);
  registry_.assign<TeamComponent>(entity_red, TEAM_RED);
  registry_.assign<GoalComponenet>(entity_red);
  auto& trans_comp2 = registry_.assign<TransformComponent>(entity_red);
  trans_comp2.position = glm::vec3(12.f, -4.f, 0.f);
}