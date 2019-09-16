#include <NetAPI/packet.hpp>

NetAPI::Common::Packet::Packet(size_t in_size)
{
	data_ = new char[in_size];
}

NetAPI::Common::Packet::~Packet()
{
	delete[] data_;
}
