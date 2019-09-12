#pragma once
#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#define WIN32_LEAN_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <thread>
#include <NetAPI/common.h>
#include <NetAPI/helper/netinitialization.h>

namespace NetAPI {
	namespace Socket
	{
		class EXPORT tcpclient
		{
		public:
			tcpclient();
			~tcpclient();
			void SetBufferSize(unsigned size);
			unsigned GetBufferSize() { return buffersize; }
			void flushbuffers();
			bool Connect(const char * addr, unsigned short port);
			bool Send(const char *data, unsigned length);
			const char *Recive();
			int querryError() { return error; }
			void setBlocking(bool block = true) { blocking = block; }
			void setActive(bool c = true) { connected = c; };
			SOCKET& GetLowLevelSocket() { return sendsocket; }
			bool isconnected() { return connected; }
			void disconnect();
			void operator=(const SOCKET& other);

		private:
			timeval timeout = {};
			fd_set readSet = {};
			bool blocking = false;
			int error = 0;
			bool connected = false;
			unsigned buffersize = 512;
			char *recbuffer = nullptr;
			SOCKET sendsocket = INVALID_SOCKET;
		};
	}
}
#endif // !TCPCLIENT_H