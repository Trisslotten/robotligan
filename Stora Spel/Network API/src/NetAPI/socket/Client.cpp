#include <NetAPI/socket/client.hpp>
namespace NetAPI {
namespace Socket {
bool Client::Connect(const char* addr, short port) {
  return client_->Connect(addr, port);
}

bool Client::Send(NetAPI::Common::Packet& p) { return client_->Send(p); }

NetAPI::Common::Packet Client::Receive(const short timeout) {
  return client_->Receive(timeout);
}
}  // namespace Socket
}  // namespace NetAPI
