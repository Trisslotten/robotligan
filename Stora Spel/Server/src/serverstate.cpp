#include "serverstate.hpp"

#include "shared/camera_component.hpp"
#include "shared/transform_component.hpp"

#include <collision.hpp>
#include <ecs\components\pick_up_event.hpp>
#include <glob\graphics.hpp>
#include <physics.hpp>
#include <shared\id_component.hpp>
#include <shared\pick_up_component.hpp>
#include "ecs/components.hpp"
#include "ecs/components/match_timer_component.hpp"
#include "gameserver.hpp"

void ServerLobbyState::Init() {
  start_game_timer.Restart();
  starting_ = false;
  for (auto& ready_c : clients_ready_) {
    ready_c.second = false;
  }
  srand(time(NULL));
}

void ServerLobbyState::Update(float dt) {
  int min_players = 1;
  for (auto& cli : this->game_server_->GetServer().GetClients()) {
    if (!cli.second->client.IsConnected() && cli.second->is_active) {
      cli.second->is_active = false;
      this->client_teams_.erase(cli.second->ID);
      this->clients_ready_.erase(cli.second->ID);
      this->game_server_->GetServer().KickPlayer(cli.second->ID);
      teams_updated_ = true;
      NetAPI::Common::Packet p;
      p << cli.second->ID;
      p << PacketBlockType::PLAYER_LOBBY_DISCONNECT;
      this->game_server_->GetServer().SendToAll(p);
    }
  }
  bool can_start = clients_ready_.size() >= min_players;
  for (auto ready : clients_ready_) {
    can_start = can_start && ready.second;
  }

  if (can_start && !starting_) {
    std::cout << "DEBUG: Start game countdown\n";
    start_game_timer.Restart();
    starting_ = true;
  }
  if (!can_start) {
    starting_ = false;
  }

  if (starting_ && start_game_timer.Elapsed() > 5.f) {
    std::cout << "DEBUG: Start game countdown is zero\n";
    game_server_->ChangeState(ServerStateType::PLAY);
  }
  for (auto& [client_id, to_send] : game_server_->GetPackets()) {
    if (teams_updated_) {
      for (auto client_team : client_teams_) {
        to_send << client_team.first;   // send id
        to_send << client_team.second;  // send team
        bool ready = clients_ready_[client_team.first];
        to_send << ready;
        to_send << PacketBlockType::LOBBY_UPDATE_TEAM;
      }
    }
    to_send << client_id;
    to_send << PacketBlockType::LOBBY_YOUR_ID;
  }
  teams_updated_ = false;
}

void ServerLobbyState::Cleanup() {
  //
}

void ServerLobbyState::HandleDataToSend() {

}

void ServerPlayState::Init() {
  reset_timer_.Restart();
  reset_timer_.Pause();
  auto& server = game_server_->GetServer();
  auto& registry = game_server_->GetRegistry();

  // initialize option values
  match_time_ = (int)GlobalSettings::Access()->ValueOf("MATCH_TIME");
  count_down_time_ = (int)GlobalSettings::Access()->ValueOf("COUNTDOWN_TIME");
  physics::SetGravity(GlobalSettings::Access()->ValueOf("PHYSICS_GRAVITY"));

  // Start the countdown and match timer
  match_timer_.Restart();
  countdown_timer_.Restart();

  CreateInitialEntities(server.GetConnectedPlayers());

  ResetEntities();

  // Create replay machine
  this->replay_machine_ = new ReplayMachine();

  for (auto& [client_id, client_data] : server.GetClients()) {
    NetAPI::Common::Packet to_send;
    to_send.GetHeader()->receiver = client_id;

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
    to_send << client_abilities_[client_id];

    to_send << PacketBlockType::GAME_START;

    server.Send(to_send);
  }
}

