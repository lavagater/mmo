#include <string.h>

#include "network_stack.h"

int NetworkStack::Send(SOCKET sock, const char* buffer, int bytes, sockaddr_in* dest, int start_layer)
{
	//create a new buffer that can be added too
	char new_buf[2048];
	memcpy(new_buf, buffer, bytes);

	int sent = bytes;
	int i = start_layer;
	if (start_layer < 0)
	{
		i = layers.size() - 1;
	}
	for (;i >= 0; --i)
	{
		sent = layers[i].Send(new_buf, sent, dest);
	}
	return Send(sock, new_buf, sent, dest);
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
	double dt = timer.GetTime();
	for (unsigned i = 0; i < layers.size(); ++i)
	{
		layers[i].Update(dt);
	}
}