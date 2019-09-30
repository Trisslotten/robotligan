#include <NetAPI/socket/tcpclient.hpp>
#include <iostream>
#include <string>
#include <vector>
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
bool NetAPI::Socket::TcpClient::Send(NetAPI::Common::Packet& p) {
  auto h = p.GetHeader();
  h->packet_size = p.GetPacketSize();

  error_ = send(send_socket_, p.GetRaw(), (int)p.GetPacketSize(), 0);
  if (error_ == SOCKET_ERROR) {
    error_ = WSAGetLastError();
    if (error_ == WSAECONNRESET || error_ == WSAECONNABORTED ||
        error_ == WSAENETRESET || error_ == WSAENOTCONN) {
      this->Disconnect();
    }
    return false;
  }
  return true;
}
std::vector<NetAPI::Common::Packet> NetAPI::Socket::TcpClient::Receive(
    unsigned short timeout) {
  int bytes = 1;
  FD_ZERO(&read_set_);
  FD_SET(send_socket_, &read_set_);
  timeout_.tv_usec = timeout;

  std::vector<NetAPI::Common::Packet> result;

  if (select(send_socket_, &read_set_, NULL, NULL, &timeout_) == 1) {
    last_buff_len_ = recv(send_socket_, rec_buffer_, buffer_size_, 0);
    // std::cout << "last_buff_len_=" << last_buff_len_ << "\n";
    if (last_buff_len_ > 0) {
      NetAPI::Common::Packet big_packet(rec_buffer_, last_buff_len_);
      int buff_left = last_buff_len_;
      int offset = 0;
      while (buff_left > 0) {
        auto packet_size = big_packet.GetHeader()->packet_size;
        NetAPI::Common::Packet packet(rec_buffer_ + offset, packet_size);

        result.push_back(packet);

        if (result.size() > 20) {
		  // something wrong
          break;
		}

        offset += packet_size;
        buff_left -= packet_size;
        big_packet = NetAPI::Common::Packet(rec_buffer_ + offset, buff_left);
      }
      return result;
    }
    if (last_buff_len_ == 0 || (WSAGetLastError() == WSAECONNRESET)) {
      connected_ = false;
      this->Disconnect();
      return result;
    } else {
      error_ = WSAGetLastError();
      std::cout << "ERROR: Network error code: " << error_ << "\n";
      return result;
    }
  } else {
    return result;
  }
  return result;
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
  closesocket(send_socket_);
  send_socket_ = INVALID_SOCKET;
}