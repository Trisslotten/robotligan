#include "serverstate.hpp"

#include "shared/camera_component.hpp"
#include "shared/transform_component.hpp"

#include <collision.hpp>
#include <glob\graphics.hpp>
#include <shared\id_component.hpp>
#include "ecs/components/player_component.hpp"
#include "gameserver.hpp"

void ServerLobbyState::Init() {
  //
}

void ServerLobbyState::Update() {
  int min_players = 1;
  bool can_start = clients_ready_.size() >= min_players;
  for (auto ready : clients_ready_) {
    can_start = can_start && ready.second;
  }

  if (can_start && !starting) {
    std::cout << "DEBUG: Start game countdown\n";
    start_game_timer.Restart();
    starting = true;
  }
  if (!can_start) {
    starting = false;
  }

  if (starting && start_game_timer.Elapsed() > 5.f) {
    std::cout << "DEBUG: Start game countdown is zero\n";
    game_server_->ChangeState(ServerStateType::PLAY);
  }
}

void ServerLobbyState::Cleanup() {
  //
}

void ServerPlayState::Init() {
  auto& server = game_server_->GetServer();
  auto& registry = game_server_->GetRegistry();

  CreateInitialEntities(server.GetConnectedPlayers());

  for (auto& [client_id, client_data] : server.GetClients()) {
    NetAPI::Common::Packet to_send;
    auto header = to_send.GetHeader();
    header->receiver = client_id;

    auto ball_view = registry.view<BallComponent, IDComponent>();
    for (auto ball : ball_view) {
      auto& ball_c = ball_view.get<BallComponent>(ball);
      auto& id_c = ball_view.get<IDComponent>(ball);

      to_send << id_c.id;

      break;
    }

    to_send << clients_player_ids_[client_id];
    for (auto ids : clients_player_ids_) {
      to_send << ids.second;
    }

    int num_players = server.GetConnectedPlayers();
    to_send << num_players;

    to_send << PacketBlockType::GAME_START;

    server.Send(to_send);
  }
}

void ServerPlayState::Update() {
  auto& server = game_server_->GetServer();
  auto& registry = game_server_->GetRegistry();

  auto player_view = registry.view<PlayerComponent, IDComponent>();
  for (auto player : player_view) {
    auto& player_c = player_view.get<PlayerComponent>(player);
    auto& id_c = player_view.get<IDComponent>(player);
    auto inputs = players_inputs_[player_c.client_id];
    player_c.actions = inputs.first;
    player_c.pitch += inputs.second.x;
    player_c.yaw += inputs.second.y;
  }
  players_inputs_.clear();

  
  for (auto& [id, client_data] : server.GetClients()) {
    NetAPI::Common::Packet to_send;
    auto header = to_send.GetHeader();
    header->receiver = id;
    EntityID client_player_id = clients_player_ids_[id];

    auto view_cam = registry.view<CameraComponent, IDComponent>();
    for (auto cam : view_cam) {
      auto& cam_c = view_cam.get<CameraComponent>(cam);
      auto& id_c = view_cam.get<IDComponent>(cam);
      if (client_player_id  == id_c.id) {
        to_send << cam_c.orientation;
        break;
      }
    }
    to_send << PacketBlockType::CAMERA_TRANSFORM;

    auto view_entities = registry.view<TransformComponent, IDComponent>();
    int num_entities = view_entities.size();
    for (auto entity : view_entities) {
      auto& trans_c = view_entities.get<TransformComponent>(entity);
      auto& id_c = view_entities.get<IDComponent>(entity);
      to_send << trans_c.rotation;
      to_send << trans_c.position;
      to_send << id_c.id;
    }
    to_send << num_entities;
    to_send << PacketBlockType::ENTITY_TRANSFORMS;

    server.Send(to_send);
  }
}

void ServerPlayState::Cleanup() {
  //
}