void ServerPlayState::Update(float dt) {
  auto& registry = game_server_->GetRegistry();
  auto& server = game_server_->GetServer();

  dispatcher.update<EventInfo>();

  registry.view<PlayerComponent>().each(
      [&](auto entity, PlayerComponent& player_c) {
        // Check if we are taking inputs from the
        // client or if we are reading them from a replay
        if (!this->replay_) {
          auto inputs = players_inputs_[player_c.client_id];
          if (countdown_timer_.Elapsed() <= count_down_time_) {
            match_timer_.Pause();
          } else {
            player_c.actions = inputs.first;
            if (reset_ == false)
              match_timer_.Resume();
            countdown_timer_.Pause();
          }
          player_c.pitch += inputs.second.x;
          player_c.yaw += inputs.second.y;
          // Check if the game should be be recorded
          if (this->record_) {
            this->Record(player_c.actions, player_c.pitch, player_c.yaw, dt);
          }
        } else {
          this->Replay(player_c.actions, player_c.pitch, player_c.yaw);
        }
      });
  // players_inputs_.clear();

  // switch goal cleanup
  auto view_goals = registry.view<GoalComponenet, TeamComponent>();
  for (auto goal : view_goals) {
    GoalComponenet& goal_goal_c = registry.get<GoalComponenet>(goal);
    TeamComponent& goal_team_c = registry.get<TeamComponent>(goal);
    if (goal_goal_c.switched_this_tick) {
      goal_goal_c.switched_this_tick = false;
    }
  }

  if (reset_timer_.Elapsed() > 3.0f) {
    ResetEntities();
    reset_timer_.Restart();
    reset_timer_.Pause();
    reset_ = false;

    match_timer_.Resume();

    GameEvent reset_event;
    reset_event.type = GameEvent::RESET;
    dispatcher.trigger<GameEvent>(reset_event);
  }
  if (match_timer_.Elapsed() > match_time_) {
    EndGame();
  }
}

