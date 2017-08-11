/*!
  \author
    Wyatt Lavigueure
  \date
    7/23/2017
  \brief
    A proram to test the database, this program will take messages with 2D position and a string of text and save it to a database
    when a new client is connected the saved messages are sent to each client
*/
#include <vector>
#include <iostream>

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

//this should be about a day(I rounded to nearest fun number)
#define MAXAGE 1337

/*!
  \brief
    This class holds message info
*/
class Message
{
public:
  /*!
    \brief
      x position
  */
  float xpos;
  /*!
    \brief
      y position
  */
  float ypos;
  /*!
    \brief
      time the message has been alive
  */
  int age;
  /*!
    \brief
      the message
  */
  char *msg;
  /*!
    \brief
      The lnegth of the message
  */
  int length;
  /*!
    \brief
      This is to check wether the message has been completely filled in
      done starts at 0 and increment it each time a varible is filled in
      so once done is equal to 4 it can be sent
  */
  int done;
  /*!
    \brief
      initalize done to 0
  */
  Message()
  {
    done = 0;
  }
};

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
      maps nonce from create message to data to save in the database
  */
  std::unordered_map<int, std::pair<char*, int> > saved;
  /*!
    \brief
      This number will be decremented periodically (each find request from database). It is used to know what age
      of messages to look up in the database
  */
  int counter = MAXAGE;
  /*!
    \brief
      This is a number to use for the nonce when sending a create message, after using it increment it so that
      it will be different nex time. just let it roll, over after 4 billion nonces we can reuse them.
  */
  int nonce;
  /*!
    \brief
      An array of the new connections, each new connection is represented as a pair of the client address and the counter value
      when the connection was added. When the counter gets back to the new connections start then its removed since its no longer new 
  */
  std::vector<std::pair<sockaddr_in, int> > new_conn;
  /*!
    \brief
      a place to gather all the data from the database before sending.
  */
  std::unordered_map<unsigned, Message> partials;
  /*!
    \brief
      function is called when the database sends back info
    \param data
      Pointer to a Network event
  */
  void GetCallback(void *data)
  {
        std::cout << "porn 1" << std::endl;
    NetworkEvent *event = static_cast<NetworkEvent*>(data);
    unsigned id = *reinterpret_cast<unsigned*>(event->buffer+1);
    unsigned row = *reinterpret_cast<unsigned*>(event->buffer+1+sizeof(unsigned));
    switch(row)
    {
      case 0:
      {
        partials[id].xpos = *reinterpret_cast<float*>(event->buffer+1+sizeof(unsigned)*2);
        partials[id].done += 1;
        break;
      }
      case 1:
      {
        partials[id].ypos = *reinterpret_cast<float*>(event->buffer+1+sizeof(unsigned)*2);
        partials[id].done += 1;
        break;
      }
      case 2:
      {
        //get the length of the string returned, it should always be the same size
        unsigned length = event->length - sizeof(unsigned) * 2 -1;
        char *str = new char[length];
        memcpy(str, event->buffer+1+sizeof(unsigned)*2, length);
        partials[id].msg = str;
        partials[id].length = length;
        partials[id].done += 1;
        break;
      }
    }
    //if the partial is completed by this message then send it and delete it from memory
    if (partials[id].done >= 4)
    {
      char buffer[MAXPACKETSIZE];
      *reinterpret_cast<float*>(buffer) = partials[id].xpos;
      *reinterpret_cast<float*>(buffer+sizeof(float)) = partials[id].ypos;
      *reinterpret_cast<int*>(buffer+sizeof(float)*2) = partials[id].age;
      memcpy(buffer+sizeof(float)*2+sizeof(int), partials[id].msg, partials[id].length);
      //send to every new connection
      for (unsigned i = 0; i < new_conn.size(); ++i)
      {
        std::cout << "sending mesage to new clients" << std::endl;
        stack->Send(buffer, sizeof(float)*2+sizeof(int)+partials[id].length, &new_conn[i].first, *flags);
      }
      //remove partial since it is no longer a partial
      delete [] partials[id].msg;
      partials.erase(partials.find(id));
    }
        std::cout << "porn 2" << std::endl;
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
    int s_nonce = *reinterpret_cast<int*>(event->buffer+1);
    unsigned id = *reinterpret_cast<unsigned*>(event->buffer+1+sizeof(int));
    char *msg_data = saved[s_nonce].first;
    int length = saved[s_nonce].second;
    //buffer to store messages to send
    char buffer[MAXPACKETSIZE];
    //send x position
    int n = CreateSetMessage(buffer, id, 0, msg_data, sizeof(float));
    stack->Send(buffer, n, event->addr, *flags);
    //send y position
    n = CreateSetMessage(buffer, id, 1, msg_data+sizeof(float), sizeof(float));
    stack->Send(buffer, n, event->addr, *flags);
    //send life time
    n = CreateSetMessage(buffer, id, 2, msg_data+sizeof(float)*2, sizeof(int));
    stack->Send(buffer, n, event->addr, *flags);
    //send text
    n = CreateSetMessage(buffer, id, 3, msg_data+sizeof(float)*2+sizeof(int), length - sizeof(float)*2+sizeof(int));
    stack->Send(buffer, n, event->addr, *flags);
    //remove the saved data
    delete [] msg_data;
    saved.erase(saved.find(s_nonce));
  }
    /*!
    \brief
      callback function for when a find response comes from db
    \param data
      Pointer to a Network event
  */
  void FindCallback(void *data)
  {
    //remove old connections from new_conn
    for (unsigned i = 0; i < new_conn.size(); ++i)
    {
      if (new_conn[i].second == counter - 1)
      {
        std::cout << "new connection done" << std::endl;
        new_conn.erase(new_conn.begin()+i);
        i -= 1;
        continue;
      }
    }
    NetworkEvent *event = static_cast<NetworkEvent*>(data);
    //the number of ids in the find call
    int num = event->buffer[sizeof(unsigned)*2+1];
    //the row its searching for
    unsigned row = *reinterpret_cast<unsigned*>(event->buffer+1);
    //make sure its the time alive row
    if (row != 2)
    {
      return;
    }
    //pointer to the start of the array of ids
    unsigned *ary = reinterpret_cast<unsigned*>(event->buffer+sizeof(unsigned)*2+2);
    //the amount of time these guys have been alive for
    unsigned life_time = *reinterpret_cast<unsigned*>(event->buffer+sizeof(unsigned)+1);
    //buffer to store messages to send
    char buffer[MAXPACKETSIZE];
    //increase the time alive
    life_time += 1;
    //set the time alive for all the objects we found
    for (int i = 0; i < num; ++i)
    {
      int n = CreateSetMessage(buffer, ary[i], 2, &life_time, sizeof(life_time));
      stack->Send(buffer, n, event->addr, *flags);
    }
    //request all info about each id to send to clients
    for (int i = 0; i < num; ++i)
    {
      int n = CreateGetMessage(buffer, ary[i], 0);
      stack->Send(buffer, n, event->addr, *flags);
      n = CreateGetMessage(buffer, ary[i], 1);
      stack->Send(buffer, n, event->addr, *flags);
      n = CreateGetMessage(buffer, ary[i], 2);
      stack->Send(buffer, n, event->addr, *flags);
      n = CreateGetMessage(buffer, ary[i], 3);
      stack->Send(buffer, n, event->addr, *flags);
    }
    //if there was 100 ids then there are more coming
    //otherwise decrement the counter and send a find request to the database for the next batch of ids
    if (num != 100)
    {
      counter -= 1;
      if (counter < 0)
      {
        counter = MAXAGE;
      }
      int n = CreateFindMessage(buffer, 2, &counter, sizeof(counter));
      stack->Send(buffer, n, event->addr, *flags);
    }
  }
};

