#include <NetAPI/socket/server.hpp>
#include <iostream>
#include <string>

using namespace std::chrono_literals;
void NetAPI::Socket::Server::ClearPackets(NetAPI::Socket::ClientData* data) {
  data->packets.clear();
}
void NetAPI::Socket::Server::SendPing() {
  NetAPI::Common::Packet to_send;
  to_send.GetHeader()->packet_id = NetAPI::Socket::EVERYONE;
  int random_number = -1 + std::rand() % ((-1000 - 1) + 1);
  to_send << random_number << PacketBlockType::PING;
  this->SendToAll(to_send);
}
void NetAPI::Socket::Server::HandleClientPacket() {
  // std::pair<const long, NetAPI::Socket::ClientData *> s
}
void NetAPI::Socket::Server::Receive() {
  for (auto& c : client_data_) {
    if (c.second && !c.second->client.IsConnected() && c.second->is_active) {
      std::cout << "DEBUG: removing client, lstrecvlen="
                << c.second->client.GetRaw()->GetLastRecvLen()
                << ", isConnected=" << c.second->client.IsConnected() << "\n";
      c.second->client.SetDisconnected(true);
      c.second->client.Disconnect();
      c.second->is_active = false;
      // connected_players_--;
      continue;
    }
    if (c.second && c.second->client.IsConnected()) {
      int num_packets = 0;
      for (auto packet : c.second->client.Receive()) {
        num_packets++;
        if (!packet.IsEmpty()) {
          c.second->packets.push_back(packet);
        }
      }
    }
  }
}
void NetAPI::Socket::Server::ListenForClients() {
  newly_connected_.clear();
  // Accept client
  if (!connection_client_) {
    connection_client_ = new ClientData();
  }
  auto s = listener_.Accept(connection_client_->client.GetRaw());
  bool accepted = false;
  if (s) {
    sockaddr_in client_addr{};
    int size = sizeof(client_addr);
    auto ret =
        getpeername(connection_client_->client.GetRaw()->GetLowLevelSocket(),
                    (sockaddr*)&client_addr, &size);
    char buffer[14];
    inet_ntop(AF_INET, &client_addr.sin_addr, buffer, 14);
    auto addr = buffer;
    auto port = ntohs(client_addr.sin_port);

    std::string address;

    if (kEnableReconnect) {
      // För att kunna reconnecta till pågående match,
      // address = addr;
    } else {
      // för att kunna testa många klienter
      address = addr + (":" + std::to_string(port));
    }

    // std::string address = "";
    std::cout << "DEBUG: tcp connection accepted: "
              << addr + (":" + std::to_string(port)) << "\n";

    auto packets = connection_client_->client.Receive();
    for (auto& packet : packets) {
      int16_t block_type = -1;
      packet >> block_type;
      if (block_type == PacketBlockType::HWID) {
        size_t size;
        packet >> size;
        std::string str;
        str.resize(size);
        packet.Remove(str.data(), size);
        if(kEnableReconnect) {
          address = str;  // Kommentera ut detta om du vill debugga två
        }
        // instanser
        break;
      }
    }
    std::cout << "Client ID: " << address << std::endl;
    if (address == "") {
      accepted = false;
      int can_join = -3;
      NetAPI::Common::Packet p;
      p << can_join << PacketBlockType::SERVER_CAN_JOIN;
      connection_client_->client.Send(p);
      connection_client_->client.Disconnect();
      connection_client_->is_active = false;
      connection_client_ = nullptr;
      return;
    }
    connection_client_->packets.clear();
    auto find_res = ids_.find(address);
    // if already found
    if (find_res != ids_.end()) {
      accepted = true;
      auto client_data = client_data_[find_res->second];
      client_data->client.Disconnect();
      delete client_data;
      connection_client_->ID = find_res->second;
      connection_client_->reconnected = true;
      client_data_[find_res->second] = connection_client_;
      std::cout << "DEBUG: Found existing client, overwriting\n";
    } else if (GetNumConnected() < max_players_) {
      std::cout << "DEBUG: adding new client\n";
      std::cout << "DEBUG: adding new client\n";

      connection_client_->ID = current_client_guid_;
      ids_[address] = current_client_guid_;
      ids_rev_[current_client_guid_] = address;
      client_data_[current_client_guid_] = connection_client_;

      //connected_players_++;
      //game_players_++;
      current_client_guid_++;
      accepted = true;
    } else {
      std::cout << "DEBUG: Server full, disconnecting" << std::endl;
      NetAPI::Common::Packet p;
      int canJoin = -2;
      p << canJoin << PacketBlockType::SERVER_CAN_JOIN;
      connection_client_->client.Send(p);
    }
    if (accepted) {
      int can_join = 2;
      NetAPI::Common::Packet p;
      p << can_join << PacketBlockType::SERVER_CAN_JOIN;
      connection_client_->client.Send(p);
      connection_client_->address = address;
      connection_client_->is_active = true;
      newly_connected_.push_back(connection_client_);
    } else {
      int can_join = -2;
      NetAPI::Common::Packet p;
      p << can_join << PacketBlockType::SERVER_CAN_JOIN;
    }
    connection_client_ = nullptr;
  }
}
void NetAPI::Socket::Server::SendStoredData() {
  SendPing();
  NetAPI::Common::PacketHeader header;
  for (auto& d : data_to_send_) {
    d >> header;
    if (header.receiver == EVERYONE) {
      for (auto& cli : client_data_) {
        cli.second->client.Send(d);
      }
    } else {
      auto result = client_data_.find(header.receiver);
      if (result != client_data_.end()) {
        auto c = client_data_[header.receiver];
        c->client.Send(d);
      }
    }
  }

  data_to_send_.clear();
}
unsigned short getHashedID(char* addr, unsigned short port) {
  unsigned short retval = 0;
  std::string s(addr);

  for (auto c : s) {
    retval += (unsigned int)c;
  }
  return retval;
}
bool NetAPI::Socket::Server::Setup(unsigned short port,
                                   unsigned short maxplayers) {
  if (!setup_) {
    if (listener_.Bind(port)) {
      setup_ = true;
      std::cout << "Server bound at port: " << port << std::endl;
    }
    client_data_.reserve(maxplayers);
    max_players_ = maxplayers;
  }
  return setup_;
}

