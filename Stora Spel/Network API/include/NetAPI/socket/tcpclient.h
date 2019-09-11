#pragma once
#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#define WIN32_LEAN_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <thread>
namespace NetAPI {
	namespace Socket
	{
		class tcpclient
		{
		public:
			tcpclient();
			~tcpclient();
			void SetBufferSize(unsigned size);
			void flushbuffers();
			bool Connect(const char* addr, unsigned short port);
			bool Send(void *data, unsigned length);
			void *Recive();
			int querryError() { return error; }
			void setBlocking(bool block = true) { blocking = block; }
		private:
			bool blocking = false;
			int error = 0;
			bool connected = false;
			unsigned buffersize = 0;
			void *sendbuffer = nullptr;
			void *recbuffer = nullptr;
			SOCKET sendsocket = INVALID_SOCKET;
		};
	}
}
#endif // !TCPCLIENT_H