#ifndef PACKET_HPP_
#define PACKET_HPP_
#include <NetAPI/common.hpp>
#include <cstdint>
#include <cstring>
#include <iostream>
namespace NetAPI {
namespace Common {
struct EXPORT PacketHeader {
  unsigned short packet_size = 0;
  unsigned short receiver = NetAPI::Socket::EVERYONE;
  unsigned packet_id = 0;
};

class EXPORT Packet {
 public:
  Packet();
  Packet(const Packet& other);
  Packet& operator=(const Packet&);

  Packet(const char* in_buffer, long in_size);
  ~Packet();

  long GetPacketSize() { return size_of_data_; }

  bool IsEmpty() {
    return (size_of_data_ <= sizeof(PacketHeader) ||
            (strcmp(data_ + sizeof(PacketHeader), kNoDataAvailable) == 0));
  }

  template <typename T>
  Packet& operator<<(T data) {
    if (size_of_data_ + sizeof(T) > kNumPacketBytes) {
      std::cout << "ERROR: packet full: currsize=" << size_of_data_
                << ", max=" << kNumPacketBytes
                << "\n\ttried adding a '" << typeid(T).name() << "' ("
                << sizeof(T) << " bytes) \n";
    } else {
      std::memcpy(data_ + size_of_data_, &data, sizeof(T));
      size_of_data_ += sizeof(T);

      // std::cout << "<< size_of_data_=" << size_of_data_ << "\n";
    }

    return *this;
  }

  Packet& operator<<(PacketHeader header) {
    std::memcpy(data_, &header, sizeof(header));

    // std::cout << "<<header size_of_data_=" << size_of_data_ << "\n";

    return *this;
  }

  template <typename T>
  Packet& operator>>(T& data) {
    if (size_of_data_ - (long)sizeof(T) < sizeof(PacketHeader)) {
      std::cout << "ERROR: packet empty: currsize=" << size_of_data_
                << ", min=" << sizeof(PacketHeader)
                << "\n\ttried getting a '" << typeid(T).name() << "' ("
                << sizeof(T) << " bytes) \n";
    }
    size_of_data_ -= sizeof(T);
    std::memcpy(&data, data_ + size_of_data_, sizeof(data));

    // std::cout << ">> size_of_data_=" << size_of_data_ << "\n";

    return *this;
  }

  Packet& operator>>(PacketHeader& header) {
    std::memcpy(&header, data_, sizeof(header));

    // std::cout << ">>header size_of_data_=" << size_of_data_ << "\n";

    return *this;
  }

  const char* GetRaw() { return data_; }

  PacketHeader* GetHeader() { return (PacketHeader*)data_; }

  template <typename T>
  Packet& Add(T* data, long size) {
    if (size_of_data_ + sizeof(T) * size > kNumPacketBytes) {
      std::cout << "ERROR: packet full, currsize=" << size_of_data_
                << ", max=" << kNumPacketBytes << "\n\ttried adding: " << size
                << " '" << typeid(T).name() << "'s (" << sizeof(T) * size
                << " bytes)\n";
    } else {
      std::memcpy(data_ + size_of_data_, data, sizeof(T) * size);
      size_of_data_ += sizeof(T) * size;

      // std::cout << "ADD: size_of_data_=" << size_of_data_ << "\n";
    }
    return *this;
  }

  template <typename T>
  Packet& Remove(T* data, long size) {
    if (size_of_data_ - long(sizeof(T)) * size < (long)sizeof(PacketHeader)) {
      std::cout << "ERROR: packet empty, currsize=" << size_of_data_
                << ", min=" << sizeof(PacketHeader) << "\n\ttried getting: " << size
                << " '" << typeid(T).name() << "'s (" << sizeof(T) * size
                << " bytes)\n";
    } else {
      size_of_data_ -= (long)sizeof(T) * size;
      std::memcpy(data, data_ + size_of_data_, sizeof(T) * size);
    }

    return *this;
  }

 private:
  char* data_ = nullptr;
  long size_of_data_ = 0;
};
}  // namespace Common
}  // namespace NetAPI

#endif
