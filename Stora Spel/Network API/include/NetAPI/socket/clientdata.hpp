#pragma once
#ifndef CLIENTDATA_HPP_
#define CLIENTDATA_HPP_
#include <NetAPI/common.hpp>
#include <NetAPI/packet.hpp>
#include <NetAPI/socket/Client.hpp>
#include <vector>
namespace NetAPI {
namespace Socket {
struct EXPORT ClientData {
  unsigned short ID = 0;
  Client client;
  std::vector<Common::Packet> packets;
};
}  // namespace Socket
}  // namespace NetAPI
#endif  // !CLIENTDATA_HPP_
