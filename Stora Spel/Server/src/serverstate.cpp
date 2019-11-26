#include "serverstate.hpp"

#include "shared/camera_component.hpp"
#include "shared/transform_component.hpp"

#include <collision.hpp>
#include <ecs\components\pick_up_event.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glob\graphics.hpp>
#include <physics.hpp>
#include <shared\id_component.hpp>
#include <shared\pick_up_component.hpp>
#include "ecs/components.hpp"
#include "ecs/components/match_timer_component.hpp"
#include "gameserver.hpp"

#include <map>

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
      NetAPI::Common::Packet p;
      p << cli.second->ID;
      p << PacketBlockType::PLAYER_LOBBY_DISCONNECT;
      this->game_server_->GetServer().KickPlayer(cli.second->ID);
      teams_updated_ = true;
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
        std::string name = game_server_->GetClientNames()[client_team.first];

        to_send.Add(name.c_str(), name.size());
        to_send << name.size();
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

void ServerLobbyState::HandleDataToSend() {}

void ServerPlayState::Init() {
  score_.reserve(2);
  score_.push_back(0);
  score_.push_back(0);

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
  CreatePickupSpawners();
  CreateInitialEntities(server.GetConnectedPlayers());

  ResetEntities();
  created_pick_ups_.clear();

  // Replay machine
  this->replay_machine_ = nullptr;

  for (auto& [client_id, client_data] : server.GetClients()) {
    NetAPI::Common::Packet to_send;
    to_send.GetHeader()->receiver = client_id;

    auto team_view =
        registry.view<TeamComponent, IDComponent, PlayerComponent>();

    unsigned int team_id = 0;
    for (auto team : team_view) {
      auto& team_c = team_view.get<TeamComponent>(team);
      auto& id_c = team_view.get<IDComponent>(team);
      auto& player_c = team_view.get<PlayerComponent>(team);

      to_send << team_c.team;
      to_send << id_c.id;
      to_send << player_c.client_id;

      if (id_c.id == clients_player_ids_[client_id]) {
        team_id = team_c.team;
      }
    }
    to_send << (int)team_view.size();

    to_send << team_id;

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
    glm::vec3 arena_scale;
    arena_scale.x = GlobalSettings::Access()->ValueOf("ARENA_SCALE_X");
    arena_scale.y = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Y");
    arena_scale.z = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Z");
    int num_players = server.GetConnectedPlayers();
    to_send << num_players;
    to_send << client_abilities_[client_id];
    to_send << arena_scale;
    to_send << PacketBlockType::GAME_START;

    auto pick_up_view =
        registry.view<PickUpComponent, TransformComponent, IDComponent>();
    for (auto entity : pick_up_view) {
      auto& t = pick_up_view.get<TransformComponent>(entity);
      auto& id = pick_up_view.get<IDComponent>(entity);
      to_send << t.position;
      to_send << id.id;
      to_send << PacketBlockType::CREATE_PICK_UP;
    }

    server.Send(to_send);
  }
  reset_ = false;
}

