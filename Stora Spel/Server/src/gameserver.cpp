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
#include "ecs/systems/lifetime_system.hpp"
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

  // very annoying thing
  ability_cooldowns_[AbilityID::BUILD_WALL] =
      GlobalSettings::Access()->ValueOf("ABILITY_BUILD_WALL_COOLDOWN");
  ability_cooldowns_[AbilityID::FAKE_BALL] =
      GlobalSettings::Access()->ValueOf("ABILITY_FAKE_BALL_COOLDOWN");
  ability_cooldowns_[AbilityID::FORCE_PUSH] =
      GlobalSettings::Access()->ValueOf("ABILITY_FORCE_PUSH_COOLDOWN");
  ability_cooldowns_[AbilityID::GRAVITY_CHANGE] =
      GlobalSettings::Access()->ValueOf("ABILITY_GRAVITY_COOLDOWN");
  ability_cooldowns_[AbilityID::HOMING_BALL] =
      GlobalSettings::Access()->ValueOf("ABILITY_HOMING_BALL_COOLDOWN");
  ability_cooldowns_[AbilityID::INVISIBILITY] =
      GlobalSettings::Access()->ValueOf("ABILITY_INVISIBILITY_COOLDOWN");
  ability_cooldowns_[AbilityID::MISSILE] =
      GlobalSettings::Access()->ValueOf("ABILITY_MISSILE_COOLDOWN");
  ability_cooldowns_[AbilityID::SUPER_STRIKE] =
      GlobalSettings::Access()->ValueOf("ABILITY_SUPER_STRIKE_COOLDOWN");
  ability_cooldowns_[AbilityID::SWITCH_GOALS] =
      GlobalSettings::Access()->ValueOf("ABILITY_SWITCH_GOALS_COOLDOWN");
  ability_cooldowns_[AbilityID::TELEPORT] =
      GlobalSettings::Access()->ValueOf("ABILITY_TELEPORT_COOLDOWN");

  ability_controller::ability_cooldowns = ability_cooldowns_;

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
    ServerStateType state;
    if (this->current_state_type_ == ServerStateType::PLAY) {
      NetAPI::Common::Packet to_send;
      for (auto client_team : lobby_state_.client_teams_) {
        std::string name = GetClientNames()[client_team.first];

        to_send.Add(name.c_str(), name.size());
        to_send << name.size();
        to_send << client_team.first;   // send id
        to_send << client_team.second;  // send team
        bool ready = true;
        to_send << ready;
        to_send << PacketBlockType::LOBBY_UPDATE_TEAM;
      }
      server_.Send(to_send);
      // s = 1;
    }
    p << this->current_state_type_ << PacketBlockType::SERVER_STATE;
    server_.Send(p);
  }

  /*
  TODO: fix
  if (client_sent_name_ && current_state_type_ == ServerStateType::PLAY) {
    for (auto& [client_id, to_send] : GetPackets()) {
      for (auto [cl_id, name] : client_names_) {
        std::cout << "TO_CLIENT_SEND: " << name << "\n";
        to_send.Add(name.data(), name.size());
        to_send << name.size();
        to_send << cl_id;
        to_send << PacketBlockType::TO_CLIENT_NAME;
      }
    }
  }
  client_sent_name_ = false;
  */

  // handle received data
  server_.Lock();
  for (auto& [id, client_data] : server_.GetClients()) {
    for (auto& packet : client_data->packets) {
      while (!packet.IsEmpty()) {
        int16_t block_type = -1;
        packet >> block_type;
        HandlePacketBlock(packet, block_type, id);
      }
    }
	server_.ClearPackets(client_data);
  }
  DoOncePerSecond();
  current_state_->Update(dt);
  current_state_->HandleDataToSend();
  server_.Unlock();
  //---------------------------------------------
  //--------------UPDATE GAME LOGIC--------------
  //---------------------------------------------
  UpdateSystems(dt);

  HandleStateChange();

  HandlePacketsToSend();

  messages.clear();
}

void GameServer::HandlePacketsToSend() {
  for (auto& [id, client_data] : server_.GetClients()) {
    NetAPI::Common::Packet to_send;
    auto header = to_send.GetHeader();
    header->receiver = id;

    // send events
    for (auto event : game_events_) {
      to_send << event;
      to_send << PacketBlockType::GAME_EVENT;
    }

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
    p << this->current_state_type_ << PacketBlockType::SERVER_STATE;
    server_.Send(p);
    
    if (went_from_play_to_lobby)
      client_names_.clear();
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
      Message message;
      message.name = client_names_[client_id] + ": ";
      message.message = str;
      auto view_player = registry_.view<TeamComponent, PlayerComponent>();
      bool found_name = false;
      for (auto player : view_player) {
        auto& team_c = view_player.get<TeamComponent>(player);
        auto& player_c = view_player.get<PlayerComponent>(player);
        if (client_id == player_c.client_id) {
          message.message_from = team_c.team;
          found_name = true;
          break;
        }
      }
      if (!found_name) {
        message.message_from = lobby_state_.client_teams_[client_id];
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
    case PacketBlockType::MY_NAME: {
      std::string name;
      size_t len;
      packet >> len;
      name.resize(len);
      packet.Remove(name.data(), len);
      if (client_names_[client_id] != name) {
        while (NameAlreadyExists(name)) {
          name.append("xD");
        }
        client_names_[client_id] = name;
        lobby_state_.SetTeamsUpdated(true);
        this->client_sent_name_ = true;
      }
      break;
    }
  }
}

bool GameServer::NameAlreadyExists(std::string name) {
  for (auto n : client_names_) {
    if (name == n.second) {
      return true;
    }
  }
  return false;
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
  lifetime::Update(registry_, dt);

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
