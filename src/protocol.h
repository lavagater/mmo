/*!
  \author
    Wyatt Lavigueure
  \date
    8/6/2017
  \brief
    This file defines a protocol for networking in the game engine. This protocol is used for Database and game logic or any other 
    uses. This protocol is not used for the underlying network stack, i.e. The network stack has its own protocol witch is
    decoupled from this protocol.
*/
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
  num
};