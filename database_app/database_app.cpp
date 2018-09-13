/*****************************************************************************/
/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  The entry point for the database executable. The database trusts its users (since the users have full control of the data) so it
  does minimal error checking with packets. This is my first time ever writing a database so it might not be what a veteran would 
  assume so you might want to double check what things do before using, documentaion might use lingo incorrectly be careful(This applys
  for all the database related files).

*/
/*****************************************************************************/
#include <math.h>
#include <iostream>

#include "database.h"
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
#include "logger.h"

int main()
{
  std::cout << "start" << std::endl;
  //load in config file
  Config config;
  config.Init("database.conf");
  //load the database
  std::string file = static_cast<std::string>(config.properties["table"])+".tbl";
  std::cout << "made name" << std::endl;
  Database db(file.c_str());
  std::cout << "created db" << std::endl;
  //setup the network stack
  Init();
  SOCKET sock = CreateSocket(IPPROTO_UDP);
  sockaddr_in local;
  CreateAddress(0,static_cast<int>(config.properties["port"]),&local);
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
  std::cout << "entering while loop" << std::endl;
  //main loop
  while(true)
  {
    //check for messages
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
    //make sure the message is big enough for the message type
    if (n >= message_type_size)
    {
      Protocol type;
      //make sure the type is zeroed out because, the protocol size 
      //might be larger than the type size
      memset(&type, 0, sizeof(Protocol));
      memcpy(&type, buffer, message_type_size);
      //handle message
      switch (type)
      {
        case Protocol::DatabaseGet:
        {
          unsigned id = *reinterpret_cast<unsigned*>(buffer+message_type_size);
          unsigned row = *reinterpret_cast<unsigned*>(buffer+ message_type_size +sizeof(unsigned));
          //check if its in the database
          if (id <= db.size)
          {
            //get data from database
            char *data = 0;
            unsigned size;
            if (row == unsigned(-1))
            {
              size = db.Get(id, data);
            }
            else
            {
              size = db.Get(id, row, data);
            }
            //send back the data from the database
            //the message type is same as the one recieved
            *reinterpret_cast<unsigned*>(buffer+message_type_size) = id;
            *reinterpret_cast<unsigned*>(buffer+sizeof(unsigned)+message_type_size) = row;
            memcpy(buffer+sizeof(unsigned)*2+message_type_size, data, size);
            stack.Send(buffer, size + 2 * sizeof(unsigned)+message_type_size, &from, flags);
            delete [] data;
          }
          break;
        }
        case Protocol::DatabaseSet:
        {
          //set the data
          unsigned id = *reinterpret_cast<unsigned*>(buffer+message_type_size);
          unsigned row = *reinterpret_cast<unsigned*>(buffer+ message_type_size +sizeof(unsigned));
          db.Set(id, row, buffer + message_type_size + sizeof(unsigned)*2);
          db.flush();
          break;
        }
        case Protocol::DatabaseCreate:
        {
          //create an object in the database and return the id
          unsigned id = db.Create();
          buffer[0] = Protocol::DatabaseCreate;
          //the create message has a unsigned nonce after the message type, keep he nonce unmodified
          *reinterpret_cast<unsigned*>(buffer+sizeof(unsigned)+message_type_size) = id;
          stack.Send(buffer, 2*sizeof(unsigned)+message_type_size, &from, flags);
          break;
        }
        case Protocol::DatabaseFind:
        {
          //find
          unsigned row = *reinterpret_cast<unsigned*>(buffer+message_type_size);
          std::vector<unsigned> ids = db.Find(row, buffer+message_type_size+sizeof(unsigned));
          //create the response
          //redundant
          buffer[0] = Protocol::DatabaseFind;
          //redundant
          *reinterpret_cast<unsigned*>(buffer+message_type_size) = row;
          //the value thats being searched for stays in the buffer and is sent in response

          //add the id's (its in a loop because we can only send back 100 id's per message so if there
          //is more than 100 id's we send more than one message, each message has the same first couple bytes)
          while(1)
          {
            unsigned num = std::min(ids.size(),(long unsigned)(100));
            //num is between 0-100 so it fits in a char
            buffer[message_type_size+sizeof(unsigned) + db.rows[row]] = num;
            for (unsigned i = 0; i < num; ++i)
            {
              *reinterpret_cast<unsigned*>(buffer+message_type_size+sizeof(unsigned)+db.rows[row]+message_type_size+ sizeof(unsigned)*i) = ids[i];
            }
            stack.Send(buffer, message_type_size+sizeof(unsigned)+db.rows[row]+message_type_size+ sizeof(unsigned)*num, &from, flags);
            //if there was more than 100 need to send another one, remove the first 100
            //note that if the number of items found is divisable by 100 then we will send on message with no ids 
            if (num == 100)
            {
              ids.erase(ids.begin(), ids.begin()+num);
            }
            else
            {
              //end while loop
              break;
            }
          }
          break;
        }
        case Protocol::DatabaseDelete:
        {
            std::cout << "Delete messsage" << std::endl;

          unsigned id = *reinterpret_cast<unsigned*>(buffer+message_type_size);
          db.Delete(id);
          break;
        }
        default:
        {
          LOG("Database got message type " << type << " which is not being handled");
          break;
        }
      }
    }
    stack.Update();
  }
  return 420;
}