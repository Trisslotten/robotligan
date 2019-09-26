#pragma warning(push)
#pragma warning(disable : 4251)
#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <NetAPI/Packet.hpp>
#include <NetAPI/common.hpp>
#include <NetAPI/socket/client.hpp>
#include <NetAPI/socket/clientdata.hpp>
#include <NetAPI/socket/tcplistener.hpp>
#include <unordered_map>
#include <vector>
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
  ClientData* operator[](short ID) { return client_data_.at(ID); }
  std::unordered_map<unsigned short, ClientData*>& GetClients() {
    return client_data_;
  }

 private:
  std::unordered_map<unsigned short, ClientData*> client_data_;
  std::vector<Common::Packet> data_to_send_;
  TcpListener listener_;
  bool setup_ = false;
  short connected_players_ = 0;
};

}  // namespace Socket
}  // namespace NetAPI
#endif  // SERVER_HPP_