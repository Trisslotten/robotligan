#pragma once
#ifndef CLIENTDATA_HPP_
#define CLIENTDATA_HPP_
#include <NetAPI/common.hpp>
#include <NetAPI/packet.hpp>
#include <NetAPI/socket/client.hpp>
#include <vector>
#include <string>
namespace NetAPI {
namespace Socket {
struct EXPORT ClientData {
  unsigned short ID = 0;
  Client client;
  std::vector<Common::Packet> packets;
  std::string address;
};
}  // namespace Socket
}  // namespace NetAPI
#endif  // !CLIENTDATA_HPP_
