#include <NetAPI/packet.hpp>
#include <iostream>

NetAPI::Common::Packet::Packet() {
  data_ = new char[kNumPacketBytes];
  PacketHeader p{};
  memcpy(data_, &p, sizeof(PacketHeader));
  GetHeader()->packet_size = sizeof(PacketHeader);
}

NetAPI::Common::Packet::Packet(const Packet& other) {
  //delete[] data_;
  data_ = new char[kNumPacketBytes];
  memcpy(data_, other.data_, kNumPacketBytes);
  GetHeader()->packet_size = other.GetHeader()->packet_size;
}

NetAPI::Common::Packet& NetAPI::Common::Packet::operator=(const Packet& other) {
  if (&other == this) return *this;

  /*
  if (data_ != nullptr)
    delete[] data_;

  */
  if (data_ == nullptr) {
    data_ = new char[kNumPacketBytes];
  }
  memcpy(data_, other.data_, kNumPacketBytes);
  GetHeader()->packet_size = other.GetHeader()->packet_size;

  return *this;
}

NetAPI::Common::Packet::Packet(const char* in_buffer, long in_size) {
  data_ = new char[kNumPacketBytes];
  if (in_size > 0 && in_size <= kNumPacketBytes) {
    memcpy(data_, in_buffer, kNumPacketBytes);
    GetHeader()->packet_size = in_size;
  } else {
    if (in_size > kNumPacketBytes || in_size < 0)
      std::cout << "ERROR: bad size in Packet buffer creation (" << in_size << " bytes)\n",
    memset(data_, 0, kNumPacketBytes);
    GetHeader()->packet_size = 0;
  }
}

NetAPI::Common::Packet::~Packet() {
	if (data_)
	{
		delete[] data_;
		data_ = nullptr;
	}
}
