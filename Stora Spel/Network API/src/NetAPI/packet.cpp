#include <NetAPI/packet.hpp>
#include <iostream>

NetAPI::Common::Packet::Packet() {
  data_ = new char[kNumPacketBytes];
  PacketHeader p{};
  memcpy(data_, &p, sizeof(PacketHeader));
  size_of_data_ = sizeof(PacketHeader);
}

NetAPI::Common::Packet::Packet(const Packet& other) {
  data_ = new char[kNumPacketBytes];
  memcpy(data_, other.data_, kNumPacketBytes);
  size_of_data_ = other.size_of_data_;
}

NetAPI::Common::Packet& NetAPI::Common::Packet::operator=(const Packet& other) {
  if (&other == this) return *this;

  data_ = new char[kNumPacketBytes];
  memcpy(data_, other.data_, kNumPacketBytes);
  size_of_data_ = other.size_of_data_;

  return *this;
}

NetAPI::Common::Packet::Packet(const char* in_buffer, size_t in_size) {
  data_ = new char[kNumPacketBytes];
  if (in_size > 0 && in_size <= kNumPacketBytes) {
    memcpy(data_, in_buffer, kNumPacketBytes);
    size_of_data_ = in_size;
  } else {
    if (in_size > kNumPacketBytes)
      std::cout << "ERROR: bad size in Packet buffer creation (" << in_size << " bytes)\n",
    memset(data_, 0, kNumPacketBytes);
    size_of_data_ = 0;
  }
}

NetAPI::Common::Packet::~Packet() { delete[] data_; }
