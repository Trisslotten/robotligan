#pragma warning(push)
#pragma warning(disable : 4251)
#ifndef SERVER_HPP_
#define SERVER_HPP_
#include <unordered_map>
#include <vector>
#include <NetAPI/Packet.hpp>
#include <NetAPI/common.hpp>
#include <NetAPI/socket/client.hpp>
#include <NetAPI/socket/clientdata.hpp>
#include <NetAPI/socket/tcplistener.hpp>
#include <shared/shared.hpp>
#include <chrono>
namespace NetAPI {
namespace Socket {
class EXPORT Server {
 public:
  bool Setup(unsigned short port);
  bool Update();
  void SendToAll(const char* data, size_t len);
  void SendToAll(NetAPI::Common::Packet& p);
  void Send(unsigned id, const char* data, size_t len);
  void Send(NetAPI::Common::Packet& p);
  short GetConnectedPlayers() { return connected_players_; }
  bool KickPlayer(long ID);
  /// ClientData* operator[](short ID) { return client_data_.at(ID); }
  std::unordered_map<long, ClientData*>& GetClients() { return client_data_; }
  std::vector<ClientData*> GetNewlyConnected() { return newly_connected_; }
  void ClearPackets(NetAPI::Socket::ClientData* data);
 private:
  void SendPing();
  void HandleClientPacket();
  void Receive();
  void ListenForClients();
  void SendStoredData();
  std::unordered_map<std::string, long> ids_;
  std::unordered_map<long, ClientData*> client_data_;
  std::vector<long> client_to_remove_;
  std::vector<ClientData*> newly_connected_;
  std::vector<Common::Packet> data_to_send_;
  ClientData* connection_client_ = nullptr;
  TcpListener listener_;
  bool setup_ = false;
  bool locked = false;
  bool new_frame = true;
  short connected_players_ = 0;
  long current_client_guid_ = 0;
};

}  // namespace Socket
}  // namespace NetAPI
#endif  // SERVER_HPP_