#include <NetAPI/socket/tcpclient.h>
#include <string>
NetAPI::Socket::tcpclient::tcpclient()
{
	buffersize = 512;
	
	sendbuffer = new char[buffersize];
	recbuffer = new char[buffersize];
}

void NetAPI::Socket::tcpclient::SetBufferSize(unsigned size)
{
	buffersize = size;
	free(sendbuffer);
	free(recbuffer);
	sendbuffer = new char[buffersize];
	recbuffer = new char[buffersize];
}
void NetAPI::Socket::tcpclient::flushbuffers()
{
	ZeroMemory(sendbuffer, sizeof(char) * buffersize);
	ZeroMemory(recbuffer, sizeof(char) * buffersize);
}
bool NetAPI::Socket::tcpclient::Connect(const char* addr, unsigned short port)
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
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		error = connect(sendsocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (error == SOCKET_ERROR) {
			closesocket(sendsocket);
			sendsocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);
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
bool NetAPI::Socket::tcpclient::Send(void* data, unsigned length)
{
	error = send(sendsocket, (char*)data, length, 0);
	if (error)
	{
		error = WSAGetLastError();
		return false;
	}
	return true;
}
void* NetAPI::Socket::tcpclient::Recive()
{
	//Implement blocking? meeh
	this->flushbuffers();
	error = recv(sendsocket, (char*)recbuffer, buffersize, 0);
	if (error > 0)
	{
		return recbuffer;
	}
	if (error == 0)
	{
		connected = false;
		return nullptr;
	}
	else
	{
		return nullptr;
	}
}
void NetAPI::Socket::tcpclient::operator=(const SOCKET& other)
{
	connected = true;
	sendsocket = other;
}
NetAPI::Socket::tcpclient::~tcpclient()
{
	error = shutdown(sendsocket, SD_SEND);
	free(sendbuffer);
	free(recbuffer);
}