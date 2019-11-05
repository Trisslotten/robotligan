#include <NetAPI/socket/server.hpp>
#include <iostream>
#include <string>
using namespace std::chrono_literals;
void NetAPI::Socket::Server::ClearPackets(NetAPI::Socket::ClientData* data)
{
	data->packets.clear();
}
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
}
void NetAPI::Socket::Server::HandleSingleClientPacket(unsigned short ID)
{
	std::chrono::high_resolution_clock timer;
	std::chrono::duration<float> durr;
	std::chrono::time_point<std::chrono::high_resolution_clock> init;
	std::chrono::time_point<std::chrono::high_resolution_clock> test;
	auto c = this->client_data_[ID];
	while (threads[ID].second)
	{
		auto islocked = IsLocked();
		if (!islocked)
		{
			if (!c->client.IsConnected() && c->is_active) {
				std::cout << "DEBUG: removing client, lstrecvlen="
					<< c->client.GetRaw()->GetLastRecvLen()
					<< ", isConnected=" << c->client.IsConnected() << "\n";
				c->client.Disconnect();
				c->is_active = false;
				connected_players_--;
			}
			else if ((c->client.IsConnected()))
			{
				/*
					Clear packet när man pushar paket? hmmm

				*/
				int num_packets = 0;
				init = timer.now();
				for (auto packet : c->client.Receive()) {
					num_packets++;
					if (!packet.IsEmpty()) {
						c->packets.push_back(packet);
					}
					test = timer.now();
					durr = test - init;
					auto cast = std::chrono::duration_cast<std::chrono::milliseconds>(durr);
					if (cast.count() > 250)
					{
						c->last_failed_ = true;
						break;
					}
					else
					{
						c->last_failed_ = false;
					}
				}
			}
			else
			{
				std::this_thread::sleep_for(100ns);
			}
		}
		else
		{
			std::this_thread::sleep_for(100ns);
		}
	}
	c = nullptr;
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
			auto &s = threads[find_res->second].second = false;
			threads[find_res->second] = std::pair<std::thread, bool>(std::thread([this, &c = find_res->second]{ HandleSingleClientPacket(c); }), true);
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
			threads[ids_[address]] = std::pair<std::thread, bool>(std::thread([this, &c = ids_[address]]{ HandleSingleClientPacket(c); }), true);
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
	SendPing();
	NetAPI::Common::PacketHeader header;
	for (auto& d : data_to_send_) {
		d >> header;
		if (header.receiver == EVERYONE) {
			for (auto& cli : client_data_) {
				if (!cli.second->last_failed_)
				{
					cli.second->client.Send(d);
				}
			}
		}
		else {
			auto result = client_data_.find(header.receiver);
			if (result != client_data_.end()) {
				auto c = client_data_[header.receiver];
				if (!c->last_failed_)
				{
					c->client.Send(d);
				}
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
  new_frame = true;
  if (!setup_) {
    return false;
  }
  ListenForClients();
  // Receive Data
  // Send Data
  SendStoredData();
  new_frame = false;
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
    client_to_remove_.push_back(ID);
    return true;
  } else {
    return false;
  }
}
