#include <NetAPI/socket/client.hpp>
#include <iostream>
#include <vector>
#include <shared/shared.hpp>
typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;
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
  auto packets = client_->Receive(timeout);
  if (!packets.empty()) {
    last_client_update_time_ = std::chrono::high_resolution_clock::now();
  }
  return packets;
}
uint64_t Client::TimeSinceLastUpdate() {
  return std::chrono::milliseconds(std::chrono::duration_cast<ms>(
                                       Time::now() - last_client_update_time_))
      .count();
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
