#include <NetAPI/socket/server.hpp>
#include <string>
bool NetAPI::Socket::Server::Setup(unsigned short port) {
  if (listener_.Bind(port)) {
    setup_ = true;
  }
  clients_.resize(NetAPI::Common::kMaxPlayers);
  for (auto& cli : clients_) {
    cli = new TcpClient();
  }
  clientdata_.resize(NetAPI::Common::kMaxPlayers);

  return setup_;
}

bool NetAPI::Socket::Server::Update() {
  if (!setup_) {
    return false;
  }
  if (connectedplayers_ < NetAPI::Common::kMaxPlayers) {
    auto s = listener_.Accept(clients_.at(connectedplayers_));
    if (s) {
      std::string init = std::to_string(connectedplayers_);

      NetAPI::Common::Packet p;
      NetAPI::Common::PacketHeader h;
      h.Reciever = connectedplayers_;
      /*p << h;
       p << init;
   clients_.at(connectedplayers_)->Send(init.c_str(), init.length());
   */
      connectedplayers_++;
    }
  }
  unsigned removed = 0;
  for (short i = 0; i < connectedplayers_; i++) {
    if (clients_[i]->IsConnected()) {
      clientdata_[i] = clients_[i]->Recieve();
      if (!clients_[i]->IsConnected()) {
        clients_[i]->Disconnect();
      }
    }
  }
  NetAPI::Common::PacketHeader header;
  for (auto& d : datatosend_) {
    d >> header;
    if (header.Reciever == EVERYONE) {
      for (auto& cli : clients_) {
        cli->Send(d);
      }
    } else {
      clients_.at(header.Reciever)->Send(d);
    }
  }
  datatosend_.clear();
  return true;
}

void NetAPI::Socket::Server::SendToAll(const char* data, size_t len) {
  datatosend_.push_back(NetAPI::Common::Packet(data, len));
}

void NetAPI::Socket::Server::SendToAll(NetAPI::Common::Packet& p) {
  datatosend_.push_back(p);
}

void NetAPI::Socket::Server::Send(unsigned id, const char* data, size_t len) {
  NetAPI::Common::Packet p(data, len);
  NetAPI::Common::PacketHeader h;
  h.Reciever = id;
  p << h;
  Send(p);
}

void NetAPI::Socket::Server::Send(NetAPI::Common::Packet& p) {
  datatosend_.push_back(p);
}
