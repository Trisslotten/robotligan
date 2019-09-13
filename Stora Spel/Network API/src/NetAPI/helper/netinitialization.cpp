#include <NetAPI/helper/netinitialization.hpp>

NetAPI::Initialization::GlobalSocketInternals&
NetAPI::Initialization::GlobalSocketInternals::GetInstance() {
  static GlobalSocketInternals instance;
  return instance;
}

bool NetAPI::Initialization::GlobalSocketInternals::InitializeWsock(
    char major, char minor) {
  if (internals_.initialized) {
    return true;
  }
  internals_.version = MAKEWORD(major, minor);
  internals_.major_version = major;
  internals_.minor_version = minor;
  internals_.error = WSAStartup(internals_.version, &internals_.socket_data);
  if (internals_.error != 0) {
    internals_.initialized = false;
    return false;
  }
  if ((HIBYTE(internals_.version) != major ||
       LOBYTE(internals_.minor_version) != minor)) {
    internals_.initialized = false;
    WSACleanup();
    return false;
  }
  internals_.initialized = true;
  return true;
}

bool NetAPI::Initialization::WinsockInitialized() {
  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED) {
    return false;
  }
  closesocket(s);
  return true;
}
