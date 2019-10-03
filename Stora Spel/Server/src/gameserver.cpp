#include "gameserver.hpp"

#include <iostream>
#include <algorithm>
#include <bitset>
#include <glob/graphics.hpp>
#include <iostream>
#include <transform_component.hpp>
#include <pick_up_component.hpp>

#include "shared/shared.hpp";
#include "shared/transform_component.hpp"

#include "ecs/components.hpp"
#include "ecs/systems/ability_controller_system.hpp"
#include "ecs/systems/buff_controller_system.hpp"
#include "ecs/systems/collision_system.hpp"
#include "ecs/systems/goal_system.hpp"
#include "ecs/systems/physics_system.hpp"
#include "ecs/systems/player_controller_system.hpp"

namespace {}  // namespace

GameServer::~GameServer() {
  if (this->replay_machine_ != nullptr) {
    delete this->replay_machine_;
  }
}

void GameServer::Init(double in_update_rate) {
  glob::SetModelUseGL(false);

  GlobalSettings::Access()->UpdateValuesFromFile();

  server_.Setup(1337);

  lobby_state_.SetGameServer(this);
  play_state_.SetGameServer(this);
  lobby_state_.Init();
  current_state_ = &lobby_state_;
  srand(time(NULL));

  //CreateEntities();
  CreateGoals();

  // Create replay machine
  this->replay_machine_ = new ReplayMachine();
  this->update_rate_ = in_update_rate;
}

void GameServer::Update(float dt) {
  server_.Update();

  for(auto client_data : server_.GetNewlyConnected()) {
    lobby_state_.SetClientIsReady(client_data->ID, false);
  }

  // handle received data
  for (auto& [id, client_data] : server_.GetClients()) {
    for (auto& packet : client_data->packets) {
      while (!packet.IsEmpty()) {
        int16_t block_type = -1;
        packet >> block_type;
        HandlePacketBlock(packet, block_type, id);
      }
    }
    client_data->packets.clear();
  }
  /*
  //---------------------------------------------
  //-----------HANDLE PLAYER ACTIONS-------------
  //---------------------------------------------
  auto player_view = registry_.view<PlayerComponent>();
  registry_.view<PlayerComponent>().each(
      [&](auto entity, PlayerComponent& player_c) {
        // Check if we are taking inputs from the
        // client or if we are reading them from a replay
        if (!this->replay_) {
          auto inputs = players_inputs_[player_c.id];
          player_c.actions = inputs.first;
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
  players_inputs_.clear();
  */

  current_state_->Update();

  //---------------------------------------------
  //--------------UPDATE GAME LOGIC--------------
  //---------------------------------------------
  UpdateSystems(dt);

  //auto pick_up_events = registry_.view<PickUpEvent>();
  /*
   auto view_player = registry_.view<PlayerComponent>();
    for (auto player : view_player) {
      auto& player_c = view_player.get(player);
      if (id == player_c.id) {
        to_send << player_c.energy_current;
        break;
      }
    }
    to_send << PacketBlockType::PLAYER_STAMINA;

    auto view_players2 =
        registry_.view<PlayerComponent, TeamComponent, PointsComponent>();

    for (auto player : view_players2) {
      auto& player_player_c = registry_.get<PlayerComponent>(player);
      auto& player_points_c = registry_.get<PointsComponent>(player);
      auto& player_team_c = registry_.get<TeamComponent>(player);

      if (player_points_c.changed) {
        to_send << player_team_c.team;
        to_send << player_player_c.id;
        to_send << player_points_c.GetPoints();
        to_send << player_points_c.GetGoals();
        to_send << PacketBlockType::UPDATE_POINTS;
      }
    }
    
    for (auto entity : pick_ups_) {
      auto& t = registry_.get<TransformComponent>(entity);
      to_send << t.position;
      to_send << PacketBlockType::CREATE_PICK_UP;
    }

    for (auto entity : pick_up_events) {
      auto& pick_event = pick_up_events.get(entity);
      to_send << 0;
      to_send << PacketBlockType::DESTROY_PICK_UP;
    
      if (id == pick_event.player_id) {
        to_send << pick_event.ability_id;
        to_send << PacketBlockType::RECEIVE_PICK_UP;
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
      if (goal_goal_c
              .switched_this_tick) {  // MAY NEED TO CHANGE, NOT A GOOD SOLUTION
        if (!sent_switch) {
          to_send << PacketBlockType::SWITCH_GOALS;
          sent_switch = true;
        }
      }
    }

    // send individual player scores
    auto view_players2 =
        registry_.view<PlayerComponent, TeamComponent, PointsComponent>();

    for (auto player : view_players2) {
      auto& player_player_c = registry_.get<PlayerComponent>(player);
      auto& player_points_c = registry_.get<PointsComponent>(player);
      auto& player_team_c = registry_.get<TeamComponent>(player);

      if (player_points_c.changed) {
        to_send << player_team_c.team;
        to_send << player_player_c.id;
        to_send << player_points_c.GetPoints();
        to_send << player_points_c.GetGoals();
        to_send << PacketBlockType::UPDATE_POINTS;
      }
    }

    // send messages
    for (auto m : messages) {
      to_send.Add(m.name.c_str(), m.name.size());
      to_send << m.name.size();
      to_send.Add(m.message.c_str(), m.message.size());
      to_send << m.message.size();
      to_send << m.message_from;
      to_send << PacketBlockType::MESSAGE;
	  }
    // send new teams
    for (auto& p : new_teams_) {
      to_send << p.second;
      to_send << p.first;
      to_send << PacketBlockType::CHOOSE_TEAM;
    }
    */

  // handle state change
  if (wanted_state_type_ != current_state_type_) {
    current_state_type_ = wanted_state_type_;
    current_state_->Cleanup();
    switch (wanted_state_type_) {
      case ServerStateType::LOBBY:
        std::cout << "Change Server State: LOBBY\n";
        current_state_ = &lobby_state_;
        break;
      case ServerStateType::PLAY:
        std::cout << "Change Server State: PLAY\n";
        current_state_ = &play_state_;
        break;
    }
    current_state_->Init();
  }
}

