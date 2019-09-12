#include <NetAPI/netinitialization.h>

NetAPI::initialization::GlobalSocketInternals& NetAPI::initialization::GlobalSocketInternals::GetInstance()
{
	static GlobalSocketInternals instance;
	return instance;
}

bool NetAPI::initialization::GlobalSocketInternals::initializeWsock(char major, char minor)
{
	if (internals.initialized)
	{
		return true;
	}
	internals.version = MAKEWORD(major, minor);
	internals.major_version = major;
	internals.minor_version = minor;
	internals.error = WSAStartup(internals.version, &internals.socket_data);
	if (internals.error != 0) {
		internals.initialized = false;
		return false;
	}
	if ((HIBYTE(internals.version) != major ||
		LOBYTE(internals.minor_version) != minor)) {
		internals.initialized = false;
		WSACleanup();
		return false;
	}
	internals.initialized = true;
	return true;
}

bool NetAPI::initialization::GlobalSocketInternals::WinsockInitialized()
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED) {
		return false;
	}
	closesocket(s);
	return true;
}