void ServerPlayState::Update(float dt) {
  WallAnimation();
  auto& registry = game_server_->GetRegistry();
  auto& server = game_server_->GetServer();

  std::map<EntityID, entt::entity> ordered_map;

  // Add all entities with an IDComponent and a PlayerComponent to a map
  registry.view<IDComponent, PlayerComponent>().each(
      [&](auto entity, IDComponent& id_c, PlayerComponent& player_c) {
        ordered_map[id_c.id] = entity;
      });

  // Go through the ordered entities
  unsigned int loop_index = 0;
  for (auto pair : ordered_map) {
    PlayerComponent& player_c = registry.get<PlayerComponent>(pair.second);

    // Check if we are taking inputs from the
    // client or if we are reading them from a replay
    if (!this->replay_) {
      auto inputs = players_inputs_[player_c.client_id];

      if (countdown_timer_.Elapsed() <= count_down_time_) {
        match_timer_.Pause();
      } else {
        player_c.actions = inputs.first;
        if (!reset_) {
          match_timer_.Resume();
        }
        countdown_timer_.Pause();
      }
      player_c.pitch = inputs.second.x;
      player_c.yaw = inputs.second.y;
      // Check if the game should be be recorded
      if (this->record_) {
        this->Record(player_c.actions, player_c.pitch, player_c.yaw, dt,
                     loop_index);
      }
    } else {
      this->Replay(player_c.actions, player_c.pitch, player_c.yaw, loop_index);
    }

    loop_index++;
  }

  // players_inputs_.clear();

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

  // Get scores
  auto en = registry.view<TeamComponent, GoalComponenet>();
  for (auto te : en) {
    GoalComponenet& goal_goal_c = registry.get<GoalComponenet>(te);
    TeamComponent& goal_team_c = registry.get<TeamComponent>(te);
    /*to_send << goal_team_c;
    to_send << goal_goal_c.goals;*/

    if (score_[goal_team_c.team] != goal_goal_c.goals) {
    }
    score_[goal_team_c.team] = goal_goal_c.goals;
  }

  if (match_timer_.Elapsed() > match_time_) {
    if (score_[0] == score_[1]) {
      OverTime();
    } else {
      EndGame();
    }
  }
  if (reconnect_id_ < 50) {
    Reconnect(reconnect_id_);
    reconnect_id_ = 100;
  }
}

