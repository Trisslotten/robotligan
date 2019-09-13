#define WIN32_LEAN_AND_MEAN
#ifndef TCPLISTENER_H
#define TCPLISTENER_H
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <NetAPI/common.hpp>
#include <NetAPI/socket/tcpclient.hpp>
namespace NetAPI {
namespace Socket {
class EXPORT TcpListener {
 public:
  TcpListener();
  ~TcpListener();
  bool Bind(const unsigned short port);
  bool Accept(NetAPI::Socket::TcpClient* cli);
  int QuerryError() { return error_; }
  const char* Recv(SOCKET& cli);  // Deprecated
  void Disconnect();
  size_t GetRecvBytes() { return last_bytes_; }

 private:
  size_t last_bytes_;
  fd_set read_set_ = {};
  timeval timeout_ = {};
  bool setup_ = false;
  int error_ = 0;
  unsigned buffer_size_ = 512;
  char* rec_buffer_;
  SOCKET listen_socket_ = INVALID_SOCKET;
};
}  // namespace Socket
}  // namespace NetAPI
#endif  // !TCPLISTENER_H
