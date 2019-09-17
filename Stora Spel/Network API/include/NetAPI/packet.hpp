#ifndef PACKET_HPP_
#define PACKET_HPP_
#include <NetAPI/common.hpp>
#include <cstdint>
#include <cstring>
namespace NetAPI {
namespace Common {
struct EXPORT PacketHeader {
  unsigned short PacketAction[10];
  unsigned short Reciever = NetAPI::Socket::EVERYONE;
  unsigned PacketID = 0;
};

class EXPORT Packet {
 public:
  Packet();
  Packet(const Packet& other);
  Packet& operator=(const Packet&);

  Packet(const char* in_buffer, size_t in_size);
  ~Packet();
  size_t& GetPacketSize() { return size_of_data_; }
  bool IsEmpty() {
    return (size_of_data_ <= sizeof(p_) ||
            (strcmp(data_ + sizeof(p_), kNoDataAvailable) == 0));
  }
  template <typename T>
  Packet& operator<<(T data) {
    std::memcpy(data_ + size_of_data_, &data, sizeof(data));
    size_of_data_ += sizeof(data);
    return *this;
  }
  Packet& operator<<(PacketHeader header) {
    std::memcpy(data_, &header, sizeof(header));
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
  template <typename T>
  Packet& Add(T* data, size_t size) {
    std::memcpy(data_ + size_of_data_, data, sizeof(T) * size);
    size_of_data_ += sizeof(T) * size;
    return *this;
  }
  template <typename T>
  Packet& Remove(T* data, size_t size) {
    size_of_data_ -= sizeof(T) * size;
    std::memcpy(data, data_ + size_of_data_, sizeof(T) * size);
    return *this;
  }

 private:
  PacketHeader p_ = {};
  char* data_ = nullptr;
  size_t size_of_data_ = 0;
};
}  // namespace Common
}  // namespace NetAPI
#endif
