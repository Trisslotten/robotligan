#include <NetAPI/socket/tcpclient.hpp>
#include <string>
NetAPI::Socket::TcpClient::TcpClient() {
  buffer_size_ = 512;

  rec_buffer_ = new char[buffer_size_];
  timeout_.tv_sec = 0;
  timeout_.tv_usec = 500;
}

void NetAPI::Socket::TcpClient::SetBufferSize(unsigned size) {
  buffer_size_ = size;
  delete rec_buffer_;
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
	error_ = send(send_socket_,p. GetRaw(), (int)p.GetPacketSize(), 0);
	if (error_ == SOCKET_ERROR) {
		error_ = WSAGetLastError();
		return false;
	}
	return true;
}
const char* NetAPI::Socket::TcpClient::Recive() {
  // Implement blocking? meeh
  int bytes = 1;

  FD_ZERO(&read_set_);
  FD_SET(send_socket_, &read_set_);
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
void NetAPI::Socket::TcpClient::Disconnect() {
  if (connected_ && send_socket_ != INVALID_SOCKET) {
    connected_ = false;
    closesocket(send_socket_);
  }
}
void NetAPI::Socket::TcpClient::operator=(const SOCKET& other) {
  connected_ = true;
  send_socket_ = other;
}
// Potentiell memoryleak?
NetAPI::Socket::TcpClient::~TcpClient() {
  error_ = shutdown(send_socket_, SD_SEND);
}