void ServerPlayState::HandleDataToSend() {
  auto& registry = game_server_->GetRegistry();
  for (auto& [client_id, to_send] : game_server_->GetPackets()) {
    EntityID client_player_id = clients_player_ids_[client_id];

    if (!clients_receive_updates_[client_id]) {
      // TODO: maybe send important packets even if not initialized
      continue;
    }

    auto view_cam = registry.view<CameraComponent, IDComponent>();
    for (auto cam : view_cam) {
      auto& cam_c = view_cam.get<CameraComponent>(cam);
      auto& id_c = view_cam.get<IDComponent>(cam);
      if (client_player_id == id_c.id) {
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

    auto view_physics = registry.view<PhysicsComponent, IDComponent>();
    int num_physics = view_physics.size();
    for (auto entity : view_physics) {
      auto& phys_c = view_physics.get<PhysicsComponent>(entity);
      auto& id_c = view_physics.get<IDComponent>(entity);

      to_send << phys_c.is_airborne;
      to_send << phys_c.velocity;
      to_send << id_c.id;
    }
    to_send << num_physics;
    to_send << PacketBlockType::PHYSICS_DATA;

    auto view_player = registry.view<PlayerComponent>();
    for (auto player : view_player) {
      auto& player_c = view_player.get(player);
      if (client_id == player_c.client_id) {
        to_send << player_c.energy_current;
        break;
      }
    }
    to_send << PacketBlockType::PLAYER_STAMINA;

    auto view_players2 = registry.view<PlayerComponent, TeamComponent,
      PointsComponent, IDComponent>();

    for (auto player : view_players2) {
      auto& player_player_c = registry.get<PlayerComponent>(player);
      auto& player_points_c = registry.get<PointsComponent>(player);
      auto& player_team_c = registry.get<TeamComponent>(player);
      auto& player_id_c = registry.get<IDComponent>(player);

      if (player_points_c.changed) {
        to_send << player_team_c.team;
        to_send << player_player_c.client_id;  // client id
        to_send << player_points_c.GetPoints();
        to_send << player_points_c.GetGoals();
        to_send << player_id_c.id;
        to_send << player_points_c.GetBlocks();
        to_send << player_points_c.GetAssists();
        to_send << PacketBlockType::UPDATE_POINTS;
      }
    }

    for (auto entity : created_pick_ups_) {
      auto& t = registry.get<TransformComponent>(entity);
      auto& id = registry.get<IDComponent>(entity);
      to_send << t.position;
      to_send << id.id;
      to_send << PacketBlockType::CREATE_PICK_UP;
      // std::cout << "PACKET: CREATED_PICK_UP\n";
    }

    auto pick_up_events = registry.view<PickUpEvent>();
    for (auto entity : pick_up_events) {
      auto& pick_event = pick_up_events.get(entity);
      to_send << pick_event.pick_up_id;
      to_send << PacketBlockType::DESTROY_PICK_UP;
      if (client_id == pick_event.client_id) {
        to_send << pick_event.ability_id;
        to_send << PacketBlockType::RECEIVE_PICK_UP;
      }
      registry.remove<PickUpEvent>(entity);
    }
    auto view_goals = registry.view<GoalComponenet, TeamComponent>();
    entt::entity blue_goal;
    bool sent_switch = false;
    for (auto goal : view_goals) {
      GoalComponenet& goal_goal_c = registry.get<GoalComponenet>(goal);
      TeamComponent& goal_team_c = registry.get<TeamComponent>(goal);
      to_send << goal_team_c;
      to_send << goal_goal_c.goals;
      to_send << PacketBlockType::TEAM_SCORE;
      if (goal_goal_c.switched_this_tick) {
        if (!sent_switch) {
          to_send << PacketBlockType::SWITCH_GOALS;
          sent_switch = true;
        }
      }
    }

    // send created projectiles
    for (auto projectiles : created_projectiles_) {
      to_send << projectiles.entity_id;
      to_send << projectiles.projectile_id;
      to_send << PacketBlockType::CREATE_PROJECTILE;
    }
    // send destroy entity
    for (auto entity_id : destroy_entities_) {
      to_send << entity_id;
      to_send << PacketBlockType::DESTROY_ENTITIES;
    }

    // Send countdown & match time in sec
    to_send << (int)countdown_timer_.Elapsed();
    to_send << (int)match_timer_.Elapsed();
    to_send << match_time_;
    to_send << count_down_time_;
    to_send << PacketBlockType::MATCH_TIMER;

    to_send << player_frame_id_[client_id];
    to_send << PacketBlockType::FRAME_ID;
  }
  created_projectiles_.clear();
  destroy_entities_.clear();
  created_pick_ups_.clear();
}

void ServerPlayState::Cleanup() {
  if (this->replay_machine_ != nullptr) {
    delete this->replay_machine_;
  }
  game_server_->GetRegistry().reset();

  client_abilities_.clear();
  client_teams_.clear();
  clients_player_ids_.clear();
  red_players_ = 0;
  blue_players_ = 0;
  entity_guid_ = 0;
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
    registry.assign<TeamComponent>(*view_iter,
                                   client_teams_[player_c.client_id]);

    AbilityID primary_id = client_abilities_[player_c.client_id];
    AbilityID secondary_id = AbilityID::SWITCH_GOALS;
    float primary_cooldown =
        GlobalSettings::Access()->ValueOf("ABILITY_SUPER_STRIKE_COOLDOWN");

    // Add components for a player
    registry.assign<AbilityComponent>(
        *view_iter,        // Entity
        primary_id,        // Primary abiliy id
        false,             // Use primary ability
        primary_cooldown,  // Primary ability cooldown
        0.0f,              // Remaining cooldown
        secondary_id,      // Secondary ability
        false,             // Use secondary ability
        false,             // Shoot
        0.0f               // Remaining shoot cooldown
    );
    view_iter++;
  }

  CreateArenaEntity();
  CreateBallEntity();
  CreateGoals();

  for (auto a : clients_player_ids_) {
    std::cout << "client_id=" << a.first << ", entity_id=" << a.second << "\n";
  }
}

void ServerPlayState::CreateArenaEntity() {
  auto& registry = game_server_->GetRegistry();

  auto entity = registry.create();
  glm::vec3 arena_scale = glm::vec3(4.0f, 4.0f, 4.0f);
  // Prepare hard-coded values
  // Scale on the hitbox for the map
  float v1 = 6.8f * arena_scale.z;
  float v2 = 10.67f * arena_scale.x;  // 13.596f;
  float v3 = 2.723f * arena_scale.y;
  float v4 = 5.723f * arena_scale.y;
  glm::vec3 zero_vec = glm::vec3(0.0f);

  glob::ModelHandle model_arena =
      glob::GetModel("assets/Map/Map_singular_TMP.fbx");
  ;

  // Add components for an arena
  // registry_.assign<ModelComponent>(entity, model_arena);
  registry.assign<TransformComponent>(entity, zero_vec, zero_vec, arena_scale);

  // Add a hitbox
  registry.assign<physics::Arena>(entity, -v2, v2, -v3, v4, -v1, v1);
  auto md = glob::GetMeshData(model_arena);
  glm::mat4 matrix =
      glm::rotate(-90.f * glm::pi<float>() / 180.f, glm::vec3(1.f, 0.f, 0.f)) *
      glm::rotate(90.f * glm::pi<float>() / 180.f, glm::vec3(0.f, 0.f, 1.f));

  for (auto& v : md.pos) v = matrix * glm::vec4(v, 1.f);
  for (auto& v : md.pos) v *= arena_scale;

  physics::Arena a;
  a.xmax = -1;
  a.xmin = 1;
  a.ymax = -1;
  a.ymin = 1;
  a.zmax = -1;
  a.zmin = 1;
  for (auto& v : md.pos) {
    if (v.x > a.xmax) a.xmax = v.x;
    if (v.x < a.xmin) a.xmin = v.x;
    if (v.y > a.ymax) a.ymax = v.y;
    if (v.y < a.ymin) a.ymin = v.y;
    if (v.z > a.zmax) a.zmax = v.z;
    if (v.z < a.zmin) a.zmin = v.z;
  }

  registry.assign<FailSafeArenaComponent>(entity, a);
  auto& mh = registry.assign<physics::MeshHitbox>(entity, std::move(md.pos),
                                                  std::move(md.indices));
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
  registry.assign<PhysicsComponent>(entity, glm::vec3(0), glm::vec3(0.f),
                                    ball_is_airborne, ball_friction);
  registry.assign<TransformComponent>(entity, zero_vec, glm::vec3(0),
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
  registry.assign<PhysicsComponent>(entity, zero_vec, zero_vec,
                                    robot_is_airborne, robot_friction);
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
  glm::vec3 camera_offset = glm::vec3(0.38f, 0.62f, -0.06f);
  registry.assign<CameraComponent>(entity, camera_offset);

  auto& player_component = registry.assign<PlayerComponent>(entity);

  // Prepare hard-coded values
  /*AbilityID primary_id = AbilityID::SWITCH_GOALS;
  AbilityID secondary_id = AbilityID::SWITCH_GOALS;
  float primary_cooldown =
      GlobalSettings::Access()->ValueOf("ABILITY_SUPER_STRIKE_COOLDOWN");

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
  );*/

  // START ---------- Buff component [MOVE TO PICK-UP EVENT] ----------
  // Available buffs: SPEED_BOOST, JUMP_BOOST, INFINITE_STAMINA
  BuffID buff_id = SPEED_BOOST;
  float buff_duration =
      GlobalSettings::Access()->ValueOf("BUFF_SPEED_BOOST_DURATION");

  // Add component for a player
  registry.assign<BuffComponent>(entity,         // Entity
                                 buff_id,        // Active buff
                                 true,           // Toggle buff
                                 buff_duration,  // Buff duration
                                 0.0f            // Remaining duration
  );
  // END ---------- Buff component [MOVE TO PICK-UP EVENT] ----------

  /*if (last_spawned_team_ == 1) {
    registry.assign<TeamComponent>(entity, TEAM_BLUE);
    last_spawned_team_ = 0;
    blue_players_++;
  } else {
    registry.assign<TeamComponent>(entity, TEAM_RED);
    last_spawned_team_ = 1;
    red_players_++;
  }*/

  registry.assign<PointsComponent>(entity);

  // TODO: call later
  // ResetEntities();

  // std::cout << "DEBUG: state.cpp: Created player" << "\n";
}

void ServerPlayState::ResetEntities() {
  auto& registry = game_server_->GetRegistry();

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

  bool switched_goals = false;
  auto view_goal =
      registry.view<TransformComponent, GoalComponenet, TeamComponent>();
  for (auto entity : view_goal) {
    auto& team = view_goal.get<TeamComponent>(entity);
    if (team.team == TEAM_BLUE) {
      auto& trans = view_goal.get<TransformComponent>(entity);
      if (trans.position.x > 0) {
        switched_goals = true;
        
        break;
      }
    }
  }

  auto player_view = registry.view<PlayerComponent, PhysicsComponent,
                                   TransformComponent, CameraComponent>();
  for (auto entity : player_view) {
    bool blue_team = true;
    PhysicsComponent& physics_component =
        player_view.get<PhysicsComponent>(entity);
    TransformComponent& transform_component =
        player_view.get<TransformComponent>(entity);
    PlayerComponent& player_component =
        player_view.get<PlayerComponent>(entity);
    CameraComponent& cam = player_view.get<CameraComponent>(entity);

    if (registry.has<TeamComponent>(entity)) {
      TeamComponent& team = registry.get<TeamComponent>(entity);
      if (team.team == TEAM_RED && !switched_goals) blue_team = false;
      if (team.team == TEAM_BLUE && switched_goals) blue_team = false;
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

  // reset pick-up
  auto pick_up_view = registry.view<PickUpComponent>();
  for (auto entity : pick_up_view) registry.destroy(entity);

  CreatePickUpComponents();

  //  std::cout << "reset entities\n";

  // Reset Balls
  auto ball_view =
      registry.view<BallComponent, PhysicsComponent, TransformComponent>();
  for (auto entity : ball_view) {
    BallComponent& ball_component = ball_view.get<BallComponent>(entity);

    if (ball_component.is_real == false) {
      registry.destroy(entity);
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
    ball_component.is_homing = false;
    ball_component.homer_cid = -1;
  }
}

void ServerPlayState::CreatePickUpComponents() {
  auto& registry = game_server_->GetRegistry();
  glm::vec3 pos = glm::vec3(30 * float(rand()) / RAND_MAX - 15.f, -8.5f,
                            30 * float(rand()) / RAND_MAX - 15.f);
  auto entity = CreateIDEntity();
  registry.assign<TransformComponent>(entity, pos, glm::vec3(0.f),
                                      glm::vec3(1.f));
  registry.assign<PickUpComponent>(entity);
  registry.assign<physics::OBB>(entity, pos, glm::vec3(1.f, 0.f, 0.f),
                                glm::vec3(0.f, 1.f, 0.f),
                                glm::vec3(0.f, 0.f, 1.f), 1.f, 1.f, 1.f);
  created_pick_ups_.push_back(entity);
}

void ServerPlayState::EndGame() {
  for (auto& [client_id, to_send] : game_server_->GetPackets()) {
    to_send << PacketBlockType::GAME_END;
  }
  game_server_->ChangeState(ServerStateType::LOBBY);
}

void ServerPlayState::ReceiveEvent(const EventInfo& e) {
  switch (e.event) {
    case Event::DESTROY_ENTITY: {
      destroy_entities_.push_back(e.e_id);
      break;
    }
    case Event::CREATE_CANNONBALL: {
      auto& registry = game_server_->GetRegistry();
      Projectile projectile;
      projectile.entity_id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, projectile.entity_id);
      projectile.projectile_id = ProjectileID::CANNON_BALL;
      created_projectiles_.push_back(projectile);
      break;
    }
    case Event::CREATE_TELEPORT_PROJECTILE: {
      auto& registry = game_server_->GetRegistry();
      Projectile projectile;
      projectile.entity_id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, projectile.entity_id);
      projectile.projectile_id = ProjectileID::TELEPORT_PROJECTILE;
      created_projectiles_.push_back(projectile);
      break;
    }
    case Event::CREATE_FORCE_PUSH: {
      auto& registry = game_server_->GetRegistry();
      Projectile projectile;
      projectile.entity_id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, projectile.entity_id);
      projectile.projectile_id = ProjectileID::FORCE_PUSH_OBJECT;
      created_projectiles_.push_back(projectile);
      break;
    }
    case Event::CREATE_MISSILE: {
      auto& registry = game_server_->GetRegistry();
      Projectile projectile;
      projectile.entity_id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, projectile.entity_id);
      projectile.projectile_id = ProjectileID::MISSILE_OBJECT;
      created_projectiles_.push_back(projectile);

      // Save game event
      GameEvent missile_fire_event;
      missile_fire_event.type = GameEvent::MISSILE_FIRE;
      missile_fire_event.missile_fire.projectile_id = projectile.entity_id;
      dispatcher.trigger(missile_fire_event);

      break;
    }
    case Event::CHANGED_TARGET: {
      std::unordered_map<int, NetAPI::Common::Packet>& packets =
          game_server_->GetPackets();
      auto p_c = game_server_->GetRegistry().get<PlayerComponent>(e.entity);
      packets[p_c.client_id] << e.e_id;
      packets[p_c.client_id] << PacketBlockType::YOUR_TARGET;
      break;
    }
    default:
      break;
  }
}

void ServerPlayState::CreateGoals() {
  auto& registry = game_server_->GetRegistry();
  // blue team's goal, place at red goal in world
  auto entity_blue = registry.create();
  registry.assign<physics::OBB>(entity_blue, glm::vec3(0.f, 0.f, 0.f),
                                glm::vec3(1, 0, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), 4.f, 4.f, 8.f);
  registry.assign<TeamComponent>(entity_blue, TEAM_BLUE);
  registry.assign<GoalComponenet>(entity_blue);
  auto& trans_comp = registry.assign<TransformComponent>(entity_blue);
  trans_comp.position = glm::vec3(-48.f, -6.f, 0.f);

  // red team's goal, place at blue goal in world
  auto entity_red = registry.create();
  registry.assign<physics::OBB>(entity_red, glm::vec3(0.f, 0.f, 0.f),
                                glm::vec3(1, 0, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), 4.f, 4.f, 8.f);
  registry.assign<TeamComponent>(entity_red, TEAM_RED);
  registry.assign<GoalComponenet>(entity_red);
  auto& trans_comp2 = registry.assign<TransformComponent>(entity_red);
  trans_comp2.position = glm::vec3(48.f, -6.f, 0.f);
}

/*
TODO: fix
void ServerPlayState::HandleNewTeam() {
  if (new_teams_.size() == 0) return;

  auto player_view = registry_.view<PlayerComponent, TeamComponent>();
  new_teams_.erase(
      std::remove_if(new_teams_.begin(), new_teams_.end(),
                     [&](std::pair<PlayerID, unsigned int> pair) {
                       if (pair.second < 2) {
                         for (auto entity : player_view) {
                           auto& player_c =
                               player_view.get<PlayerComponent>(entity);
                           auto& team_c =
                               player_view.get<TeamComponent>(entity);

                           if (player_c.id == pair.first) {
                             if (pair.second == team_c.team) {
                               return true;
                             } else {
                               // TODO : decline team if it is full

                               team_c.team = pair.second;
                               if (pair.second == TEAM_BLUE) {
                                 blue_players_++;
                                 red_players_--;
                               } else {
                                 blue_players_--;
                                 red_players_++;
                               }
                             }
                             break;
                           }
                         }
                         return false;
                       }
                       return true;
                     }),
      new_teams_.end());
}
*/

void ServerPlayState::StartResetTimer() {
  match_timer_.Pause();
  reset_timer_.Restart();
  reset_ = true;
}

// Replay stuff---
bool ServerPlayState::StartRecording(unsigned int in_replay_length_seconds) {
  if (!this->record_) {
    // Initiate the Replay Machine
    std::cout << "Recording...\n";
    this->replay_machine_->Init(in_replay_length_seconds, kServerUpdateRate, 1);
    this->record_ = true;
    return true;  // NTS: Return false if recording cannot start?
  }
  return false;
}

void ServerPlayState::Record(std::bitset<10>& in_bitset, float& in_x_value,
                             float& in_y_value, const float& in_dt) {
  auto& registry = this->game_server_->GetRegistry();

  // Save the frame with the ReplayMachine
  if (this->replay_machine_->SaveReplayFrame(in_bitset, in_x_value, in_y_value,
                                             registry, in_dt)) {
    std::cout << "Replaying...\n";
    // If true is returned it means the internal
    // BitPack is fully written and there is no
    // more space to record on.
    // Turn off recording.
    this->record_ = false;
    // Then start replaying the saved data
    this->replay_ = true;
  }
}

void ServerPlayState::Replay(std::bitset<10>& in_bitset, float& in_x_value,
                             float& in_y_value) {
  auto& registry = this->game_server_->GetRegistry();
  // Ensure we are not recording the replay
  this->record_ = false;
  // Read a frame with the ReplayMachine
  if (this->replay_machine_->LoadReplayFrame(in_bitset, in_x_value, in_y_value,
                                             registry)) {
    std::cout << "Finished.\n";
    // If true is returned it means the internal
    // BiPack has been fully read and there is no
    // more data to replay.
    // Turn off replaying.
    this->replay_ = false;
  }
}
//---