void ServerPlayState::HandleDataToSend() {
  bool pick_ups_sent = false;

  auto& registry = game_server_->GetRegistry();
  for (auto& [client_id, to_send] : game_server_->GetPackets()) {
    EntityID client_player_id = clients_player_ids_[client_id];

    if (!clients_receive_updates_[client_id]) {
      // TODO: maybe send important packets even if not initialized
      continue;
    }

    auto view_cam = registry.view<CameraComponent, IDComponent>();
    // for (auto cam : view_cam) {
    //  auto& cam_c = view_cam.get<CameraComponent>(cam);
    //  auto& id_c = view_cam.get<IDComponent>(cam);
    //  if (client_player_id == id_c.id) {
    //    to_send << cam_c.orientation;
    //    break;
    //  }
    //}
    // to_send << PacketBlockType::CAMERA_TRANSFORM;

    for (auto cam : view_cam) {
      auto& cam_c = view_cam.get<CameraComponent>(cam);
      auto& id_c = view_cam.get<IDComponent>(cam);
      to_send << cam_c.GetLookDir();
      to_send << id_c.id;
    }
    int num_dirs = view_cam.size();
    to_send << num_dirs;
    to_send << PacketBlockType::PLAYER_LOOK_DIR;

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

    auto view_player_id = registry.view<PlayerComponent, IDComponent>();
    for (auto player : view_player_id) {
      auto& player_c = view_player_id.get<PlayerComponent>(player);
      auto& id_c = view_player_id.get<IDComponent>(player);
      to_send << player_c.wanted_move_dir;
      to_send << id_c.id;
    }
    to_send << (int)view_player_id.size();
    to_send << PacketBlockType::PLAYER_MOVE_DIR;

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

    for (auto entity : created_walls_) {
      auto& t = registry.get<TransformComponent>(entity);
      auto& id = registry.get<IDComponent>(entity);

      to_send << t.rotation;
      to_send << t.position;
      to_send << id.id;
      to_send << PacketBlockType::CREATE_WALL;
    }

    for (auto entity : created_mines_) {
      auto& t_c = registry.get<TransformComponent>(entity);
      auto& id_c = registry.get<IDComponent>(entity);
      auto& mine_c = registry.get<MineComponent>(entity);

      to_send << t_c.position;
      to_send << id_c.id;
      to_send << mine_c.owner_team;
      to_send << PacketBlockType::CREATE_MINE;
    }

    for (auto entity : created_pick_ups_) {
      auto& t = registry.get<TransformComponent>(entity);
      auto& id = registry.get<IDComponent>(entity);
      to_send << t.position;
      to_send << id.id;
      to_send << PacketBlockType::CREATE_PICK_UP;
      pick_ups_sent = true;
      // std::cout << "PACKET: CREATED_PICK_UP id: " << id.id << std::endl;
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
      // registry.remove<PickUpEvent>(entity);
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
        switch_goal_timer_.Restart();
      }
      if (switch_goal_timer_.Elapsed() <= switch_goal_time_) {
        if (!sent_switch) {
          to_send << switch_goal_time_;
          to_send << (float)switch_goal_timer_.Elapsed();
          to_send << PacketBlockType::SWITCH_GOALS;
          sent_switch = true;
        }
      } else {
        if (!sent_switch) {
          switch_goal_timer_.Pause();
          to_send << switch_goal_time_;
          to_send << (float)switch_goal_time_;
          to_send << PacketBlockType::SWITCH_GOALS;
          sent_switch = true;
        }
      }
    }

    // Tell client if secondary ability was used
    // already added game event for this, sry :P not ideal to set
    // use_secondary to false here since ability_controller::TriggerAbility
    // can return false, i.e. too far away to use super strike or homing
    // ball

    /*auto view_abilities = registry.view<PlayerComponent,
    AbilityComponent>(); for (auto entity : view_abilities) { auto& player =
    view_abilities.get<PlayerComponent>(entity);

      if (player.client_id == client_id) {
        auto& ability = view_abilities.get<AbilityComponent>(entity);

        if (ability.use_secondary) {
          ability.use_secondary = false;
          to_send << PacketBlockType::SECONDARY_USED;
        }
      }
    }*/

    // send created projectiles
    for (auto projectiles : created_projectiles_) {
      to_send << projectiles.entity_id;
      to_send << projectiles.projectile_id;
      to_send << projectiles.pos;
      to_send << projectiles.ori;
      to_send << projectiles.creator_team;
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

    auto view_players = registry.view<PlayerComponent>();
    for (auto player : view_players) {
      auto& player_c = registry.get<PlayerComponent>(player);
      if (client_id == player_c.client_id) {
        to_send << player_c.ready_to_smash;
        to_send << PacketBlockType::YOU_CAN_SMASH;
        break;
      }
    }
  }

  created_projectiles_.clear();
  created_mines_.clear();
  destroy_entities_.clear();

  auto pick_up_events = registry.view<PickUpEvent>();
  for (auto entity : pick_up_events) {
    registry.destroy(entity);
  }

  // switch goal cleanup
  auto view_goals = registry.view<GoalComponenet, TeamComponent>();
  for (auto goal : view_goals) {
    GoalComponenet& goal_goal_c = registry.get<GoalComponenet>(goal);
    TeamComponent& goal_team_c = registry.get<TeamComponent>(goal);
    if (goal_goal_c.switched_this_tick) {
      goal_goal_c.switched_this_tick = false;
    }
  }

  created_walls_.clear();
  if (pick_ups_sent) created_pick_ups_.clear();
}

void ServerPlayState::Cleanup() {
  if (this->replay_machine_ != nullptr) {
    delete this->replay_machine_;
  }
  game_server_->GetRegistry().reset();
  game_server_->GetServer().ResetPlayers();
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
    AbilityID secondary_id = AbilityID::NULL_ABILITY;
    float primary_cooldown = game_server_->GetAbilityCooldowns()[primary_id];

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

  CreateMapEntity();
  CreateBallEntity();
  CreateGoals();

  for (auto a : clients_player_ids_) {
    std::cout << "client_id=" << a.first << ", entity_id=" << a.second << "\n";
  }
}

