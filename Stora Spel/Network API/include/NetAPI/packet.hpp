#ifndef PACKET_HPP_
#define PACKET_HPP_
#include <NetAPI/common.hpp>
#include <cstdint>
#include <cstring>
#include <iostream>
namespace NetAPI {
namespace Common {
struct EXPORT PacketHeader {
  short packet_size = 0;
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

  long GetPacketSize() { return GetHeader()->packet_size; }

  bool IsEmpty() {
    return (GetHeader()->packet_size <= sizeof(PacketHeader) ||
            (strcmp(data_ + sizeof(PacketHeader), kNoDataAvailable) == 0));
  }

  template <typename T>
  Packet& operator<<(T data) {
    if (GetHeader()->packet_size + sizeof(T) > kNumPacketBytes) {
      std::cout << "ERROR: packet full: currsize=" << GetHeader()->packet_size
                << ", max=" << kNumPacketBytes << "\n\ttried adding a '"
                << typeid(T).name() << "' (" << sizeof(T) << " bytes) \n";
    } else {
      std::memcpy(data_ + GetHeader()->packet_size, &data, sizeof(T));
      GetHeader()->packet_size += sizeof(T);

      // std::cout << "<< GetHeader()->packet_size=" << GetHeader()->packet_size << "\n";
    }

    return *this;
  }

  Packet& operator<<(PacketHeader header) {
    std::memcpy(data_, &header, sizeof(header));

    // std::cout << "<<header GetHeader()->packet_size=" << GetHeader()->packet_size << "\n";

    return *this;
  }

  Packet& operator<<(const Packet& other) {
    int size_without_head = other.GetHeader()->packet_size - (int)sizeof(PacketHeader);
    if (size_without_head <= 0) {
      return *this;
    }
    if (GetHeader()->packet_size + size_without_head > kNumPacketBytes) {
      std::cout << "ERROR: packet full: currsize=" << GetHeader()->packet_size
                << ", max=" << kNumPacketBytes
                << "\n\ttried adding another packet (" << size_without_head
                << " bytes) \n";
    } else {
      std::memcpy(data_ + GetHeader()->packet_size, other.data_ + sizeof(PacketHeader),
                  size_without_head);
      GetHeader()->packet_size += size_without_head;
      // std::cout << "<< GetHeader()->packet_size=" << GetHeader()->packet_size << "\n";
    }

    return *this;
  }

  template <typename T>
  Packet& operator>>(T& data) {
    if (GetHeader()->packet_size - (long)sizeof(T) < (long)sizeof(PacketHeader)) {
      std::cout << "ERROR: packet empty: currsize=" << GetHeader()->packet_size
                << ", min=" << sizeof(PacketHeader) << "\n\ttried getting a '"
                << typeid(T).name() << "' (" << sizeof(T) << " bytes) \n";
      GetHeader()->packet_size = sizeof(PacketHeader);
    } else {
      GetHeader()->packet_size -= sizeof(T);
      std::memcpy(&data, data_ + GetHeader()->packet_size, sizeof(data));
    }
    // std::cout << ">> GetHeader()->packet_size=" << GetHeader()->packet_size << "\n";

    return *this;
  }

  Packet& operator>>(PacketHeader& header) {
    std::memcpy(&header, data_, sizeof(header));

    // std::cout << ">>header GetHeader()->packet_size=" << GetHeader()->packet_size << "\n";

    return *this;
  }

  const char* GetRaw() { return data_; }

  PacketHeader* GetHeader() const { return (PacketHeader*)data_; }

  template <typename T>
  Packet& Add(T* data, long size) {
    if (GetHeader()->packet_size + (long)sizeof(T) * size > kNumPacketBytes) {
      std::cout << "ERROR: packet full, currsize=" << GetHeader()->packet_size
                << ", max=" << kNumPacketBytes << "\n\ttried adding: " << size
                << " '" << typeid(T).name() << "'s (" << sizeof(T) * size
                << " bytes)\n";
    } else {
      std::memcpy(data_ + GetHeader()->packet_size, data, sizeof(T) * size);
      GetHeader()->packet_size += sizeof(T) * size;

      // std::cout << "ADD: GetHeader()->packet_size=" << GetHeader()->packet_size << "\n";
    }
    return *this;
  }

  template <typename T>
  Packet& Remove(T* data, long size) {
    if (GetHeader()->packet_size - long(sizeof(T)) * size < (long)sizeof(PacketHeader)) {
      std::cout << "ERROR: packet empty, currsize=" << GetHeader()->packet_size
                << ", min=" << sizeof(PacketHeader)
                << "\n\ttried getting: " << size << " '" << typeid(T).name()
                << "'s (" << sizeof(T) * size << " bytes)\n";
      GetHeader()->packet_size = sizeof(PacketHeader);
    } else {
      GetHeader()->packet_size -= (long)sizeof(T) * size;
      std::memcpy(data, data_ + GetHeader()->packet_size, sizeof(T) * size);
    }

    return *this;
  }

 private:
  char* data_ = nullptr;
};

}  // namespace Common
}  // namespace NetAPI

#endif
