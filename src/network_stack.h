/*!
\author Wyatt Lavigueure
\date   5/5/2017
\brief  
  The network stack. When a message is to be sent over the internet it has(should?) to go through here!
*/
#ifndef NTSTCK_H
#define NTSTCK_H
#include <vector>

#include "wyatt_sock.h"
#include "frame_rate.h"

/*!
  \brief
    This is what all packet buffers should be set to to insure that they are large enough. No checks are made so dont be stupid
*/
#define MAXSOCKETSIZE 2048

//forward decleration so that NetworkLayer can use
class NetworkStack;

/*!
  \brief
    Base class for the layers of the network stack
*/
class NetworkLayer
{
public:
	/*!
	  \brief
	    Destructor for network layer, virtual so that NetworkLayer babies destructors also get called
	*/
  virtual ~NetworkLayer();
	/*!
	  \brief
	    Send function does not actually send anything over the wire but what ever changes it makes
	    to the packet are kept and will be sent.
	  \param buffer
	    the data that wants to be sent. important! must have extra room in the buffer for added on things.
	  \param bytes
	    the number of bytes in buffer
	  \param dest
	    The address of the entity being sent to
	  \return
	    The number of bytes "sent" wich is just the new size of the buffer
	*/
	virtual int Send(char* buffer, int bytes, sockaddr_in* dest) = 0;
	/*!
	  \brief
	    recieve function similar to the send function but for sending things.
	  \param buffer
	    The location to plop the data
	  \param bytes
	    The number of bytes in the packet
	  \param location
	    The entity that we recieved from
	  \return
	    The number of bytes "recieved" which is how much of the buffer is left
	*/
	virtual int Receive(char* buffer, int bytes, sockaddr_in* location) = 0;
	/*!
	  \brief
	    This is ment to be called every frame
	  \param dt
	    the time since the last updae called
	*/
	virtual void Update(float dt) = 0;
private:
	//pointer back to the network stack that its a part of
	NetworkStack *stack;
	//the index into the array of layers
	int layer_id;
};

/*!
  \brief
    The network stack
*/
class NetworkStack
{
public:
	/*!
	  \brief
	    Frees the network layers
	*/
	~NetworkStack();
	/*!
	  \brief
	    The layers of the network stack
	*/
  std::vector<NetworkLayer*> layers;
	/*!
	  \brief
	    Calls the send functions of each layer, then actaully sends the data
	  \param sock
	    the socket to send from
	  \param buffer
	    the data that wants to be sent. important! must have extra room in the buffer for added on things.
	  \param bytes
	    the number of bytes in buffer
	  \param dest
	    The address of the entity being sent to
	  \param start_layer
	    The layer to start sending from so that one can skip layers, usually for reliability layer
	    so that when it resends a packet it can start from itself.
	  \return
	    The number of bytes sent on the wire, almost garanteed to be different from bytes
	*/
	int Send(SOCKET sock, const char* buffer, int bytes, sockaddr_in* dest, int start_layer = -1);
	/*!
	  \brief
	    Recieves data from wire then sends it through every layers Receive function in opposite order
	  \param sock
	    The socket to recieve from
	  \param buffer
	    The location to plop the data
	  \param max_bytes
	    The largest packet that fits into the buffer
	  \param location
	    The entity that we recieved from
	  \return
	    The number of bytes that are in buffer, not the number of bytes read from the wire
	*/
	int Receive(SOCKET sock, char* buffer, int max_bytes, sockaddr_in *location);
	/*!
	  \brief
	    This is ment to be called every frame
	*/
	void Update();
private:
	FrameRate timer;
};

#endif