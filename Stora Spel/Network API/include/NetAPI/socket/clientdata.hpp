#pragma once
#ifndef CLIENTDATA_HPP_
#define CLIENTDATA_HPP_
#include <vector>
#include <string>
#include <chrono>
#include <NetAPI/common.hpp>
#include <NetAPI/packet.hpp>
#include <NetAPI/socket/client.hpp>
namespace NetAPI {
namespace Socket {
const unsigned KAveragePingCount = 64;
struct EXPORT ClientData {
  ClientData() { ping.resize(KAveragePingCount); }
  unsigned short ID = 0;
  Client client;
  std::vector<Common::Packet> packets;
  std::string address;
  bool is_active = false;
  std::chrono::time_point<std::chrono::steady_clock> last_time =
      std::chrono::steady_clock::now();
  std::vector<unsigned> ping;
  uint64_t ping_sum = 0;
  unsigned ping_id = 0;
};
}  // namespace Socket
}  // namespace NetAPI
#endif  // !CLIENTDATA_HPP_
