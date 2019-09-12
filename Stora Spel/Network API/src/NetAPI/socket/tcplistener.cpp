#include <NetAPI/Socket/tcplistener.h>
#include <string>
NetAPI::Socket::tcplistener::tcplistener()
{
	recbuffer = new char[buffersize];
}
NetAPI::Socket::tcplistener::~tcplistener()
{
	error = shutdown(listensocket, SD_RECEIVE);
	free(recbuffer);
}
NetAPI::Socket::tcpclient NetAPI::Socket::tcplistener::Accept()
{
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(listensocket, &readSet);
	timeval timeout;
	timeout.tv_sec = 0; 
	timeout.tv_usec = 0;
	SOCKET s = INVALID_SOCKET;
	tcpclient cli;
	if (select(listensocket, &readSet, NULL, NULL, &timeout) == 1)
	{
		s = accept(listensocket, NULL, NULL);
		if (s == INVALID_SOCKET)
		{
			error = WSAGetLastError();
		}
		else
		{
			cli = s;
			cli.setActive(true);
		}
	}
	return cli;
}
bool NetAPI::Socket::tcplistener::Bind(const unsigned short port)
{
	struct addrinfo* result = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	error = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
	if (error != 0)
	{
		error = WSAGetLastError();
		return false;
	}
	listensocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listensocket == INVALID_SOCKET) 
	{
		error = WSAGetLastError();
		freeaddrinfo(result);
		return false;
	}
	error = bind(listensocket, result->ai_addr, (int)result->ai_addrlen);
	if (error != 0)
	{
		error = WSAGetLastError();
		return false;
	}
	error = listen(listensocket, SOMAXCONN);
	if (error == SOCKET_ERROR)
	{
		error = WSAGetLastError();
		closesocket(listensocket);
		return false;
	}
	setup = true;
	return true;
}