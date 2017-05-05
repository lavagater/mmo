

#include "channel.h"

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

ConnectionState *Channel::GetConnection(sockaddr_in addr)
{
	if (connections.find(addr) != connections.end())
	{
		return &connections[addr];
	}
	//not exist
	return 0;
}
void Channel::SetAuth(sockaddr_in addr, int auth_level)
{
	ConnectionState *con = GetConnection(addr);
	if (con == 0)
	{
		return;
	}
	con->auth_level = auth_level;
}