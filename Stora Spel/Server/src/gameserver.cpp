#include "gameserver.hpp"

#include <algorithm>
#include <bitset>
#include <glob/graphics.hpp>
#include <iostream>

#include "shared/pick_up_component.hpp"
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

  // CreateEntities();

}

void GameServer::Update(float dt) {
  server_.Update();

  for (auto client_data : server_.GetNewlyConnected()) {
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

  */

  current_state_->Update(dt);

  //---------------------------------------------
  //--------------UPDATE GAME LOGIC--------------
  //---------------------------------------------
  UpdateSystems(dt);

  // auto pick_up_events = registry_.view<PickUpEvent>();
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
      if(start_replay)
        play_state_.StartRecording(10);
      break;
    }
    case PacketBlockType::MESSAGE: {
      size_t strsize = 0;
      packet >> strsize;
      std::string str;
      str.resize(strsize);
      packet.Remove(str.data(), strsize);
      int player_id = client_id + 1;
      Message message;
      message.name = "player " + std::to_string(player_id) + ": ";
      message.message = str;
      auto view_player = registry_.view<TeamComponent, PlayerComponent>();
      for (auto player : view_player) {
        auto& team_c = view_player.get<TeamComponent>(player);
        auto& player_c = view_player.get<PlayerComponent>(player);
        if (client_id == player_c.client_id) {
          message.message_from = team_c.team;
          break;
        }
      }
      messages.push_back(message);
      break;
    }
      /*
      TODO: fix
      case PacketBlockType::CHOOSE_TEAM: {
        PlayerID pid;
        unsigned int team;
        packet >> pid;
        packet >> team;

        new_teams_.push_back({pid, team});
      }
      */
  }
}

void GameServer::UpdateSystems(float dt) {
  player_controller::Update(registry_, dt);
  ability_controller::Update(registry_, dt);
  buff_controller::Update(registry_, dt);

  UpdatePhysics(registry_, dt);
  UpdateCollisions(registry_);
  if (goal_system::Update(registry_)) {
    play_state_.ResetEntities();
  }
  dispatcher.update<EventInfo>();
  // glob::LoadWireframeMesh(model_arena, mh.pos, mh.indices);
}


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
