#include <NetAPI/socket/tcpclient.h>
#include <string>
NetAPI::Socket::tcpclient::tcpclient()
{
	buffersize = 512;
	
	recbuffer = new char[buffersize];
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
}

void NetAPI::Socket::tcpclient::SetBufferSize(unsigned size)
{
	buffersize = size;
	delete recbuffer;
	recbuffer = new char[buffersize];
}
void NetAPI::Socket::tcpclient::flushbuffers()
{
	ZeroMemory(recbuffer, sizeof(char) * buffersize);
}
bool NetAPI::Socket::tcpclient::Connect(const char * addr, unsigned short port)
{
	struct addrinfo* result = NULL, * ptr = NULL, hints = {};
	error = getaddrinfo(addr, std::to_string(port).c_str(), &hints, &result);
	if (error)
	{
		error = WSAGetLastError();
		return false;
	}
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		sendsocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (sendsocket == INVALID_SOCKET) {
			error = WSAGetLastError();
			return false;
		}
		error = connect(sendsocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (error == SOCKET_ERROR) {
			closesocket(sendsocket);
			sendsocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	if (sendsocket == INVALID_SOCKET) 
	{
		error = WSAGetLastError();
		return false;
	}
	char on = 1;
	error = setsockopt(sendsocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	error = setsockopt(sendsocket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
	if (error)
	{
		error = WSAGetLastError();
		return false;
	}
	freeaddrinfo(result);
	connected = true;
	return true;
}
bool NetAPI::Socket::tcpclient::Send(const char * data, unsigned length)
{
	int bytes = send(sendsocket, data, length, 0);
	if (bytes == SOCKET_ERROR)
	{
		error = WSAGetLastError();
		return false;
	}
	return true;
}
const char *NetAPI::Socket::tcpclient::Recive()
{
	//Implement blocking? meeh
	int bytes = 1;
	FD_ZERO(&readSet);
	FD_SET(sendsocket, &readSet);
	if (select(sendsocket, &readSet, NULL, NULL, &timeout) == 1)
	{
		bytes = recv(sendsocket, recbuffer, buffersize, 0);
		if (bytes > 0)
		{
			return recbuffer;
		}
		if (bytes == 0)
		{
			connected = false;
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
void NetAPI::Socket::tcpclient::disconnect()
{
	if (connected && sendsocket != INVALID_SOCKET)
	{
		connected = false;
		closesocket(sendsocket);
	}
}
void NetAPI::Socket::tcpclient::operator=(const SOCKET& other)
{
	connected = true;
	sendsocket = other;
}
//Potentiell memoryleak?
NetAPI::Socket::tcpclient::~tcpclient()
{
	error = shutdown(sendsocket, SD_SEND);
}