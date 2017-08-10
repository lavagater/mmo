/*!
  \author
    Wyatt Lavigueure
  \date
    8/7/2017
  \brief
    Handles database messages on the server side.
*/
#ifndef REMOT_H
#define REMOT_H

#include <unordered_map>

#include "network_stack.h"
#include "event.h"
/*!
  \brief
    Glorified wrapper around the event system so that objects can asyncronously wait for database responses.
    The average use case for the remote database is when you send a requst to the database you subscribe to 
    the corrisponding event use the address of the database to get the correct event system. Then when the
    response is recieved your function will be called with the buffer from the server the length and the server
    The callback function will be responsible for making sure that the buffer contains the info it wanted from the server
    i.e. Database response for DatabaseGet for object id 0 will call the same event as DatabaseGet with any other id
    so the callback function will have to check the buffer that the id and row is what it wants. In the call back function
    you could unsubscribe from the event because you have the info you wanted or you could leave it subscribed and keep
    updating the info. 
*/
class RemoteDatabase
{
public:
  /*!
    \brief
      A map that maps database address to an event system. so that a get message from the player database wont be confused for
      a get from the monster database
  */
  std::unordered_map<sockaddr_in, Event, SockAddrHash> db_events;
  /*!
    \brief
      Sends DatabaseGet event with the buffer and length
    \param buffer
      The buffer from the NetworkStack::Recieve call including the message type
    \param length
      The length of the buffer, the return value of NetworkStack::Recieve
    \param from
      The address of the database that sent the message, the same from that is
      filled in by NetworkStack::Recieve
  */
  void ProcessGet(char *buffer, int length, sockaddr_in *from);
  /*!
    \brief
      Sends DatabaseCreate event with the buffer and length
    \param buffer
      The buffer from the NetworkStack::Recieve call including the message type
    \param length
      The length of the buffer, the return value of NetworkStack::Recieve
    \param from
      The address of the database that sent the message, the same from that is
      filled in by NetworkStack::Recieve
  */
  void ProcessCreate(char *buffer, int length, sockaddr_in *from);
  /*!
    \brief
      Sends DatabaseFind event with the buffer and length
    \param buffer
      The buffer from the NetworkStack::Recieve call including the message type
    \param length
      The length of the buffer, the return value of NetworkStack::Recieve
    \param from
      The address of the database that sent the message, the same from that is
      filled in by NetworkStack::Recieve
  */
  void ProcessFind(char *buffer, int length, sockaddr_in *from);

};
#endif