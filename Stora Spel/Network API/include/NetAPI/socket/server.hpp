#pragma warning(push)
#pragma warning(disable : 4251)
#ifndef SERVER_HPP_
#define SERVER_HPP_
#define SERVER_HPP_
#include <unordered_map>
#include <vector>
#include <chrono>
#include <NetAPI/Packet.hpp>
#include <NetAPI/common.hpp>
#include <NetAPI/socket/client.hpp>
#include <NetAPI/socket/clientdata.hpp>
#include <NetAPI/socket/tcplistener.hpp>
#include <shared/shared.hpp>
namespace NetAPI {
namespace Socket {
class EXPORT Server {
 public:
  bool Setup(unsigned short port, unsigned short maxplayers = 6);
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
  void ResetPlayers() { game_players_ = connected_players_; }
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
  short connected_players_ = 0;
  short game_players_ = 0;
  long current_client_guid_ = 0;
  unsigned short max_players_ = 6;
};

}  // namespace Socket
}  // namespace NetAPI
#endif  // SERVER_HPP_