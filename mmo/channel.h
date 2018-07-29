/*!
\author Wyatt Lavigueure
\date   5/4/2017
\brief  
  Channel layer handles connections, gets ping times, and removes connections if they dont respond to pings.
*/
#ifndef CHAN_H
#define CHAN_H

#include "wyatt_sock.h"
#include "network_stack.h"

/*!
  \brief
    how often to send pings in seconds 
*/
#define TIME_BETWEEN_PINGS 0.25

/*!
  \brief
    The mount of time to wait until disconnecting a nonresponsize connection
*/
#define DISCONNECT_TIME 1*TIME_BETWEEN_PINGS

/*!
	\brief
	  This class might do something in the future
*/
class Channel : public NetworkLayer
{
public:
	/*!
	  \brief
	    The channel send does not do anything atm
	*/
	int Send(char* buffer, int bytes, const sockaddr_in* dest, BitArray<HEADERSIZE> &flags);
	/*!
	  \brief
	    The channel recieve checks if the message is a ping message
	*/
	int Receive(char* buffer, int bytes, sockaddr_in* location, BitArray<HEADERSIZE> &flags);
	/*!
	  \brief
	    The channel updates occasianally sends pings to its connections
	*/
	void Update(double dt);
	void RemoveConnection(const sockaddr_in*addr);
private:
};

#endif