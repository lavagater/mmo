/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  The lowest part of the network stack before the operating system takes over. The channel layer 
  is responsible for connection state and authentication
*/
#ifndef CHAN_H
#define CHAN_H
#include <unordered_map>

#include "wyatt_sock.h"

/*!
	\brief
	  The connection state of a connection also stores the address and the authentication and id
*/
class ConnectionState
{
public:
	// An enum of type of connections
	enum
	{
		NotConnected,
		Connected,
		AuthedPlayer,
		AuthedServer,
		AuthedAdmin,
		num
	};
	int auth_level; /*!< The level of authentication*/
	sockaddr_in *addr; /*!< The address*/
	unsigned connection_id; /*!< This is a unique id for this connection shared along all the onnections*/
};

/*!
	\brief
	  A functor for hashing a connection state
*/
class SockAddrHash
{
public:
	/*!
		\brief
		  Hash function for unordered map
	*/
	size_t operator()(const sockaddr_in &rhs) const;
};

/*!
	\brief
	  Comparison for sockaddr_in so that it can be used as a key
*/
bool operator==(const sockaddr_in &lhs, const sockaddr_in &rhs);

/*!
	\brief
	  The class that handles all of the connection state
*/
class Channel
{
public:
	/*!
	  \brief
	    Get a connection state
	  \param addr
	    The address of the connection
	  \return
	    a pointer to the connection state. returns null if not found
	*/
	ConnectionState *GetConnection(sockaddr_in addr);
	/*!
	  \brief
	    Sets a connections authentication if the connection exists
	  \param addr
	    The address of the connection
	  \param auth_level
	    The new authentication level
	*/
	void SetAuth(sockaddr_in addr, int auth_level);
private:
	std::unordered_map<sockaddr_in, ConnectionState, SockAddrHash> connections;
};

#endif