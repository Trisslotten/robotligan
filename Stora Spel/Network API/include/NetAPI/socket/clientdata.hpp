#pragma once
#ifndef CLIENTDATA_HPP_
#define CLIENTDATA_HPP_
#include <NetAPI/common.hpp>
#include <vector>
#include <NetAPI/packet.hpp>
#include <NetAPI/socket/Client.hpp>
namespace NetAPI
{
	namespace Socket
	{
		struct EXPORT ClientData {
			unsigned short ID = 0;
			Client client;
			std::vector<Common::Packet> packets;
		};
	}
}
#endif // !CLIENTDATA_HPP_
