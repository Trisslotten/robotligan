#include <NetAPI/socket/Client.hpp>
namespace NetAPI {
namespace Socket {
int Client::Connect(const char* addr, short port) {
  bool success = client_->Connect(addr, port);
  return client_->QuerryError();
}

int Client::Send(NetAPI::Common::Packet& p) {
  bool success = client_->Send(p);
  return client_->QuerryError();
}

NetAPI::Common::Packet Client::Receive(const short timeout) {
  return client_->Recieve(timeout);
}
}  // namespace Socket
}  // namespace NetAPI