void ServerPlayState::CreateMapEntity() {
  auto& registry = game_server_->GetRegistry();

  auto entity = registry.create();
  glm::vec3 arena_scale2;
  arena_scale2.x = GlobalSettings::Access()->ValueOf("ARENA_SCALE_X");
  arena_scale2.y = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Y");
  arena_scale2.z = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Z");
  glm::vec3 arena_scale = glm::vec3(2.6f) * arena_scale2;
  // Prepare hard-coded values
  // Scale on the hitbox for the map
  float v1 = 6.8f * arena_scale.z;
  float v2 = 10.67f * arena_scale.x;  // 13.596f;
  float v3 = 2.723f * arena_scale.y;
  float v4 = 5.723f * arena_scale.y;
  glm::vec3 zero_vec = glm::vec3(0.0f);

  glob::ModelHandle model_map = glob::GetModel("assets/MapV3/Map_Hitbox.fbx");

  // Add components for an arena
  // registry_.assign<ModelComponent>(entity, model_arena);
  registry.assign<TransformComponent>(entity, zero_vec, zero_vec, arena_scale);

  // Add a hitbox
  registry.assign<physics::Arena>(entity, -v2, v2, -v3, v4, -v1, v1);
  auto md = glob::GetMeshData(model_map);
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
      (alter_scale * character_scale),          // Center
      glm::vec3(1.f, 0.f, 0.f),                 //
      glm::vec3(0.f, 1.f, 0.f),                 // Normals
      glm::vec3(0.f, 0.f, 1.f),                 //
      coeff_x_side * character_scale.x * 0.4f,  //
      coeff_y_side * character_scale.y * 0.5f,  // Length of each plane
      coeff_z_side * character_scale.z * 0.7f   //
  );
  glm::vec3 camera_offset = glm::vec3(-0.2f, 0.4f, 0.f);
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

  /*if (last_spawned_team_ == 1) {
    registry.assign<TeamComponent>(entity, TEAM_BLUE);
    last_spawned_team_ = 0;
    blue_players_++;
  } else {
    registry.assign<TeamComponent>(entity, TEAM_RED);
    last_spawned_team_ = 1;
    red_players_++;
  }*/

  // TEMP : Just so the replay knows the number of players all get added to
  // the blue team
  this->blue_players_++;
  // TEMP

  registry.assign<PointsComponent>(entity);

  // TODO: call later
  // ResetEntities();

  // std::cout << "DEBUG: state.cpp: Created player" << "\n";
}

