/*!
  \author
    Wyatt Lavigueure
  \date
    9/26/2017
  \brief
    A proram to test the database. The program expects there two be databases running, one is the player database each object
    has HP which is a float, a level which is an int and an equided item which is a reference to an item. The second table is a table of items
    each item has a name which is a string and a damage which is a float.
*/
#include <vector>
#include <iostream>
#include <chrono>

#include "meta.h"
#include "config.h"
#include "channel.h"
#include "network_stack.h"
#include "blowfish.h"
#include "network_stack.h"
#include "bit_array.h"
#include "reliability.h"
#include "prioritization.h"
#include "encryption.h"
#include "protocol.h"
#include "remote_database.h" 

/*!
  \brief
    class that will have member function for the database callback
*/
class DatabaseCallback
{
public:
  /*!
    \brief
      refrence to the network stack for sending things
  */
  NetworkStack *stack;
  /*!
    \brief
      refrence to the flags for sending things
  */
  BitArray<HEADERSIZE> *flags;
  /*!
    \brief
      function is called when the database sends back info
    \param data
      Pointer to a Network event
  */
  void GetCallback(void *data)
  {
    NetworkEvent *event = static_cast<NetworkEvent*>(data);
    std::cout << "get" << event << std::endl;
  }
  /*!
    \brief
      callback function for when a new object is added to the database
    \param data
      Pointer to a Network event
  */
  void CreateCallback(void *data)
  {
    NetworkEvent *event = static_cast<NetworkEvent*>(data);
    int nounce = *reinterpret_cast<int*>(event->buffer+1);
    unsigned id = *reinterpret_cast<unsigned*>(event->buffer+1+sizeof(int));
    std::cout << "create for nounce " << nounce << " id = " << id << std::endl;

    //set the to random values
    char buffer[100];
    //hp is set between 0 and 100
    float hp = rand() / (double)RAND_MAX * 100;
    int n = CreateSetMessage(buffer, id, 0, &hp, sizeof(float));
    stack->Send(buffer, n, event->addr, *flags);

    //level also between 0 and 100
    int level = rand() % 100;
    n = CreateSetMessage(buffer, id, 1, &level, sizeof(int));
    stack->Send(buffer, n, event->addr, *flags);

    //null pointer
    char null_reference[8] = {0};
    n = CreateSetMessage(buffer, id, 2, &null_reference, 8);
    stack->Send(buffer, n, event->addr, *flags);
  }
  /*!
    \brief
      used for perfomance testing
  */
  std::chrono::high_resolution_clock::time_point timer;
    /*!
    \brief
      used for perfomance testing
  */
  double time_spent;
    /*!
    \brief
      used for perfomance testing
  */
  double average = 0;
  /*!
    \brief
      callback function for when a find response comes from db
    \param data
      Pointer to a Network event
  */
  void FindCallback(void *data)
  {
    NetworkEvent *event = static_cast<NetworkEvent*>(data);
    std::cout << "found" << event << std::endl;
  }
};

int main()
{
  //load in config file
  Config config;
  config.Init("test_database.conf");

  //setup the network stack
  Init();
  SOCKET sock = CreateSocket(IPPROTO_UDP);
  sockaddr_in local;
  CreateAddress(0, static_cast<int>(config.properties["port"]),&local);
  Bind(sock, &local);
  SetNonBlocking(sock);
  NetworkStack stack(sock);
  Channel *channel = new Channel();
  Reliability *reliability = new Reliability();
  Encryption *encryption = new Encryption();
  stack.AddLayer(channel);
  stack.AddLayer(reliability);
  stack.AddLayer(encryption);
  //the flags for sending messages
  BitArray<HEADERSIZE> flags;
  flags.SetBit(ReliableFlag);
  //buffer for receiving messages
  char buffer[MAXPACKETSIZE];
  //the address we recieve from
  sockaddr_in from;

  //create the addres for the database
  sockaddr_in db_addr;
  std::string ip = static_cast<std::string>(config.properties["player_db_ip"]);
  CreateAddress(ip.c_str(), static_cast<int>(config.properties["player_db_port"]), &db_addr);
  //make the remote database
  RemoteDatabase remotedb;
  //make a callback class to recieve the messages from the database
  DatabaseCallback obj;
  obj.stack = &stack;
  obj.flags = &flags;
  //subscribe call back functions
  remotedb.db_events[db_addr].ConnectEvent(DatabaseGetEvent, &obj, &DatabaseCallback::GetCallback);
  remotedb.db_events[db_addr].ConnectEvent(DatabaseCreateEvent, &obj, &DatabaseCallback::CreateCallback);
  remotedb.db_events[db_addr].ConnectEvent(DatabaseFindEvent, &obj, &DatabaseCallback::FindCallback);

  int nounce = 0;
  double timer = 0;
  //main loop
  while (1)
  {
    //using the black magic number
    int n = stack.Receive(buffer, 666, &from);
    if (n > 0)
    {
      //call the correct remote db functions
      switch(buffer[0])
      {
        case Protocol::DatabaseGet:
        {
          remotedb.ProcessGet(buffer, n, &from);
          break;
        }
        case Protocol::DatabaseCreate:
        {
          remotedb.ProcessCreate(buffer, n, &from);
          break;
        }
        case Protocol::DatabaseFind:
        {
          remotedb.ProcessFind(buffer, n, &from);
          break;
        }
      }
    }

    //create a player every second
    if (timer > 0.05)
    {
      int msg_len = CreateCreateMessage(buffer, nounce++);
      stack.Send(buffer, msg_len, &db_addr, flags);
      timer = 0;
    }
    timer += stack.timer.GetTime();


    stack.Update();
  }
}