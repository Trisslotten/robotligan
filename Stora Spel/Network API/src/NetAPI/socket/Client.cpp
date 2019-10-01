#include <NetAPI/socket/client.hpp>
#include <iostream>
#include <vector>
namespace NetAPI {
namespace Socket {
bool Client::Connect(const char* addr, short port) {
  return client_->Connect(addr, port);
}

bool Client::Send(NetAPI::Common::Packet& p) { return client_->Send(p); }

std::vector<NetAPI::Common::Packet> Client::Receive(const short timeout) {
  //std::cout << "last_len_: " << client_->GetLastRecvLen();
  return client_->Receive(timeout);
}
}  // namespace Socket
}  // namespace NetAPI
