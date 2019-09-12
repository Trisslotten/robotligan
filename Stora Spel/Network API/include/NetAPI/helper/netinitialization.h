#pragma once
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <NetAPI/common.h>
#ifndef SOCKET_INTERNALS_H
#define SOCKET_INTERNALS_H
namespace NetAPI
{
	namespace initialization {
		 struct SocketInternals {
			char major_version = 0, minor_version = 0;
			WORD version = WORD();
			int error = 0;
			WSADATA socket_data = WSADATA();
			bool initialized = false;
		};
		class GlobalSocketInternals final
		{
		 public:
			 static GlobalSocketInternals& GetInstance();
			 bool initializeWsock(char major = 2, char minor = 2);
			 SocketInternals internals = {};
			 
		 private:
			 GlobalSocketInternals() = default;
			 ~GlobalSocketInternals() = default;

			 GlobalSocketInternals(const GlobalSocketInternals&) = delete;
			 GlobalSocketInternals& operator=(const GlobalSocketInternals&) = delete;
			 GlobalSocketInternals(GlobalSocketInternals&&) = delete;
			 GlobalSocketInternals& operator=(GlobalSocketInternals&&) = delete;
		 };
		EXPORT bool  WinsockInitialized();
	}
}
#endif // !SOCKET_INTERNALS_H