bool NetAPI::Socket::Server::Update() {
  if (!setup_) {
    return false;
  }
  ListenForClients();
  // Receive Data
  Receive();
  // Send Data
  SendStoredData();

  // CHECK DC
  for (auto& [id, client_data] : client_data_) {
    if (client_data->client.TimeSinceLastUpdate() > 3000) {
      client_data->client.Disconnect();
    }
  }

  for(auto client_id : client_to_remove_) {
    //std::cout << "!!!!!!!!!!!!!!!! Remove client: " << client_id << "\n";
    client_data_.erase(client_id);
  }
  client_to_remove_.clear();

  return true;
}

void NetAPI::Socket::Server::SendToAll(const char* data, size_t len) {
  data_to_send_.push_back(NetAPI::Common::Packet(data, len));
}

void NetAPI::Socket::Server::SendToAll(NetAPI::Common::Packet& p) {
  data_to_send_.push_back(p);
}

void NetAPI::Socket::Server::Send(unsigned id, const char* data, size_t len) {
  NetAPI::Common::Packet p(data, len);
  NetAPI::Common::PacketHeader h;
  h.receiver = id;
  p << h;
  Send(p);
}

void NetAPI::Socket::Server::Send(NetAPI::Common::Packet& p) {
  data_to_send_.push_back(p);
}

bool NetAPI::Socket::Server::KickPlayer(long ID) {
  auto id_res = ids_rev_.find(ID);
  if(id_res != ids_rev_.end()) {
    ids_.erase(id_res->second);
    ids_rev_.erase(id_res);
  }

  auto it = client_data_.find(ID);
  if (it != client_data_.end()) {
    client_to_remove_.push_back(ID);
    return true;
  } else {
    return false;
  }
}
