#ifndef PACKET_H
#define PACKET_H
#include <NetAPI/common.hpp>
#include <cstdint>
#include <cstring>
namespace NetAPI {
namespace Common {
struct EXPORT PacketHeader {
  byte PacketAction[10];
  byte Reciever = NetAPI::Socket::EVERYONE;
  unsigned PacketID = 0;
};
class EXPORT Packet {
 public:
  Packet(size_t in_size = 512);
  ~Packet();
  size_t& GetPacketSize() { return size_of_data_; }
  bool IsEmpty() { return (size_of_data_ <= sizeof(p_)); }
  template <typename T>
  Packet& operator<<(T& data) {
    std::memcpy(data_ + size_of_data_, &data, sizeof(data));
    size_of_data_ += sizeof(data);
    return *this;
  }
  Packet& operator<<(PacketHeader& header) {
    std::memcpy(&header, data_, sizeof(header));
    return *this;
  }
  template <typename T>
  Packet& operator>>(T& data) {
    size_of_data_ -= sizeof(data);
    std::memcpy(&data, data_ + size_of_data_, sizeof(data));
    return *this;
  }
  Packet& operator>>(PacketHeader& header) {
    std::memcpy(&header, data_, sizeof(header));
    return *this;
  }
  const char* GetRaw() { return data_; }
  PacketHeader& GetHeader() { return p_; }

 private:
  PacketHeader p_ = {};
  char* data_ = {};
  size_t size_of_data_ = 0;
};
}  // namespace Common
}  // namespace NetAPI
#endif
