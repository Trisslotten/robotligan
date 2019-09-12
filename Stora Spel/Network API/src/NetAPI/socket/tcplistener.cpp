#include <NetAPI/Socket/tcplistener.h>
#include <string>

NetAPI::Socket::tcplistener::tcplistener()
{
	recbuffer = new char[buffersize];
	FD_ZERO(&readSet);
	FD_SET(listensocket, &readSet);

	timeout.tv_sec = 0;
	timeout.tv_usec = 20;
}
NetAPI::Socket::tcplistener::~tcplistener()
{
	error = shutdown(listensocket, SD_RECEIVE);
	delete recbuffer;
}
NetAPI::Socket::tcpclient NetAPI::Socket::tcplistener::Accept()
{
	FD_ZERO(&readSet);
	FD_SET(listensocket, &readSet);
	SOCKET s = INVALID_SOCKET;
	tcpclient cli;
	if (select(listensocket, &readSet, NULL, NULL, &timeout) > 0)
	{
		while (s == INVALID_SOCKET)
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
	}
	return cli;
}
const char* NetAPI::Socket::tcplistener::Recv(SOCKET &cli)
{
	FD_ZERO(&readSet);
	FD_SET(listensocket, &readSet);
	int bytes = 1;
	if (select(cli, &readSet, NULL, NULL, &timeout) > 0)
	{
		bytes = recv(cli, recbuffer, buffersize, 0);
		if (bytes > 0)
		{
			return recbuffer;
		}
		if (bytes == 0)
		{
			return NetAPI::Common::socket_not_connected;
		}
		else
		{
			error = WSAGetLastError();
			return nullptr;
		}
	}
	else
	{
		return NetAPI::Common::no_data_available;
	}
}
void NetAPI::Socket::tcplistener::disconnect()
{
	if (setup && listensocket != INVALID_SOCKET)
	{
		setup = false;
		closesocket(listensocket);
	}
}
bool NetAPI::Socket::tcplistener::Bind(const unsigned short port)
{
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	listensocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listensocket == INVALID_SOCKET)
	{
		error = WSAGetLastError();
		return false;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
	if (iResult != 0) 
	{
		error = WSAGetLastError();
		return false;
	}
	listensocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listensocket == INVALID_SOCKET)
	{
		error = WSAGetLastError();
		return false;
	}
	iResult = bind(listensocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listensocket);
		return false;
	}
	freeaddrinfo(result);
	iResult = listen(listensocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) 
	{
		error = WSAGetLastError();
		closesocket(listensocket);
		return false;
	}
	char on = 1;
	error = setsockopt(listensocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	error = setsockopt(listensocket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
	error = 0;
	setup = true;
	return setup;
}