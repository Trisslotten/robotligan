#include <NetAPI/socket/tcpclient.hpp>
#include <string>
NetAPI::Socket::TcpClient::TcpClient() {
  rec_buffer_ = new char[buffer_size_];
  timeout_.tv_sec = 0;
  timeout_.tv_usec = 50;
}

NetAPI::Socket::TcpClient::TcpClient(const TcpClient& other) {
  rec_buffer_ = new char[other.buffer_size_];
  memcpy(rec_buffer_, other.rec_buffer_, other.buffer_size_);
  buffer_size_ = other.buffer_size_;
  connected_ = other.connected_;
  ID_ = other.ID_;
  send_socket_ = other.send_socket_;
  blocking_ = other.blocking_;
  error_ = other.error_;
  last_buff_len_ = other.last_buff_len_;
  read_set_ = other.read_set_;
  timeout_ = other.timeout_;
}

void NetAPI::Socket::TcpClient::SetBufferSize(unsigned size) {
  buffer_size_ = size;
  delete[] rec_buffer_;
  rec_buffer_ = new char[buffer_size_];
}
void NetAPI::Socket::TcpClient::FlushBuffers() {
  ZeroMemory(rec_buffer_, sizeof(char) * buffer_size_);
}
bool NetAPI::Socket::TcpClient::Connect(const char* addr, unsigned short port) {
  struct addrinfo *result = NULL, *ptr = NULL, hints = {};
  error_ = getaddrinfo(addr, std::to_string(port).c_str(), &hints, &result);
  if (error_) {
    error_ = WSAGetLastError();
    return false;
  }
  for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
    send_socket_ = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (send_socket_ == INVALID_SOCKET) {
      error_ = WSAGetLastError();
      return false;
    }
    error_ = connect(send_socket_, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (error_ == SOCKET_ERROR) {
      closesocket(send_socket_);
      send_socket_ = INVALID_SOCKET;
      continue;
    }
    break;
  }
  if (send_socket_ == INVALID_SOCKET) {
    error_ = WSAGetLastError();
    return false;
  }
  char on = 1;
  error_ = setsockopt(send_socket_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  error_ = setsockopt(send_socket_, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
  if (error_) {
    error_ = WSAGetLastError();
    return false;
  }
  freeaddrinfo(result);
  connected_ = true;
  return true;
}
bool NetAPI::Socket::TcpClient::Send(const char* data, size_t length) {
  error_ = send(send_socket_, data, (int)length, 0);
  if (error_ == SOCKET_ERROR) {
    error_ = WSAGetLastError();
    return false;
  }
  return true;
}
bool NetAPI::Socket::TcpClient::Send(NetAPI::Common::Packet& p) {
  error_ = send(send_socket_, p.GetRaw(), (int)p.GetPacketSize(), 0);
  if (error_ == SOCKET_ERROR) {
    error_ = WSAGetLastError();
    return false;
  }
  return true;
}
const char* NetAPI::Socket::TcpClient::Recive(unsigned short timeout) {
  // Implement blocking? meeh
  int bytes = 1;

  FD_ZERO(&read_set_);
  FD_SET(send_socket_, &read_set_);
  timeout_.tv_usec = timeout;
  if (select(send_socket_, &read_set_, NULL, NULL, &timeout_) == 1) {
    last_buff_len_ = recv(send_socket_, rec_buffer_, buffer_size_, 0);
    if (last_buff_len_ > 0) {
      return rec_buffer_;
    }
    if (last_buff_len_ == 0) {
      connected_ = false;
      return NetAPI::Common::kSocketNotConnected;
    } else {
      error_ = WSAGetLastError();
      return nullptr;
    }
  } else {
    return NetAPI::Common::kNoDataAvailable;
  }
}
NetAPI::Common::Packet NetAPI::Socket::TcpClient::Receive(
    unsigned short timeout) {
  int bytes = 1;
  FD_ZERO(&read_set_);
  FD_SET(send_socket_, &read_set_);
  timeout_.tv_usec = timeout;
  if (select(send_socket_, &read_set_, NULL, NULL, &timeout_) == 1) {
    last_buff_len_ = recv(send_socket_, rec_buffer_, buffer_size_, 0);
    if (last_buff_len_ > 0) {
      return NetAPI::Common::Packet(rec_buffer_, last_buff_len_);
    }
    if (last_buff_len_ == 0 || (WSAGetLastError() == WSAECONNRESET)) {
      connected_ = false;
      this->Disconnect();
      return NetAPI::Common::Packet(nullptr, 0);
    } else {
      error_ = WSAGetLastError();
      return NetAPI::Common::Packet(nullptr, 0);
    }
  } else {
    return NetAPI::Common::Packet(rec_buffer_, last_buff_len_);
  }
}
void NetAPI::Socket::TcpClient::Disconnect() {
  connected_ = false;
  closesocket(send_socket_);
  send_socket_ = INVALID_SOCKET;
}
void NetAPI::Socket::TcpClient::operator=(const SOCKET& other) {
  connected_ = true;
  send_socket_ = other;
}
NetAPI::Socket::TcpClient& NetAPI::Socket::TcpClient::operator=(
    const TcpClient& other) {
  rec_buffer_ = new char[other.buffer_size_];
  memcpy(rec_buffer_, other.rec_buffer_, other.buffer_size_);
  buffer_size_ = other.buffer_size_;
  connected_ = other.connected_;
  ID_ = other.ID_;
  send_socket_ = other.send_socket_;
  blocking_ = other.blocking_;
  error_ = other.error_;
  last_buff_len_ = other.last_buff_len_;
  read_set_ = other.read_set_;
  timeout_ = other.timeout_;
  return *this;
}
NetAPI::Socket::TcpClient::~TcpClient() {
  delete[] this->rec_buffer_;
  error_ = shutdown(send_socket_, SD_SEND);
}