entt::entity ServerPlayState::CreateIDEntity() {
  auto& registry = game_server_->GetRegistry();

  auto result = registry.create();
  registry.assign<IDComponent>(result, GetNextEntityGuid());
  return result;
}

void ServerPlayState::CreateInitialEntities(int num_players) {
  auto& registry = game_server_->GetRegistry();
  auto& server = game_server_->GetServer();

  for (int i = 0; i < num_players; i++) {
    CreatePlayerEntity();
  }
  // get assigned id from entity
  auto clients = server.GetClients();
  auto players_view = registry.view<PlayerComponent, IDComponent>();
  auto view_iter = players_view.begin();
  for (auto& client : clients) {
    auto& id_c = registry.get<IDComponent>(*view_iter);
    auto& player_c = registry.get<PlayerComponent>(*view_iter);
    clients_player_ids_[client.first] = id_c.id;
    player_c.client_id = client.first;
    view_iter++;
  }

  CreateArenaEntity();
  CreateBallEntity();

  for (auto a : clients_player_ids_) {
    std::cout << "client_id=" << a.first << ", entity_id=" << a.second << "\n";
  }
}

void ServerPlayState::CreateArenaEntity() {
  auto& registry = game_server_->GetRegistry();

  auto entity = registry.create();
  ;

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
  registry.assign<TransformComponent>(entity, zero_vec, zero_vec, arena_scale);

  // Add a hitbox
  registry.assign<physics::Arena>(entity, -v2, v2, -v3, v3, -v1, v1);
  auto md = glob::GetMeshData(model_arena);
  glm::mat4 matrix =
      glm::rotate(-90.f * glm::pi<float>() / 180.f, glm::vec3(1.f, 0.f, 0.f)) *
      glm::rotate(90.f * glm::pi<float>() / 180.f, glm::vec3(0.f, 0.f, 1.f));

  for (auto& v : md.pos) v = matrix * glm::vec4(v, 1.f);
  auto& mh = registry.assign<physics::MeshHitbox>(entity, std::move(md.pos),
                                                  std::move(md.indices));
  // glob::LoadWireframeMesh(model_arena, mh.pos, mh.indices);
}

void ServerPlayState::CreateBallEntity() {
  auto& registry = game_server_->GetRegistry();

  auto entity = CreateIDEntity();

  // Prepare hard-coded values
  bool ball_is_real = true;
  bool ball_is_airborne = true;
  float ball_friction = 1.0f;
  float ball_radius = 1.0f;
  glm::vec3 zero_vec = glm::vec3(0.0f);
  glm::vec3 ball_scale = glm::vec3(1.0f);
  // glob::ModelHandle model_ball = glob::GetModel("assets/Ball/Ball.fbx");

  // Add components for a ball
  registry.assign<BallComponent>(entity, ball_is_real, ball_is_airborne);
  // registry_.assign<ModelComponent>(entity, model_ball);
  registry.assign<PhysicsComponent>(entity, glm::vec3(0), ball_is_airborne,
                                    ball_friction);
  registry.assign<TransformComponent>(entity, glm::vec3(0), zero_vec,
                                      ball_scale);

  // Add a hitbox
  registry.assign<physics::Sphere>(entity, zero_vec, ball_radius);
}

void ServerPlayState::CreatePlayerEntity() {
  auto& registry = game_server_->GetRegistry();

  auto entity = CreateIDEntity();

  // TODO: change with lobby and starting game
  glm::vec3 start_pos{-2.f, 4.f, 0.f};

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
  registry.assign<PhysicsComponent>(entity, zero_vec, robot_is_airborne,
                                    robot_friction);
  registry.assign<TransformComponent>(entity, start_pos, zero_vec,
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
  registry.assign<CameraComponent>(entity, camera_offset);

  auto& player_component = registry.assign<PlayerComponent>(entity);

  // std::cout << "DEBUG: state.cpp: Created player" << "\n";
}