void ServerPlayState::ResetEntities() {
  auto& registry = game_server_->GetRegistry();

  // Reset Players
  glm::vec3 player_pos[3];
  glm::vec3 arena_scale;
  arena_scale.x = GlobalSettings::Access()->ValueOf("ARENA_SCALE_X");
  arena_scale.y = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Y");
  arena_scale.z = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Z");
  for (int i = 0; i < 3; ++i) {
    player_pos[i].x =
        GlobalSettings::Access()->ValueOf(std::string("PLAYERPOSITION") +
                                          std::to_string(i) + "X") *
        arena_scale.x;
    player_pos[i].y = GlobalSettings::Access()->ValueOf(
        std::string("PLAYERPOSITION") + std::to_string(i) + "Y");
    player_pos[i].z =
        GlobalSettings::Access()->ValueOf(std::string("PLAYERPOSITION") +
                                          std::to_string(i) + "Z") *
        arena_scale.z;
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
  if (switched_goals) {
    auto view_goal =
        registry.view<TransformComponent, GoalComponenet, TeamComponent>();
    GoalComponenet* first_goal_comp = nullptr;
    GoalComponenet* second_goal_comp = nullptr;
    bool got_first = false;
    for (auto goal : view_goal) {
      TeamComponent& goal_team_c = registry.get<TeamComponent>(goal);
      GoalComponenet& goal_goal_c = registry.get<GoalComponenet>(goal);

      if (goal_team_c.team == TEAM_RED) {
        goal_team_c.team = TEAM_BLUE;
      } else {
        goal_team_c.team = TEAM_RED;
      }
      if (!got_first) {
        first_goal_comp = &goal_goal_c;
        got_first = true;
      } else {
        second_goal_comp = &goal_goal_c;
      }
    }
    if (first_goal_comp != nullptr && second_goal_comp != nullptr) {
      unsigned int first_goals = first_goal_comp->goals;
      first_goal_comp->goals = second_goal_comp->goals;
      second_goal_comp->goals = first_goals;
    }
  }
  switched_goals = false;

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
    player_component.can_jump = false;
    player_component.stunned = false;
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
  auto pick_up_view = registry.view<PickUpComponent, IDComponent>();
  for (auto pick_up : pick_up_view) {
    auto entity = registry.create();
    registry.assign<PickUpEvent>(entity, registry.get<IDComponent>(pick_up).id,
                                 -1, AbilityID::NULL_ABILITY);
    registry.destroy(pick_up);
  }
  auto spawner_view = registry.view<PickupSpawnerComponent, IDComponent>();
  for (auto spawner : spawner_view) {
    registry.get<PickupSpawnerComponent>(spawner).override_respawn = true;
  }

  // CreatePickUpComponents();

  //  std::cout << "reset entities\n";

  // Reset Balls
  auto ball_view =
      registry.view<BallComponent, PhysicsComponent, TransformComponent>();
  for (auto entity : ball_view) {
    BallComponent& ball_component = ball_view.get<BallComponent>(entity);

    if (ball_component.is_real == false) {
      EventInfo e;
      e.event = Event::DESTROY_ENTITY;
      e.entity = entity;
      e.e_id = registry.get<IDComponent>(entity).id;
      // registry.destroy(entity);
      dispatcher.enqueue(e);
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
    if (ball_component.is_homing) {
      ball_component.is_homing = false;
      // Save game event
      IDComponent& ball_id_c = registry.get<IDComponent>(entity);
      GameEvent homing_ball_end_event;
      homing_ball_end_event.type = GameEvent::HOMING_BALL_END;
      homing_ball_end_event.homing_ball_end.ball_id = ball_id_c.id;
      dispatcher.trigger(homing_ball_end_event);
    }
    ball_component.homer_cid = -1;
  }
}

EntityID ServerPlayState::CreatePickUpComponents(glm::vec3 pos) {
  auto& registry = game_server_->GetRegistry();

  int rand_id = rand() % 2;
  std::string config_str = "PICKUPPOSITION" + std::to_string(rand_id);

  // glm::vec3 pos = glm::vec3(30 * float(rand()) / RAND_MAX - 15.f, -8.5f,
  //                        30 * float(rand()) / RAND_MAX - 15.f);

  auto entity = CreateIDEntity();
  registry.assign<TransformComponent>(entity, pos, glm::vec3(0.f),
                                      glm::vec3(1.f));
  registry.assign<PickUpComponent>(entity, pos);
  registry.assign<physics::OBB>(entity, pos, glm::vec3(1.f, 0.f, 0.f),
                                glm::vec3(0.f, 1.f, 0.f),
                                glm::vec3(0.f, 0.f, 1.f), 1.f, 1.f, 1.f);
  created_pick_ups_.push_back(entity);
  return registry.get<IDComponent>(entity).id;
}

void ServerPlayState::OverTime() {
  for (auto& [client_id, to_send] : game_server_->GetPackets()) {
    to_send << PacketBlockType::GAME_OVERTIME;
  }

  if (score_[0] > score_[1] || score_[1] > score_[0]) {
    EndGame();
  }
}

void ServerPlayState::EndGame() {
  for (auto& [client_id, to_send] : game_server_->GetPackets()) {
    to_send << PacketBlockType::GAME_END;
  }
  game_server_->ChangeState(ServerStateType::LOBBY);
}

void ServerPlayState::WallAnimation() {
  auto view_wall =
      game_server_->GetRegistry()
          .view<TimerComponent, WallComponent, TransformComponent>();

  for (auto wall : view_wall) {
    auto& timer = view_wall.get<TimerComponent>(wall);
    auto& trans = view_wall.get<TransformComponent>(wall);

    float delta = 10 - timer.time_left;
    if (delta > 1.f) delta = 1.f;
    float y = glm::lerp(-9.3f, -1.f, delta);

    trans.position.y = y;
  }
}

void ServerPlayState::ReceiveEvent(const EventInfo& e) {
  switch (e.event) {
    case Event::DESTROY_ENTITY: {
      auto& registry = game_server_->GetRegistry();
      if (registry.valid(e.entity)) {
        destroy_entities_.push_back(e.e_id);
        registry.destroy(e.entity);
      }
      break;
    }
    case Event::CREATE_CANNONBALL: {
      auto& registry = game_server_->GetRegistry();
      Projectile projectile;
      projectile.entity_id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, projectile.entity_id);
      projectile.projectile_id = ProjectileID::CANNON_BALL;
      auto& trans_c = registry.get<TransformComponent>(e.entity);
      auto& team_c = registry.get<TeamComponent>(e.entity);
      projectile.pos = trans_c.position;
      projectile.ori = trans_c.rotation;
      projectile.creator_team = team_c.team;
      created_projectiles_.push_back(projectile);

      break;
    }
    case Event::CREATE_TELEPORT_PROJECTILE: {
      auto& registry = game_server_->GetRegistry();
      Projectile projectile;
      projectile.entity_id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, projectile.entity_id);
      projectile.projectile_id = ProjectileID::TELEPORT_PROJECTILE;
      auto& trans_c = registry.get<TransformComponent>(e.entity);
      projectile.pos = trans_c.position;
      projectile.ori = trans_c.rotation;
      created_projectiles_.push_back(projectile);
      break;
    }
    case Event::CREATE_FORCE_PUSH: {
      auto& registry = game_server_->GetRegistry();
      Projectile projectile;
      projectile.entity_id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, projectile.entity_id);
      projectile.projectile_id = ProjectileID::FORCE_PUSH_OBJECT;
      auto& trans_c = registry.get<TransformComponent>(e.entity);
      projectile.pos = trans_c.position;
      projectile.ori = trans_c.rotation;
      created_projectiles_.push_back(projectile);
      break;
    }
    case Event::CREATE_MISSILE: {
      auto& registry = game_server_->GetRegistry();
      Projectile projectile;
      projectile.entity_id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, projectile.entity_id);
      projectile.projectile_id = ProjectileID::MISSILE_OBJECT;
      auto& trans_c = registry.get<TransformComponent>(e.entity);
      projectile.pos = trans_c.position;
      projectile.ori = trans_c.rotation;
      created_projectiles_.push_back(projectile);

      break;
    }
    case Event::CREATE_MINE: {
      auto& registry = game_server_->GetRegistry();

      EntityID id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, id);

      created_mines_.push_back(e.entity);

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
    case Event::CREATE_FAKE_BALL: {
      auto& registry = game_server_->GetRegistry();
      std::unordered_map<int, NetAPI::Common::Packet>& packets =
          game_server_->GetPackets();
      EntityID new_id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, new_id);

      unsigned int faker_team =
          registry.get<BallComponent>(e.entity).faker_team;

      auto view_players =
          registry.view<PlayerComponent, TeamComponent, IDComponent>();

      for (auto player : view_players) {
        auto& player_player_c = registry.get<PlayerComponent>(player);
        auto& player_team_c = registry.get<TeamComponent>(player);
        auto& player_id_c = registry.get<IDComponent>(player);

        if (player_team_c.team == faker_team) {
          packets[player_player_c.client_id] << new_id;
          packets[player_player_c.client_id]
              << PacketBlockType::CREATE_FAKE_BALL;
        } else {
          packets[player_player_c.client_id] << new_id;
          packets[player_player_c.client_id] << PacketBlockType::CREATE_BALL;
        }
      }
      break;
    }
    case Event::BUILD_WALL: {
      auto& registry = game_server_->GetRegistry();
      auto id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, id);

      created_walls_.push_back(e.entity);

      break;
    }
    case Event::SPAWNER_SPAWNED_PICKUP: {
      auto& registry = game_server_->GetRegistry();
      glm::vec3 pos(0);

      auto view_spawners =
          registry
              .view<PickupSpawnerComponent, TransformComponent, IDComponent>();
      entt::entity spawner;
      for (auto entity : view_spawners) {
        if (registry.get<IDComponent>(entity).id == e.e_id) {
          spawner = entity;
          pos = registry.get<TransformComponent>(entity).position;
        }
      }

      EntityID id = CreatePickUpComponents(pos);
      registry.get<PickupSpawnerComponent>(spawner).spawned_id = id;
      break;
    }
    case Event::CREATE_BLACK_HOLE: {
      auto& registry = game_server_->GetRegistry();
      Projectile projectile;
      projectile.entity_id = GetNextEntityGuid();
      registry.assign<IDComponent>(e.entity, projectile.entity_id);
      projectile.projectile_id = ProjectileID::BLACK_HOLE;
      auto& trans_c = registry.get<TransformComponent>(e.entity);
      projectile.pos = trans_c.position;
      projectile.ori = trans_c.rotation;
      created_projectiles_.push_back(projectile);

      break;
	}
    default:
      break;
  }
}

