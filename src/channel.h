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
	  The class that handles all of the connection state
*/
class Channel : public NetworkLayer
{
public:
	int Send(char* buffer, int bytes, sockaddr_in* dest);
	int Receive(char* buffer, int bytes, sockaddr_in* location);
	void Update(double dt);
private:
};

#endif