#include <string.h>

#include "network_stack.h"
#include "logger.h"

NetworkLayer::~NetworkLayer(){}

NetworkStack::NetworkStack(SOCKET socket)
{
	sock= socket;
	bytes_sent = 0;
	time_start = 0;
	time_split = 0;
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

int NetworkStack::Send(const char* buffer, int bytes, const sockaddr_in* dest, BitArray<HEADERSIZE> &flags, int start_layer)
{
	//create a new buffer that can be added too
	char new_buf[MAXPACKETSIZE] = {0};//set to all zero for debugging
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
		//if a layer sends 0 bytes then we stop
		if (sent <= 0)
		{
			last_error = sent;
			return sent;
		}
	}
	//put flags into buffer
	for (unsigned i = 0; i < HEADERSIZE/8; ++i)
	{
		new_buf[i] = flags.buffer[i];
	}
	sent += HEADERSIZE/8;
	//update bytes sent for bandwidth
	bytes_sent += sent;
	//calling the socket library send function
	return ::Send(sock, new_buf, sent, dest);
}
int NetworkStack::Receive(char* buffer, int max_bytes, sockaddr_in* location)
{
	//calling the socket library Receive function
	int recv = ::Receive(sock, buffer, max_bytes, location);
	if (recv < 0)
	{
		last_error= GetError();
		return last_error;
	}
	LOG("Recieve " << recv << " bytes");
	//packet cant be less than the header size, the packet must be bad
	if (recv < HEADERSIZE/8)
	{
		last_error = MALEFORMEDPACKET;
		return MALEFORMEDPACKET;
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
double NetworkStack::GetBandwidth()
{
	return bytes_sent / (timer.GetTotalTime() - time_start);
}
void NetworkStack::Update()
{
	//refresh bandwidth
	double temp = GetBandwidth();
	time_start = time_split;
	time_split = timer.GetTotalTime();
	bytes_sent = temp * (time_split-time_start);

	//update layers
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
void NetworkStack::RemoveConnection(const sockaddr_in *addr)
{
	//remove the connection from all the layers first so they can still see what the connection was
	for (unsigned i = 0; i < layers.size(); ++i)
	{
		layers[i]->RemoveConnection(addr);
	}

	//then actually remove the connection
	connections.erase(connections.find(*addr));
}
ConnectionState::ConnectionState()
{
	ping = 0.05; 
	time_since_ping = 0;
	ping_timer = 0;
	auth_level = 0;
	static unsigned counter = 0;
	counter += 1;
	connection_id = counter;
}