void ServerPlayState::CreateGoals() {
  auto& registry = game_server_->GetRegistry();
  // blue team's goal, place at red goal in world
  glm::vec3 arena_scale;
  arena_scale.x = GlobalSettings::Access()->ValueOf("ARENA_SCALE_X");
  arena_scale.y = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Y");
  arena_scale.z = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Z");
  auto entity_blue = registry.create();
  registry.assign<physics::OBB>(entity_blue, glm::vec3(0.f, 0.f, 0.f),
                                glm::vec3(1, 0, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), 2.f * arena_scale.x,
                                2.f * arena_scale.y, 6.5f * arena_scale.z);
  registry.assign<TeamComponent>(entity_blue, TEAM_BLUE);
  registry.assign<GoalComponenet>(entity_blue);
  auto& trans_comp = registry.assign<TransformComponent>(entity_blue);
  trans_comp.position =
      glm::vec3(-41.f * arena_scale.x, 2.0f * arena_scale.y, 0.f);

  // red team's goal, place at blue goal in world
  auto entity_red = registry.create();
  registry.assign<physics::OBB>(entity_red, glm::vec3(0.f, 0.f, 0.f),
                                glm::vec3(1, 0, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), 2.f * arena_scale.x,
                                2.f * arena_scale.y, 6.5f * arena_scale.z);
  registry.assign<TeamComponent>(entity_red, TEAM_RED);
  registry.assign<GoalComponenet>(entity_red);
  auto& trans_comp2 = registry.assign<TransformComponent>(entity_red);
  trans_comp2.position =
      glm::vec3(41.f * arena_scale.x, 2.f * arena_scale.y, 0.f);
}

