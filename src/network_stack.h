/*!
\author Wyatt Lavigueure
\date   5/5/2017
\brief  
  The network stack. When a message is to be sent over the internet it has(should?) to go through here!
*/
#ifndef NTSTCK_H
#define NTSTCK_H
#include <vector>
#include <unordered_map>

#include "wyatt_sock.h"
#include "frame_rate.h"
#include "bit_array.h"

/*!
  \brief
    This is what all packet buffers should be set to to insure that they are large enough. No checks are made so dont be stupid
*/
#define MAXSOCKETSIZE 2048
/*!
  \brief
    The number of flags in each packet
*/
#define HEADERSIZE 8

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
	  \param flags
	    The flags for the packet to have, example would be reliable flag or a encrypt flag. These are added onto the packet
	    right before its sent
	  \return
	    The number of bytes "sent" wich is just the new size of the buffer
	*/
	virtual int Send(char* buffer, int bytes, sockaddr_in* dest, BitArray<HEADERSIZE> &flags) = 0;
	/*!
	  \brief
	    recieve function similar to the send function but for sending things.
	  \param buffer
	    The location to plop the data
	  \param bytes
	    The number of bytes in the packet
	  \param location
	    The entity that we recieved from
	  \param flags
	    The flags that the packet was sent with, example would be reliable flag or a encrypt flag.
	    These are extracted when the packet is received 
	  \return
	    The number of bytes "recieved" which is how much of the buffer is left
	*/
	virtual int Receive(char* buffer, int bytes, sockaddr_in* location, BitArray<HEADERSIZE> &flags) = 0;
	/*!
	  \brief
	    This is ment to be called every frame
	  \param dt
	    the time since the last updae called
	*/
	virtual void Update(double dt) = 0;
private:
	//let the network stack set the stack pointer and id
	friend class NetworkStack;
	//pointer back to the network stack that its a part of
	NetworkStack *stack;
	//the index into the array of layers
	int layer_id;
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
      Adds a layer to the top of network stack (i.e calls push_back) and sets the layers layer number
      and sets the networkstack pointer
    \param layer
      A pointer to the layer to add
  */
  void AddLayer(NetworkLayer *layer);
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
	  \param flags
	    The flags for the packet to have, example would be reliable flag or a encrypt flag
	  \param start_layer
	    The layer to start sending from so that one can skip layers, usually for reliability layer
	    so that when it resends a packet it can start from itself.
	  \return
	    The number of bytes sent on the wire, almost garanteed to be different from bytes
	*/
	int Send(SOCKET sock, const char* buffer, int bytes, sockaddr_in* dest, BitArray<HEADERSIZE> &flags, int start_layer = -1);
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
  /*!
    \brief
      The connection information for ever entity we have communicated with
  */
  std::unordered_map<sockaddr_in, ConnectionState, SockAddrHash> connections;
private:
	FrameRate timer;
};

#endif