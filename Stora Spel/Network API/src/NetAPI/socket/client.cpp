#include <NetAPI/socket/client.h>

NetAPI::Socket::client::client()
{

}

bool NetAPI::Socket::client::connect(const char* addr, unsigned short port)
{
	bool Connected = cli.Connect(addr, port);
	if (!Connected)
	{
		error = cli.querryError();
	}
	return Connected;
}

const char * NetAPI::Socket::client::recv()
{
	if (!cli.isconnected())
	{
		error = cli.querryError();
		return NetAPI::Common::socket_not_connected;
	}
	const char* recbuff = cli.Recive();
	return recbuff;
}

bool NetAPI::Socket::client::send(const char * data, unsigned size)
{
	if (!cli.isconnected() || size > cli.GetBufferSize())
	{
		return false;
	}
	return cli.Send(data, size);
}

void NetAPI::Socket::client::operator=( client& other)
{
	cli = other.cli.GetLowLevelSocket();
	error = other.cli.querryError();
}