void GameServer::HandlePacketBlock(NetAPI::Common::Packet& packet,
                                   int16_t block_type, int client_id) {
  switch (block_type) {
    case PacketBlockType::INPUT: {
      uint16_t actions = 0;
      float pitch = 0.f;
      float yaw = 0.f;
      packet >> yaw;
      packet >> pitch;
      packet >> actions;
      play_state_.SetPlayerInput(client_id, actions, pitch, yaw);
      // std::cout << "PACKET: INPUT, " << actions << ", " << yaw << ", " <<
      // pitch << "\n";
      break;
    }
        case PacketBlockType::CLIENT_READY: {
      lobby_state_.SetClientIsReady(client_id, true);
      std::cout << "PACKET: CLIENT_READY: " << client_id << "\n";
      break;
    }
    case PacketBlockType::CLIENT_NOT_READY: {
      lobby_state_.SetClientIsReady(client_id, false);
      break;
    }
    
    case PacketBlockType::TEST_REPLAY_KEYS: {
      // If P is pressed, record 10 seconds
      bool start_replay;
      packet >> start_replay;
      if (start_replay && !this->record_) {
        this->StartRecording(10);
      }
      break;
    }
    case PacketBlockType::MESSAGE: {
      size_t strsize = 0;
      packet >> strsize;
      std::string str;
      str.resize(strsize);
      packet.Remove(str.data(), strsize);
      int player_id = id + 1;
      Message message;
      message.name = "player " + std::to_string(player_id) + ": ";
      message.message = str;
      auto view_player = registry_.view<TeamComponent, PlayerComponent>();
      for (auto player : view_player) {
        auto& team_c = view_player.get<TeamComponent>(player);
        auto& player_c = view_player.get<PlayerComponent>(player);
        if (id == player_c.id) {
          message.message_from = team_c.team;
          break;
        }
      }
      messages.push_back(message);
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
  if (new_teams_.size() == 0) return;

  auto player_view = registry_.view<PlayerComponent, TeamComponent>();
  new_teams_.erase(std::remove_if(new_teams_.begin(), new_teams_.end(),
       [&](std::pair<PlayerID, unsigned int> pair) {
    if (pair.second < 2) {
      for (auto entity : player_view) {
        auto& player_c = player_view.get<PlayerComponent>(entity);
        auto& team_c = player_view.get<TeamComponent>(entity);
      
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
    }
),new_teams_.end());
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
  AbilityID primary_id = AbilityID::SWITCH_GOALS;
  AbilityID secondary_id = AbilityID::SWITCH_GOALS;
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

  // START ---------- Buff component [MOVE TO PICK-UP EVENT] ----------
  // Available buffs: SPEED_BOOST, JUMP_BOOST, INFINITE_STAMINA 
  BuffID buff_id = SPEED_BOOST;
  float buff_duration =
      GlobalSettings::Access()->ValueOf("BUFF_SPEED_BOOST_DURATION");

  // Add component for a player
  registry_.assign<BuffComponent>(entity,                     // Entity
                                  buff_id,                    // Active buff
                                  true,                       // Toggle buff
                                  buff_duration,              // Buff duration
                                  0.0f                        // Remaining duration
  );
  // END ---------- Buff component [MOVE TO PICK-UP EVENT] ----------

  if (last_spawned_team_ == 1) {
    registry_.assign<TeamComponent>(entity, TEAM_BLUE);
    last_spawned_team_ = 0;
    blue_players_++;
  } else {
    registry_.assign<TeamComponent>(entity, TEAM_RED);
    last_spawned_team_ = 1;
    red_players_++;
  }

  registry_.assign<PointsComponent>(entity);

  auto& player_component = registry_.assign<PlayerComponent>(entity);
  player_component.id = id;
  created_players_.push_back(id);

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
  auto player_view = registry_.view<PlayerComponent, PhysicsComponent,
                                    TransformComponent, CameraComponent>();
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
  
  // reset pick-up
  auto pick_up_view = registry_.view<PickUpComponent>();
  for (auto entity : pick_up_view) registry_.destroy(entity);

  CreatePickUpComponents();

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

void GameServer::UpdateSystems(float dt) {
  player_controller::Update(registry_, dt);
  ability_controller::Update(registry_, dt);

  for (auto& v : md.pos) v = matrix * glm::vec4(v, 1.f);
  auto& mh = registry_.assign<physics::MeshHitbox>(entity, std::move(md.pos),
                                                   std::move(md.indices));
  // glob::LoadWireframeMesh(model_arena, mh.pos, mh.indices);
}

void GameServer::CreatePickUpComponents() {
  glm::vec3 pos = glm::vec3(3.f, -2.f, 3.f);
  auto entity = registry_.create();
  registry_.assign<TransformComponent>(entity, pos, glm::vec3(0.f),
                                       glm::vec3(1.f));
  registry_.assign<PickUpComponent>(entity);
  registry_.assign<physics::OBB>(entity, pos, glm::vec3(1.f, 0.f, 0.f),
                                 glm::vec3(0.f, 1.f, 0.f),
                                 glm::vec3(0.f, 0.f, 1.f),
                                  1.f, 1.f, 1.f);

  pick_ups_.push_back(entity);
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


// Replay stuff---
bool GameServer::StartRecording(unsigned int in_replay_length_seconds) {
  // Initiate the Replay Machine
  std::cout << "Recording...\n";
  this->replay_machine_->Init(in_replay_length_seconds, this->update_rate_, 1);
  this->record_ = true;
  return true;  // NTS: Return false if recording cannot start?
}

void GameServer::Record(std::bitset<10>& in_bitset, float& in_x_value,
                        float& in_y_value, const float& in_dt) {
  // Save the frame with the ReplayMachine
  if (this->replay_machine_->SaveReplayFrame(in_bitset, in_x_value, in_y_value,
                                             this->registry_, in_dt)) {
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

void GameServer::Replay(std::bitset<10>& in_bitset, float& in_x_value,
                        float& in_y_value) {
  // Ensure we are not recording the replay
  this->record_ = false;
  // Read a frame with the ReplayMachine
  if (this->replay_machine_->LoadReplayFrame(in_bitset, in_x_value, in_y_value,
                                             this->registry_)) {
    std::cout << "Finished.\n";
    // If true is returned it means the internal
    // BiPack has been fully read and there is no
    // more data to replay.
    // Turn off replaying.
    this->replay_ = false;
  }
}
//---


void GameServer::ReceiveEvent(const EventInfo& e) {
  switch (e.event) {
    case Event::DESTROY_ENTITY: {
      break;
    }
    case Event::CREATE_CANNONBALL: {
      break;
	}
    default:
      break;
  }
}