int main()
{
    //load in config file
  Config config;
  config.Init("TextMMO.conf");

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
  std::string ip = static_cast<std::string>(config.properties["db_ip"]);
  CreateAddress(ip.c_str(), static_cast<int>(config.properties["db_port"]), &db_addr);
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
  //start the infinite loop of findng and updating 
  int msg_len = CreateFindMessage(buffer, 2, &obj.counter, sizeof(obj.counter));
  stack.Send(buffer, msg_len, &db_addr, flags);
  //main loop
  while (1)
  {
    //using the black magic number
    int n = stack.Receive(buffer, 666, &from);
    if (n > 0)
    {
      //this check is not normally needed but the messages from the client have
      //no message type in the example program
      if (from == db_addr)
      {
        //call the correct remote db functions
        switch(buffer[0])
        {
          case Protocol::DatabaseGet:
          {
            std::cout << "DatabaseGet" << std::endl;
            remotedb.ProcessGet(buffer, n, &from);
            break;
          }
          case Protocol::DatabaseCreate:
          {
            std::cout << "DatabaseCreate" << std::endl;
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
      //got a beautyful message
      else
      {
        if (n <= int(sizeof(float)) * 2)
        {
          std::cout << "new connection" << std::endl;
          //this means its a new connection add the address to the vector of new connections
          obj.new_conn.push_back(std::make_pair(from, obj.counter));
        }
        else
        {
          std::cout << "client message" << std::endl;
          //this is a new message
          std::cout << buffer + sizeof(float) * 2 << std::endl;
          //shift the string over by one int to make room for time alive
          for (unsigned i = n-1; i >= sizeof(float)*2; --i)
          {
            buffer[i+sizeof(int)] = buffer[i];
          }
          //its a new message so time alive is 0
          *reinterpret_cast<int*>(buffer + sizeof(float)*2) = 0;
          //send the message to every connection except the db, and the person who ent it
          for (auto it = stack.connections.begin(); it != stack.connections.end(); ++it)
          {
            if (it->first == db_addr || it->first == from)
            {
              continue;
            }
            stack.Send(buffer, n+sizeof(int), &it->first, flags);
          }
          //make a create message to send to database
          char buf[1+sizeof(int)];
          int msg_size = CreateCreateMessage(buf, obj.nonce);
          //save the message so we can set the set the values later
          char *save = new char[n+sizeof(int)];
          memcpy(save, buffer, n+sizeof(int));
          obj.saved[obj.nonce] = std::make_pair(save, n+sizeof(int));
          //increment nonce
          obj.nonce += 1;
          //send create msg to database
          stack.Send(buf, msg_size, &db_addr, flags);
        }
      }
    }
    stack.Update();
  }
}