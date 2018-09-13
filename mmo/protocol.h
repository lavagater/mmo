/*!
  \author
    Wyatt Lavigueure
  \date
    9/8/2018
  \brief
    This file defines a protocol for networking in the game engine. This protocol is used for Database and game logic or any other 
    uses. This protocol is not used for the underlying network stack, i.e. The network stack has its own protocol witch is
    decoupled from this protocol. There should be a function for each enum that will put info into the char* to be sent, these function
    go in a seperate file.
*/
#ifndef PROTOCOL_H
#define PROTOCOL_H

enum Protocol
{
  //Each Database message type has a different meaning depending on who is recieving the message
  //ex. DatabaseGet message is asking the database for a value in the database but
  //DatabaseGet is also a message containing a value from the database
  DatabaseGet,
  DatabaseSet,
  DatabaseCreate,
  DatabaseFind,
  DatabaseDelete,
  //these are for the load balancer protocol
  EncryptionKey,
  num
};

//number of bytes the message type is
const int message_type_size = 2; 

#endif