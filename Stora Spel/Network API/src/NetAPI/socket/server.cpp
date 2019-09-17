#include <NetAPI/socket/server.hpp>
#include <string>
bool NetAPI::Socket::Server::Setup(unsigned short port) {
  if (listener_.Bind(port)) {
    setup_ = true;
  }
  clients_.push_back(new TcpClient());
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
      clients_.push_back(new TcpClient());
      clients_.at(connectedplayers_)->Send(init.c_str(), init.length());
      clientdata_.resize(6);
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
    if (header.PacketID == EVERYONE) {
      for (auto& cli : clients_) {
        cli->Send(d);
      }
    } else {
      clients_.at(header.PacketID)->Send(d);
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
	NetAPI::Common::Packet p;
	NetAPI::Common::PacketHeader h;
	h.PacketID = id;
	p << h;
	Send(p);
}

void NetAPI::Socket::Server::Send(NetAPI::Common::Packet& p) {
	datatosend_.push_back(p);
}
