#pragma once
#define WIN32_LEAN_AND_MEAN
#ifndef TCPLISTENER_H
#define TCPLISTENER_H
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <NetAPI/socket/tcpclient.h>
namespace NetAPI
{
	namespace Socket
	{
		class tcplistener
		{
		public:
			tcplistener();
			~tcplistener();
			bool Bind(const unsigned short port);
			tcpclient Accept();
			int querryError() { return error; }
		private:
			bool setup = false;
			int error = 0;
			unsigned buffersize = 512;
			void* recbuffer = nullptr;
			SOCKET listensocket = INVALID_SOCKET;
		};
	}
}
#endif // !TCPLISTENER_H
