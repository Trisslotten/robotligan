#include "gameserver.hpp"

#include <algorithm>
#include <bitset>
#include <glob/graphics.hpp>
#include <iostream>
#include <numeric>

#include "shared/id_component.hpp"
#include "shared/pick_up_component.hpp"
#include "shared/shared.hpp";
#include "shared/transform_component.hpp"

#include "ecs/components.hpp"
#include "ecs/systems/ability_controller_system.hpp"
#include "ecs/systems/buff_controller_system.hpp"
#include "ecs/systems/collision_system.hpp"
#include "ecs/systems/goal_system.hpp"
#include "ecs/systems/missile_system.hpp"
#include "ecs/systems/physics_system.hpp"
#include "ecs/systems/player_controller_system.hpp"
#include "ecs/systems/target_system.hpp"

namespace {}  // namespace

GameServer::~GameServer() {}

void GameServer::Init(double in_update_rate) {
  glob::SetModelUseGL(false);

  server_.Setup(1337);

  lobby_state_.SetGameServer(this);
  play_state_.SetGameServer(this);
  lobby_state_.Init();
  current_state_ = &lobby_state_;
  srand(time(NULL));
  pings_.resize(NetAPI::Common::kMaxPlayers);
  // CreateEntities();
}

void GameServer::Update(float dt) {
  server_.Update();

  packets_.clear();
  for (auto& [client_id, client_data] : server_.GetClients()) {
    packets_[client_id] = NetAPI::Common::Packet();
  }

  for (auto client_data : server_.GetNewlyConnected()) {
    lobby_state_.SetClientIsReady(client_data->ID, false);
    play_state_.SetClientReceiveUpdates(client_data->ID, false);
    lobby_state_.HandleNewClientTeam(client_data->ID);
    NetAPI::Common::Packet p;
    int s;
    if (this->current_state_type_ == ServerStateType::LOBBY) {
      s = 0;
    } else {
      NetAPI::Common::Packet to_send;
      for (auto client_team : lobby_state_.client_teams_) {
        to_send << client_team.first;   // send id
        to_send << client_team.second;  // send team
        bool ready = true;
        to_send << ready;
        to_send << PacketBlockType::LOBBY_UPDATE_TEAM;
      }
      server_.Send(to_send);
      s = 1;
    }
    p << s << PacketBlockType::STATE;
    server_.Send(p);
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
  DoOncePerSecond();
  current_state_->Update(dt);

  //---------------------------------------------
  //--------------UPDATE GAME LOGIC--------------
  //---------------------------------------------
  UpdateSystems(dt);

  /*
  TODO: fix
    // send new teams
    for (auto& p : new_teams_) {
      to_send << p.second;
      to_send << p.first;
      to_send << PacketBlockType::CHOOSE_TEAM;
    }
    */
  HandleStateChange();

  HandlePacketsToSend();

  messages.clear();
}

void GameServer::HandlePacketsToSend() {
  for (auto& [id, client_data] : server_.GetClients()) {
    NetAPI::Common::Packet to_send;
    auto header = to_send.GetHeader();
    header->receiver = id;

    if (!packets_[id].IsEmpty()) {
      to_send << packets_[id];
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

    // send events
    for (auto event : game_events_) {
      to_send << event;
      to_send << PacketBlockType::GAME_EVENT;
    }

    if (!to_send.IsEmpty()) {
      server_.Send(to_send);
    }
  }
  game_events_.clear();
}

void GameServer::HandleStateChange() {
  // handle state change
  if (wanted_state_type_ != current_state_type_) {
    bool went_from_lobby_to_play =
        current_state_type_ == ServerStateType::LOBBY &&
        wanted_state_type_ == ServerStateType::PLAY;

    bool went_from_play_to_lobby =
        current_state_type_ == ServerStateType::PLAY &&
        wanted_state_type_ == ServerStateType::LOBBY;

    current_state_type_ = wanted_state_type_;

    std::unordered_map<int, unsigned int> client_teams_lobby;
    std::unordered_map<int, AbilityID> client_abilities_lobby;
    if (went_from_lobby_to_play) {
      client_teams_lobby = lobby_state_.client_teams_;
      client_abilities_lobby = lobby_state_.client_abilities_;
    }

    current_state_->Cleanup();
    switch (wanted_state_type_) {
      case ServerStateType::LOBBY:
        std::cout << "Change Server State: LOBBY\n";
        current_state_ = &lobby_state_;
        break;
      case ServerStateType::PLAY:
        std::cout << "Change Server State: PLAY\n";
        if (went_from_lobby_to_play) {
          play_state_.client_teams_ = client_teams_lobby;
          play_state_.client_abilities_ = client_abilities_lobby;
        }
        if (went_from_play_to_lobby) {
          lobby_state_.SetTeamsUpdated(true);
        }
        current_state_ = &play_state_;
        break;
    }
    current_state_->Init();
    NetAPI::Common::Packet p;
    int s;
    if (this->current_state_type_ == ServerStateType::LOBBY) {
      s = 0;
    } else {
      s = 1;
    }
    p << s << PacketBlockType::STATE;
    server_.Send(p);
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
      if (start_replay) play_state_.StartRecording(10);
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
    case PacketBlockType::CLIENT_RECEIVE_UPDATES: {
      bool receive = false;
      packet >> receive;
      play_state_.SetClientReceiveUpdates(client_id, receive);
      break;
    }

    case PacketBlockType::LOBBY_SELECT_TEAM: {
      unsigned int team = 0;
      packet >> team;
      lobby_state_.SetClientTeam(client_id, team);
      lobby_state_.SetClientIsReady(client_id, false);
      break;
    }
    case PacketBlockType::PING: {
      int challenge = 0;
      packet >> challenge;
      auto now = std::chrono::steady_clock::now();
      auto before = server_.GetClients().at(client_id)->last_time;
      auto id = server_.GetClients().at(client_id)->ping_id;
      if (id > NetAPI::Socket::kAveragePingCount - 1) {
        server_.GetClients().at(client_id)->ping_id = 0;
        id = 0;
      }
      server_.GetClients().at(client_id)->ping[id] =
          std::chrono::duration_cast<std::chrono::milliseconds>(now - before)
              .count();
      server_.GetClients().at(client_id)->ping_sum = 0;
      for (auto& v : server_.GetClients().at(client_id)->ping) {
        server_.GetClients().at(client_id)->ping_sum += v;
      }
      if (challenge > 0) {
        // Failed ping
      } else {
        server_.GetClients().at(client_id)->last_time = now;
      }
      break;
    }
    case PacketBlockType::LOBBY_SELECT_ABILITY: {
      AbilityID id;
      packet >> id;
      lobby_state_.SetClientAbility(client_id, id);
      break;
    }
    case PacketBlockType::FRAME_ID: {
      int id;
      packet >> id;
      play_state_.SetFrameID(client_id, id);
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

void GameServer::ReceiveGameEvent(const GameEvent& event) {
  game_events_.push_back(event);
  if (event.type == GameEvent::GOAL) {
    play_state_.StartResetTimer();
  }
}

void GameServer::UpdateSystems(float dt) {
  player_controller::Update(registry_, dt);
  ability_controller::Update(registry_, dt);
  buff_controller::Update(registry_, dt);
  target_system::Update(registry_);
  missile_system::Update(registry_, dt);

  UpdatePhysics(registry_, dt);
  UpdateCollisions(registry_);
  if (!play_state_.IsResetting()) goal_system::Update(registry_);

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
    case Event::CREATE_TELEPORT_PROJECTILE: {
      break;
    }
    default:
      break;
  }
}

void GameServer::DoOncePerSecond() {
  /*
          Broadcast client pings to all clients
  */
  NetAPI::Common::Packet p;
  p.GetHeader()->receiver = NetAPI::Socket::EVERYONE;
  auto& data = server_.GetClients();
  pings_.resize(NetAPI::Common::kMaxPlayers);
  for (auto cli : data) {
    pings_[cli.first] = cli.second->ping_sum;
  }
  unsigned size = pings_.size();
  p.Add(pings_.data(), size);
  p << size;
  p << PacketBlockType::PING_RECIEVE;
  server_.Send(p);
}
