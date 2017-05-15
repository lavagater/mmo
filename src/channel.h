/*!
\author Wyatt Lavigueure
\date   5/4/2017
\brief  
  The lowest part of the network stack before the operating system takes over. The channel layer 
  is responsible for connection state and authentication
*/
#ifndef CHAN_H
#define CHAN_H

#include "wyatt_sock.h"
#include "network_stack.h"

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
	int Send(char* buffer, int bytes, sockaddr_in* dest, BitArray<HEADERSIZE> &flags);
	/*!
	  \brief
	    The channel recieve does nothing
	*/
	int Receive(char* buffer, int bytes, sockaddr_in* location, BitArray<HEADERSIZE> &flags);
	/*!
	  \brief
	    The channel updates nothing
	*/
	void Update(double dt);
private:
};

#endif