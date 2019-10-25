#include <NetAPI/socket/server.hpp>
#include <iostream>
#include <string>

void NetAPI::Socket::Server::SendPing() {
  NetAPI::Common::Packet to_send;
  to_send.GetHeader()->packet_id = NetAPI::Socket::EVERYONE;
  int random_number = -1 + std::rand() % ((-1000 - 1) + 1);
  to_send << random_number << PacketBlockType::PING;
  this->SendToAll(to_send);
}
void NetAPI::Socket::Server::HandleClientPacket()
{
	//std::pair<const long, NetAPI::Socket::ClientData *> s
	for (auto& c : client_data_) {
		HandleSingleClientPacket(c);
	}
}
void NetAPI::Socket::Server::HandleSingleClientPacket(std::pair<const long, NetAPI::Socket::ClientData*> &c)
{
	if (c.second && !c.second->client.IsConnected() && c.second->is_active) {
		std::cout << "DEBUG: removing client, lstrecvlen="
			<< c.second->client.GetRaw()->GetLastRecvLen()
			<< ", isConnected=" << c.second->client.IsConnected() << "\n";
		c.second->client.Disconnect();
		c.second->is_active = false;
		connected_players_--;
		return
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
void NetAPI::Socket::Server::ListenForClients()
{
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
		// auto ID = getHashedID(addr, port);
		std::string address = addr + (":" + std::to_string(port));

		std::cout << "DEBUG: tcp connection accepted: "
			<< addr + (":" + std::to_string(port)) << "\n";

		auto find_res = ids_.find(address);
		// if already found
		if (find_res != ids_.end()) {
			accepted = true;
			auto client_data = client_data_[find_res->second];
			client_data->client.Disconnect();
			delete client_data;
			connection_client_->ID = find_res->second;
			client_data_[find_res->second] = connection_client_;

			std::cout << "DEBUG: Found existing client, overwriting\n";
		}
		else if (connected_players_ < NetAPI::Common::kMaxPlayers) {
			std::cout << "DEBUG: adding new client\n";

			connection_client_->ID = current_client_guid_;
			ids_[address] = current_client_guid_;
			client_data_[current_client_guid_] = connection_client_;

			connected_players_++;
			current_client_guid_++;
			accepted = true;
		}
		else {
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
		}
		else {
			int can_join = -2;
			NetAPI::Common::Packet p;
			p << can_join << PacketBlockType::SERVER_CAN_JOIN;
		}
		connection_client_ = nullptr;
	}
}
void NetAPI::Socket::Server::SendStoredData()
{
	NetAPI::Common::PacketHeader header;
	for (auto& d : data_to_send_) {
		d >> header;
		if (header.receiver == EVERYONE) {
			for (auto& cli : client_data_) {
				cli.second->client.Send(d);
			}
		}
		else {
			auto result = client_data_.find(header.receiver);
			if (result != client_data_.end()) {
				client_data_[header.receiver]->client.Send(d);
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
  ListenForClients();
  // Receive Data
  HandleClientPacket();
  // Send Data
  SendPing();

  SendStoredData();
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
  auto it = client_data_.find(ID);
  if (it != client_data_.end()) {
    this->client_data_.erase(ID);
    connected_players_--;
    return true;
  } else {
    return false;
  }
}
