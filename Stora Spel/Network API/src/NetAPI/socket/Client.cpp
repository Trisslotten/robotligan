#include <NetAPI/socket/client.hpp>
#include <iostream>
#include <vector>
#include <shared/shared.hpp>
namespace NetAPI {
namespace Socket {
bool Client::Connect(const char* addr, short port) {
  Common::Packet p;
  client_->Connect(addr, port);
  auto hwid = GetHWID();
  p.Add(hwid.data(), hwid.size());
  p << hwid.size();
  p << PacketBlockType::HWID;
  auto status = client_->Send(p);
  // std::cout << "Success : " << status << std::endl;
  return status;
}

bool Client::Send(NetAPI::Common::Packet& p) { return client_->Send(p); }

std::vector<NetAPI::Common::Packet> Client::Receive(const short timeout) {
  // std::cout << "last_len_: " << client_->GetLastRecvLen();
  return client_->Receive(timeout);
}
std::string Client::GetHWID() {
  HW_PROFILE_INFO hwProfileInfo;
  if (GetCurrentHwProfile(&hwProfileInfo)) {
    std::wstring str(hwProfileInfo.szHwProfileGuid);
    auto hwid = std::string(str.begin(), str.end());
    return hwid;
  } else {
    return std::string("");
  }
}
}  // namespace Socket
}  // namespace NetAPI
