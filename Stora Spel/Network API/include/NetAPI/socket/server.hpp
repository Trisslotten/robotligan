#pragma warning(push)
#pragma warning(disable : 4251)
#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <NetAPI/common.hpp>
#include <NetAPI/socket/tcplistener.hpp>
#include <vector>
namespace NetAPI {
namespace Socket {
class EXPORT Server {
 public:
  bool Setup(unsigned short port);
  bool Update();
  void SendToAll(const char* data, size_t len);
  void SendToAll(Data& d);
  void SendToAll(NetAPI::Common::Packet& p);
  void Send(unsigned id, const char* data, size_t len);
  void Send(Data& d);
  void Send(NetAPI::Common::Packet& p);
  bool SocketDisconnected(Data& d) {
    return (strcmp(d.buffer, NetAPI::Common::kSocketNotConnected) == 0);
  }
  bool HasData(Data& d) {
    if (strcmp(d.buffer, Common::kNoDataAvailable) == 0 ||
        strcmp(d.buffer, Common::kFailedToRecieve) == 0) {
      return false;
    }
    return true;
  }
  short GetConnectedPlayers() { return connectedplayers_; }
  NetAPI::Common::Packet& operator[](short ID) { return clientdata_.at(ID); }
  void Cleanup() {
    for (auto ptr : clients_) {
      delete ptr;
    }
  }

 private:
  TcpListener listener_;
  bool setup_ = false;
  short connectedplayers_ = 0;
  std::vector<TcpClient*> clients_ = std::vector<TcpClient*>();
  std::vector<Data> datatosend_;
  std::vector<NetAPI::Common::Packet> clientdata_;
};

}  // namespace Socket
}  // namespace NetAPI
#endif // SERVER_HPP_