void ServerPlayState::Reconnect(int id) {
  auto& server = game_server_->GetServer();
  auto& registry = game_server_->GetRegistry();
  NetAPI::Common::Packet to_send;
  to_send.GetHeader()->receiver = id;

  auto team_view = registry.view<TeamComponent, IDComponent, PlayerComponent>();

  unsigned int team_id = 0;
  for (auto team : team_view) {
    auto& team_c = team_view.get<TeamComponent>(team);
    auto& id_c = team_view.get<IDComponent>(team);
    auto& player_c = team_view.get<PlayerComponent>(team);

    to_send << team_c.team;
    to_send << id_c.id;
    to_send << player_c.client_id;

    if (id_c.id == clients_player_ids_[id]) {
      team_id = team_c.team;
    }
  }
  to_send << (int)team_view.size();

  to_send << team_id;

  auto ball_view = registry.view<BallComponent, IDComponent>();
  for (auto ball : ball_view) {
    auto& ball_c = ball_view.get<BallComponent>(ball);
    auto& id_c = ball_view.get<IDComponent>(ball);

    to_send << id_c.id;

    break;
  }

  to_send << clients_player_ids_[id];

  for (auto ids : clients_player_ids_) {
    to_send << ids.second;
  }
  glm::vec3 arena_scale;
  arena_scale.x = GlobalSettings::Access()->ValueOf("ARENA_SCALE_X");
  arena_scale.y = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Y");
  arena_scale.z = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Z");
  int num_players = server.GetConnectedPlayers();
  std::cout << "Num players : " << num_players << std::endl;
  to_send << num_players;
  to_send << client_abilities_[id];
  to_send << arena_scale;
  to_send << PacketBlockType::GAME_START;

  auto pick_up_view =
      registry.view<PickUpComponent, TransformComponent, IDComponent>();
  for (auto entity : pick_up_view) {
    auto& t = pick_up_view.get<TransformComponent>(entity);
    auto& id = pick_up_view.get<IDComponent>(entity);
    to_send << t.position;
    to_send << id.id;
    to_send << PacketBlockType::CREATE_PICK_UP;
  }

  server.Send(to_send);
}

