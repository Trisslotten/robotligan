#include <NetAPI/packet.hpp>

NetAPI::Common::Packet::Packet() {
  data_ = new char[kNumPacketBytes];
  memcpy(data_, &p_, sizeof(p_));
  size_of_data_ = sizeof(p_);
}

NetAPI::Common::Packet::Packet(const Packet& other) {
  data_ = new char[kNumPacketBytes];
  memcpy(data_, other.data_, kNumPacketBytes);
  size_of_data_ = other.size_of_data_;
  p_ = other.p_;
}

NetAPI::Common::Packet& NetAPI::Common::Packet::operator=(const Packet& other) {
  if (&other == this) return *this;

  data_ = new char[kNumPacketBytes];
  memcpy(data_, other.data_, kNumPacketBytes);
  size_of_data_ = other.size_of_data_;
  p_ = other.p_;

  return *this;
}

NetAPI::Common::Packet::Packet(const char* in_buffer, size_t in_size) {
  data_ = new char[kNumPacketBytes];
  memcpy(data_, in_buffer, kNumPacketBytes);
  size_of_data_ = in_size;
}

NetAPI::Common::Packet::~Packet() { delete[] data_; }
