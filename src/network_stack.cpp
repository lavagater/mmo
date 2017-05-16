#include <string.h>

#include "network_stack.h"

NetworkLayer::~NetworkLayer(){}

NetworkStack::NetworkStack(SOCKET socket)
{
	sock= socket;
}

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

int NetworkStack::Send(const char* buffer, int bytes, sockaddr_in* dest, BitArray<HEADERSIZE> &flags, int start_layer)
{
	//create a new buffer that can be added too
	char new_buf[MAXPACKETSIZE];
	//leave room in buffer for header
	memcpy(new_buf+HEADERSIZE/8, buffer, bytes);

	int sent = bytes;
	int i = start_layer;
	if (start_layer < 0)
	{
		i = layers.size() - 1;
	}
	for (;i >= 0; --i)
	{
		//leave room in buffer for header
		sent = layers[i]->Send(new_buf+HEADERSIZE/8, sent, dest, flags);
	}
	//put flags into buffer
	for (unsigned i = 0; i < HEADERSIZE/8; ++i)
	{
		new_buf[i] = flags.buffer[i];
	}
	sent += HEADERSIZE/8;
	//calling the socket library send function
	return ::Send(sock, new_buf, sent, dest);
}
int NetworkStack::Receive(char* buffer, int max_bytes, sockaddr_in* location)
{
	//calling the socket library Receive function
	int recv = ::Receive(sock, buffer, max_bytes);
	//packet cant be less than the header size, the packet must be bad
	if (recv < HEADERSIZE/8)
	{
		return 0;
	}
	//set the flags
	BitArray<HEADERSIZE> flags;
	for (unsigned i = 0; i < HEADERSIZE/8; ++i)
	{
		flags.buffer[i] = buffer[i];
	}
	//shift the packet
	for (int i = 0; i < recv - HEADERSIZE/8; ++i)
	{
		buffer[i] = buffer[i+HEADERSIZE/8];
	}
	recv -= HEADERSIZE/8;
	//call the rest of the layers
	for (unsigned i = 0; i < layers.size(); ++i)
	{
		recv = layers[i]->Receive(buffer, recv, location, flags);
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
void NetworkStack::AddLayer(NetworkLayer *layer)
{
	layer->layer_id = layers.size();
	layer->stack = this;
	layers.push_back(layer);
}