void ServerPlayState::CreatePickupSpawners() {
  std::vector<glm::vec3> positions;

  std::string config_str = "PICKUPPOSITION0";
  glm::vec3 pos;
  glm::vec3 arena_scale;
  arena_scale.x = GlobalSettings::Access()->ValueOf("ARENA_SCALE_X");
  arena_scale.y = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Y");
  arena_scale.z = GlobalSettings::Access()->ValueOf("ARENA_SCALE_Z");
  pos.x = GlobalSettings::Access()->ValueOf(config_str + "X") * arena_scale.x;
  pos.y = GlobalSettings::Access()->ValueOf(config_str + "Y");
  pos.z = GlobalSettings::Access()->ValueOf(config_str + "Z") * arena_scale.z;
  positions.push_back(pos);
  pos.x *= -1;
  positions.push_back(pos);
  pos.z *= -1;
  positions.push_back(pos);
  pos.x *= -1;
  positions.push_back(pos);

  auto& registry = this->game_server_->GetRegistry();

  for (int i = 0; i < 4; i++) {
    auto spawner = registry.create();

    auto& spawner_c = registry.assign<PickupSpawnerComponent>(spawner);
    auto& trans_c = registry.assign<TransformComponent>(spawner);
    registry.assign<IDComponent>(spawner, GetNextEntityGuid());
    trans_c.position = positions[i];
    spawner_c.override_respawn = true;
  }
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
  if (!this->record_ && !this->replay_) {
    // Create Replay Machine
    std::cout << "Recording...\n";
    this->replay_machine_ =
        new ReplayMachine(in_replay_length_seconds, kServerUpdateRate, 1,
                          (this->blue_players_ + this->red_players_), false);
    this->record_ = true;
    return true;  // NTS: Return false if recording cannot start?
  }
  return false;
}

void ServerPlayState::Record(std::bitset<10>& in_bitset, float& in_x_value,
                             float& in_y_value, const float& in_dt,
                             unsigned int in_player_index) {
  auto& registry = this->game_server_->GetRegistry();

  // Save the frame with the ReplayMachine
  if (this->replay_machine_->SaveReplayFrame(in_bitset, in_x_value, in_y_value,
                                             registry, in_dt,
                                             in_player_index)) {
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
                             float& in_y_value, unsigned int in_player_index) {
  auto& registry = this->game_server_->GetRegistry();
  // Ensure we are not recording the replay
  this->record_ = false;
  // Read a frame with the ReplayMachine
  if (this->replay_machine_->LoadReplayFrame(in_bitset, in_x_value, in_y_value,
                                             registry, in_player_index)) {
    std::cout << "Finished.\n";
    // If true is returned it means the internal
    // BiPack has been fully read and there is no
    // more data to replay.
    // Turn off replaying.
    this->replay_ = false;

    // Once replay has been played, remove the replay machine
    delete this->replay_machine_;
    this->replay_machine_ = nullptr;
  }
}
//---