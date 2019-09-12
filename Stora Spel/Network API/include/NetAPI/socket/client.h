#pragma once
#ifndef CLIENT_H
#define CLIENT_H
#define WIN32_LEAN_MEAN
#include <NetAPI/common.h>
#include <NetAPI/socket/tcpclient.h>
#include <NetAPI/helper/errors.h>
#include <NetAPI/helper/netinitialization.h>
namespace NetAPI 
{
	namespace Socket
	{
		class EXPORT client
		{
		public:
			client();
			bool connect(const char* addr, unsigned short port);
			const char* recv();
			bool send(const char* data, unsigned size);
			void operator=(client& other);
			int GetError() { return error; }
			std::string getErrorString() { return NetAPI::Helper::printWSAError(error); }
			bool IsConnected() { return cli.isconnected(); }
			void disconnect() { cli.disconnect();}
		private:
			int error = 0;
			tcpclient cli;
		};
	}
}
#endif // CLIENT_H

