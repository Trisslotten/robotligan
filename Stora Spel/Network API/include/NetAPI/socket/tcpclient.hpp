#ifndef TCPCLIENT_HPP_
#define TCPCLIENT_HPP_
#define WIN32_LEAN_MEAN
#include <NetAPI/common.hpp>
#include <NetAPI/helper/netinitialization.hpp>
#include <NetAPI/packet.hpp>
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <thread>

namespace NetAPI {
namespace Socket {
class EXPORT TcpClient {
 public:
  TcpClient();
  TcpClient(const TcpClient& other);
  TcpClient& operator=(const TcpClient& other);
  ~TcpClient();
  void SetBufferSize(unsigned size);
  unsigned GetBufferSize() { return buffer_size_; }
  void FlushBuffers();
  bool Connect(const char* addr, unsigned short port);
  bool Send(const char* data, size_t length);
  bool Send(NetAPI::Common::Packet& p);
  const char* Recive();
  NetAPI::Common::Packet Recieve();
  int QuerryError() { return error_; }
  void SetBlocking(bool block = true) { blocking_ = block; }
  void SetActive(bool c = true) { connected_ = c; };
  SOCKET& GetLowLevelSocket() { return send_socket_; }
  bool IsConnected() { return connected_; }
  void Disconnect();
  void operator=(const SOCKET& other);
  size_t GetLastRecvLen() { return last_buff_len_; }
  const char* GetBuffer() { return rec_buffer_; }
  byte GetID() { return ID_; }

 private:
  byte ID_ = 0;
  int last_buff_len_ = 0;
  timeval timeout_ = {};
  fd_set read_set_ = {};
  bool blocking_ = false;
  int error_ = 0;
  bool connected_ = false;
  unsigned buffer_size_ = Common::kNumPacketBytes;
  char* rec_buffer_ = nullptr;
  SOCKET send_socket_ = INVALID_SOCKET;
};
}  // namespace Socket
}  // namespace NetAPI
#endif  // !TCPCLIENT_H