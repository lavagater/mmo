#include <string.h>

#include "network_stack.h"

NetworkLayer::~NetworkLayer(){}

NetworkStack::~NetworkStack()
{
	for (unsigned i = 0; i < layers.size(); ++i)
	{
		delete layers[i];
	}
}

bool operator==(const sockaddr_in &lhs, const sockaddr_in &rhs)
{
	//just check the ip and port
  return lhs.sin_addr.s_addr == rhs.sin_addr.s_addr && lhs.sin_port == rhs.sin_port;
}

size_t SimpleHash(unsigned key)
{
	for (unsigned i = 0; i < 17; ++i)
  {
    key = (key * 5915587277 + 1) % 9576890767;
  }
  return key;
}

size_t SockAddrHash::operator()(const sockaddr_in &rhs) const
{
	return SimpleHash(rhs.sin_addr.s_addr) ^ SimpleHash(rhs.sin_port);
}

int NetworkStack::Send(SOCKET sock, const char* buffer, int bytes, sockaddr_in* dest, int start_layer)
{
	//create a new buffer that can be added too
	char new_buf[MAXSOCKETSIZE];
	memcpy(new_buf, buffer, bytes);

	int sent = bytes;
	int i = start_layer;
	if (start_layer < 0)
	{
		i = layers.size() - 1;
	}
	for (;i >= 0; --i)
	{
		sent = layers[i]->Send(new_buf, sent, dest);
	}
	//calling the socket library send function
	return ::Send(sock, new_buf, sent, dest);
}
int NetworkStack::Receive(SOCKET sock, char* buffer, int max_bytes, sockaddr_in* location)
{
	//calling the socket library Receive function
	int recv = ::Receive(sock, buffer, max_bytes);
	for (unsigned i = 0; i < layers.size(); ++i)
	{
		recv = layers[i]->Receive(buffer, recv, location);
	}
	return recv;
}
void NetworkStack::Update()
{
	double dt = timer.GetTime();
	for (unsigned i = 0; i < layers.size(); ++i)
	{
		layers[i]->Update(dt);
	}
}