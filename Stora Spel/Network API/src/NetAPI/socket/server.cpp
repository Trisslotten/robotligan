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
      clientdata_[i].buffer = clients_.at(i)->Recive();
      if (this->SocketDisconnected(clientdata_[i])) {
        clients_[i]->Disconnect();
      }
      clientdata_[i].len = clients_.at(i)->GetLastRecvLen();
      clientdata_[i].ID = clients_.at(i)->GetID();
    }
  }
  for (auto& d : datatosend_) {
    if (d.ID == EVERYONE) {
      for (auto& cli : clients_) {
        cli->Send(d.buffer, d.len);
      }
    } else {
      clients_.at(d.ID)->Send(d.buffer, d.len);
    }
  }
  return true;
}

void NetAPI::Socket::Server::SendToAll(const char* data, size_t len) {
  datatosend_.push_back(Data(data, len, NetAPI::Socket::EVERYONE));
}
void NetAPI::Socket::Server::SendToAll(Data& d) { datatosend_.push_back(d); }

void NetAPI::Socket::Server::SendToAll(NetAPI::Common::Packet& p)
{
	datatosend_.push_back(Data());
	datatosend_.back().buffer = p.GetRaw();
	datatosend_.back().ID = p.GetHeader().PacketID;
	datatosend_.back().len = p.GetPacketSize();
}

void NetAPI::Socket::Server::Send(unsigned id, const char* data, size_t len) {
  datatosend_.push_back(Data(data, len, id));
}
void NetAPI::Socket::Server::Send(Data& d) { datatosend_.push_back(d); }

void NetAPI::Socket::Server::Send(NetAPI::Common::Packet& p)
{
	datatosend_.push_back(Data());
	datatosend_.back().buffer = p.GetRaw();
	datatosend_.back().ID = p.GetHeader().PacketID;
	datatosend_.back().len = p.GetPacketSize();
}
