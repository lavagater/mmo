/*!
  \author
    Wyatt Lavigueure
  \date
    7/24/2017
  \brief
    An enum for all the different types of events. All custom events should be added hear. This file is also where structs that get passed
    for different events are defined
*/
#ifndef EVENT_TYP_H
#define EVENT_TYP_H

#include "wyatt_sock.h" /*for sockaddr_in*/
/*!
  \brief
    The enum of types of events
*/
enum EventType
{
   TestEvent,
   TestEvent2,
   DatabaseGetEvent,
   DatabaseCreateEvent,
   DatabaseFindEvent,
   Num
};

/*!
  \brief
    Contains pointer to the buffer and the length of the buffer and the sender address
*/
class NetworkEvent
{
public:
  /*!
    \brief
      The entire unmodified buffer from the recieve call
  */
  char *buffer;
  /*!
    \brief
      number of bytes in the buffer
  */
  int length;
  /*!
    \brief
      The address that the message was sent from
  */
  sockaddr_in *addr;
  /*!
    \brief
      constructor to initalize the values
    \param buffer
      The entire unmodified buffer from the recieve call
    \param length
      length of buffer
    \param addr
      The address that the message was sent from
  */
  NetworkEvent(char *buffer, int length, sockaddr_in *addr) : buffer(buffer), length(length), addr(addr){}
};
#endif