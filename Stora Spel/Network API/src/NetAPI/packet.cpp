#include <NetAPI/packet.hpp>

NetAPI::Common::Packet::Packet(size_t in_size) {
  data_ = new char[in_size + sizeof(p_)];
  memcpy(data_, &p_, sizeof(p_));
  size_of_data_ = sizeof(p_);

NetAPI::Common::Packet::Packet(const char* in_buffer, size_t in_size) {
  data_ = new char[in_size + sizeof(p_)];
  memcpy(data_, &in_buffer, in_size);
  size_of_data_ = in_size;
}

NetAPI::Common::Packet::~Packet() { delete[] data_; }
