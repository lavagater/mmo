/*!
\author Wyatt Lavigueure
\date   5/5/2017
\brief  
  The network stack. When a message is to be sent over the internet it has(should?) to go through here! Note that the 
  network stack should have the same layers on both ends of the connection.
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
#define MAXPACKETSIZE 2048
/*!
  \brief
    The number of flags in each packet
*/
#define HEADERSIZE 8

/*!
  \brief
    Error message when a packet coming through the network stack is not something sent from the network stack
*/
#define MALEFORMEDPACKET -123
/*!
  \brief
    Error mesage when trying to send an encrypted message to a connection that does not have a shared key
*/
#define NOENCRYPTIONKEY -124
/*!
  \brief
    Error message for when messages are being sent to fastfor the amount of bandwidth
*/
#define BANDWIDTHLIMIT -125
/*!
  \brief
    enum for header flags
*/
enum
{
	//Makes the packet reliable
	ReliableFlag,
	//respone to the reliabile message
	AckNumberFlag,
	//this is a message for the networ stack
	MessageTypeFlag,
	//the message is encrypted using shared key
	EncryptFlag
};
/*!
  /brief
    enum for message types
*/
enum
{
	Ping,
	Pong,
	NumMsgTypes
};

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
	virtual int Send(char* buffer, int bytes, const sockaddr_in* dest, BitArray<HEADERSIZE> &flags) = 0;
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
	/*!
	  \brief
	    This addr was removed from connections and if this addr is being used in a layer it should remove it as well
	*/
	virtual void RemoveConnection(const sockaddr_in *addr) = 0;
	/*!
	  \brief
	    The network stack that this layer is a part of
	*/
	NetworkStack *stack;
	/*!
	  \brief
	    the index into the array of layers
	*/
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
	double ping = 0.05; /*!< The ping in seconds for this connection, a rolling average of pings*/
	float time_since_ping;/*!< The time since this connection responded to a ping, used to determine if dissconnected*/
	float ping_timer; /*!< The time until another ping will be sent*/
	int auth_level; /*!< The level of authentication*/
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
	    Sets the socket to use
	  \param socket
	    the socket for this network stack to use
	*/
	NetworkStack(SOCKET socket);
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
	int Send(const char* buffer, int bytes, const sockaddr_in* dest, BitArray<HEADERSIZE> &flags, int start_layer = -1);
	/*!
	  \brief
	    Recieves data from wire then sends it through every layers Receive function in opposite order
	  \param buffer
	    The location to plop the data
	  \param max_bytes
	    The largest packet that fits into the buffer
	  \param location
	    The entity that we recieved from
	  \return
	    The number of bytes that are in buffer, not the number of bytes read from the wire
	*/
	int Receive(char* buffer, int max_bytes, sockaddr_in *location);
	/*!
	  \brief
	    This is ment to be called every frame
	*/
	void Update();
	/*!
	  \brief
	    Removes a connection
	  \param addr
	    the address of the connection to remove
	*/
	void RemoveConnection(const sockaddr_in *addr);
	/*!
	  \brief
	    Calculates the bandwidth being used by the program
	*/
	double GetBandwidth();
  /*!
    \brief
      The connection information for ever entity we have communicated with
  */
  std::unordered_map<sockaddr_in, ConnectionState, SockAddrHash> connections;
  /*!
    \brief
      The timer used to update network layers and time ping 
  */
	FrameRate timer;
	/*!
	  \brief
	    The last error that was encountered
	*/
	int last_error;
private:
	SOCKET sock;
	//bytes_sent time_start and time split are for getting the bandwidth
	unsigned bytes_sent;
	double time_start;
	double time_split;
};

#endif