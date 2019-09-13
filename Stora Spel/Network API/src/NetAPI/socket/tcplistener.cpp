#include <NetAPI/Socket/tcplistener.hpp>
#include <string>

NetAPI::Socket::TcpListener::TcpListener() {
  rec_buffer_ = new char[buffer_size_];
  FD_ZERO(&read_set_);
  FD_SET(listen_socket_, &read_set_);
}
NetAPI::Socket::TcpListener::~TcpListener() {
  error_ = shutdown(listen_socket_, SD_RECEIVE);
  delete rec_buffer_;
}
bool NetAPI::Socket::TcpListener::Accept(NetAPI::Socket::TcpClient* cli) {
  FD_ZERO(&read_set_);
  FD_SET(listen_socket_, &read_set_);
  timeout_.tv_sec = 0;
  timeout_.tv_usec = 0;
  SOCKET s = INVALID_SOCKET;
  if (select(listen_socket_, &read_set_, NULL, NULL, &timeout_) > 0) {
    s = accept(listen_socket_, NULL, NULL);
    if (s == INVALID_SOCKET) {
      error_ = WSAGetLastError();
    } else {
      *cli = s;
      cli->SetActive(true);
      return true;
    }
  } else {
    return false;
  }
}
const char* NetAPI::Socket::TcpListener::Recv(SOCKET& cli) {
  FD_ZERO(&read_set_);
  FD_SET(listen_socket_, &read_set_);
  timeout_.tv_sec = 0;
  timeout_.tv_usec = 0;
  int bytes = 1;
  if (select(cli, &read_set_, NULL, NULL, &timeout_) > 0) {
    bytes = recv(cli, rec_buffer_, buffer_size_, 0);
    if (bytes > 0) {
      return rec_buffer_;
    }
    if (bytes == 0) {
      return NetAPI::Common::kSocketNotConnected;
    } else {
      error_ = WSAGetLastError();
      return nullptr;
    }
  } else {
    error_ = WSAGetLastError();
    return NetAPI::Common::kNoDataAvailable;
  }
}
void NetAPI::Socket::TcpListener::Disconnect() {
  if (setup_ && listen_socket_ != INVALID_SOCKET) {
    setup_ = false;
    closesocket(listen_socket_);
  }
}
bool NetAPI::Socket::TcpListener::Bind(const unsigned short port) {
  struct addrinfo* result = NULL;
  struct addrinfo hints;
  listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listen_socket_ == INVALID_SOCKET) {
    error_ = WSAGetLastError();
    return false;
  }
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  int res = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
  if (res != 0) {
    error_ = WSAGetLastError();
    return false;
  }
  listen_socket_ =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (listen_socket_ == INVALID_SOCKET) {
    error_ = WSAGetLastError();
    return false;
  }
  res = bind(listen_socket_, result->ai_addr, (int)result->ai_addrlen);
  if (res == SOCKET_ERROR) {
    printf("bind failed with error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(listen_socket_);
    return false;
  }
  freeaddrinfo(result);
  res = listen(listen_socket_, SOMAXCONN);
  if (res == SOCKET_ERROR) {
    error_ = WSAGetLastError();
    closesocket(listen_socket_);
    return false;
  }
  char on = 1;
  // error = setsockopt(listensocket, SOL_SOCKET, SO_REUSEADDR, &on,
  // sizeof(on)); error = setsockopt(listensocket, IPPROTO_TCP, TCP_NODELAY, &on,
  // sizeof(on));
  error_ = 0;
  setup_ = true;
  return setup_;
}