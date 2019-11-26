#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <chrono>
#include <NetAPI/common.hpp>
#include <NetAPI/packet.hpp>
#include <NetAPI/socket/tcpclient.hpp>
namespace NetAPI {
namespace Socket {
class EXPORT Client {
 public:
  Client() { client_ = new NetAPI::Socket::TcpClient(); }
  ~Client() { delete client_; }
  bool Connect(const char* addr, short port);
  bool Send(NetAPI::Common::Packet& p);
  std::vector<NetAPI::Common::Packet> Receive(const short timeout = 50);
  unsigned short& GetID() { return ID_; }

  void Disconnect() { client_->Disconnect(); }
  bool IsConnected() { return client_->IsConnected(); }
  TcpClient* GetRaw() { return client_; }
  bool JustDiconnected() { return just_disconnected_; }
  void SetDisconnected(bool disconnected) { just_disconnected_ = disconnected; }
  uint64_t TimeSinceLastUpdate();

 private:
  std::chrono::high_resolution_clock::time_point last_client_update_time_ =
      std::chrono::high_resolution_clock::now();
  bool just_disconnected_ = false;
  unsigned short ID_ = 0;
  NetAPI::Socket::TcpClient* client_ = nullptr;
  std::string GetHWID();
};
}  // namespace Socket
}  // namespace NetAPI
#endif  // !CLIENT_HPP_