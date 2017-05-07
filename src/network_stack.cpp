

#include "network_stack.h"

int NetworkStack::Send(SOCKET sock, char* buffer, int bytes, sockaddr_in* dest, int start_layer)
{
	int sent = bytes;
	int i = start_layer;
	if (start_layer < 0)
	{
		i = layers.size() - 1;
	}
	for (;i >= 0; --i)
	{
		sent = layers[i].Send(buffer, sent, dest);
	}
	return Send(sock, buffer, sent, dest);
}
int NetworkStack::Receive(SOCKET sock, char* buffer, int max_bytes)
{
	int recv = Receive(sock, buffer, max_bytes);
	for (unsigned i = 0; i < layers.size(); ++i)
	{
		recv = layers[i].Receive(buffer, recv);
	}
	return recv;
}
void NetworkStack::Update()
{
	
}