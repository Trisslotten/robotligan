#pragma warning(push)
#pragma warning(disable : 4251)
#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <NetAPI/Packet.hpp>
#include <NetAPI/common.hpp>
#include <NetAPI/socket/Client.hpp>
#include <NetAPI/socket/tcplistener.hpp>
#include <NetAPI/socket/clientdata.hpp>
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
  short GetConnectedPlayers() { return connectedplayers_; }
  ClientData& operator[](short ID) { return clientdata_.at(ID); }

 private:
  std::unordered_map<unsigned short, ClientData> clientdata_;
  std::vector<Common::Packet> datatosend_;
  TcpListener listener_;
  bool setup_ = false;
  short connectedplayers_ = 0;
};

}  // namespace Socket
}  // namespace NetAPI
#endif  // SERVER_HPP_