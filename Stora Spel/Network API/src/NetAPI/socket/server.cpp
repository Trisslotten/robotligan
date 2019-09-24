#include <NetAPI/socket/server.hpp>
#include <string>
unsigned short getHashedID(char* addr, unsigned short port)
{
	unsigned short retval = 0;
	for (unsigned short c = 0; addr[c] != '\0'; c++)
	{
			retval = retval + (unsigned short)(addr[c]);
	}
	return retval % sizeof(unsigned short);
}
bool NetAPI::Socket::Server::Setup(unsigned short port) {
  if (listener_.Bind(port)) {
    setup_ = true;
  }
  clientdata_.reserve(Common::kMaxPlayers);
  return setup_;
}

bool NetAPI::Socket::Server::Update() {
  if (!setup_) {
    return false;
  }
  //Accept client
  if (connectedplayers_ < NetAPI::Common::kMaxPlayers) {
	  ClientData data;
	  auto s = listener_.Accept(data.client.GetRaw());
	  if (s)
	  {
		  sockaddr_in client_addr{};
		  int size = sizeof(client_addr);
		  auto ret = getpeername(data.client.GetRaw()->GetLowLevelSocket(), (sockaddr*)& client_addr, &size);
		  auto addr = inet_ntoa(client_addr.sin_addr);
		  auto port = ntohs(client_addr.sin_port);
		  auto ID = getHashedID(addr, port);
		  clientdata_[ID] = data;
		  connectedplayers_++;
	  }
  }
  //Recieve Data
  for (auto& c : clientdata_)
  {
	  if (!c.second.client.IsConnected())
	  {
		  c.second.client.Disconnect();
	  }
	  c.second.packets.push_back(c.second.client.Receive());
	  
  }
  //Send Data
  NetAPI::Common::PacketHeader header;
  for (auto& d : datatosend_) {
    d >> header;
    if (header.Receiver == EVERYONE) {
      for (auto& cli : clientdata_) {
        cli.second.client.Send(d);
      }
    } else {
		clientdata_[header.Receiver].client.Send(d);
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
  h.Receiver = id;
  p << h;
  Send(p);
}

void NetAPI::Socket::Server::Send(NetAPI::Common::Packet& p) {
  datatosend_.push_back(p);
}
