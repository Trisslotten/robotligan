#include <NetAPI/socket/server.hpp>
#include <iostream>
#include <string>
unsigned short getHashedID(char* addr, unsigned short port) {
  unsigned short retval = 0;
  std::string s(addr);
  for (auto c : s) {
    retval += (unsigned int)c;
  }
  return retval;
}
bool NetAPI::Socket::Server::Setup(unsigned short port) {
  if (listener_.Bind(port)) {
    setup_ = true;
  }
  client_data_.reserve(Common::kMaxPlayers);
  return setup_;
}

bool NetAPI::Socket::Server::Update() {
  if (!setup_) {
    return false;
  }
  // Accept client
  if (connected_players_ < NetAPI::Common::kMaxPlayers) {
    ClientData* data = new ClientData();
    auto s = listener_.Accept(data->client.GetRaw());
    if (s) {
      std::cout << "DEBUG: tcp connection opened\n";
      sockaddr_in client_addr{};
      int size = sizeof(client_addr);
      auto ret = getpeername(data->client.GetRaw()->GetLowLevelSocket(),
                             (sockaddr*)&client_addr, &size);
      char buffer[14];
      inet_ntop(AF_INET, &client_addr.sin_addr, buffer, 14);
      auto addr = buffer;
      auto port = ntohs(client_addr.sin_port);
      auto ID = getHashedID(addr, port);
      if (client_data_.find(ID) != client_data_.end()) {
        client_data_[ID]->client.Disconnect();
        delete client_data_[ID];
      } else {
        std::cout << "DEBUG: adding new client\n";
        connected_players_++;
      }
      client_data_[ID] = data;
    }
  }

  std::vector<unsigned short> to_remove;
  // Receive Data
  for (auto& c : client_data_) {
    if (!c.second->client.IsConnected() ||
        c.second->client.GetRaw()->GetLastRecvLen() == 0) {
      std::cout << "DEBUG: removing client, lstrecvlen="
                << c.second->client.GetRaw()->GetLastRecvLen()
                << ", isConnected=" << c.second->client.IsConnected() << "\n";
      c.second->client.Disconnect();
      connected_players_--;
      delete c.second;
      to_remove.push_back(c.first);
      continue;
    }
    auto packet = c.second->client.Receive();
    if (!packet.IsEmpty()) {
      c.second->packets.push_back(packet);
    }
  }

  for (auto& id : to_remove) {
    client_data_.erase(id);
  }

  // Send Data
  NetAPI::Common::PacketHeader header;
  for (auto& d : data_to_send_) {
    d >> header;
    if (header.Receiver == EVERYONE) {
      for (auto& cli : client_data_) {
        cli.second->client.Send(d);
      }
    } else {
      auto result = client_data_.find(header.Receiver);
      if (result != client_data_.end()) {
        client_data_[header.Receiver]->client.Send(d);
      }
    }
  }
  data_to_send_.clear();
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
  h.Receiver = id;
  p << h;
  Send(p);
}

void NetAPI::Socket::Server::Send(NetAPI::Common::Packet& p) {
  data_to_send_.push_back(p);
}
