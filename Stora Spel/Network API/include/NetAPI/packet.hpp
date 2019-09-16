#ifndef PACKET_H
#define PACKET_H
#include <cstdint>
#include <cstring>
#include <NetAPI/common.hpp>
struct EXPORT shit
{
	int lol = 2;
	float derp = 11.f;
};
struct EXPORT shitdelux
{
	int lol = 2;
	float derp = 11.f;
};
namespace NetAPI
{
	namespace Common
	{
		class EXPORT Packet
		{
		public:
			Packet(size_t in_size = 512);
			~Packet();
			size_t& GetPacketSize()
			{
				return size_of_data_;
			}
			bool IsEmpty()
			{
				return (size_of_data_ == 0);
			}
			template<typename T>
			Packet& operator<<(T& data)
			{
				std::memcpy(data_ + size_of_data_, &data, sizeof(data));
				size_of_data_ += sizeof(data);
				return *this;
			}
			template<typename T>
			Packet& operator>>(T& data)
			{
				size_of_data_ -= sizeof(data);
				std::memcpy(&data, data_ + size_of_data_, sizeof(data));
				return *this;
			}
		private:
			char* data_ = {};
			size_t size_of_data_ = 0;
		};
	}
}
#endif
