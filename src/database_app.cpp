/*****************************************************************************/
/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  The entry point for the database executable. The database trusts its users (since the users have full control of the data) so it
  does minimal error checking with packets

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
    //make sure the message is big enough 1 byte for message type 2 unsigned's
    if (n > int(1 + sizeof(unsigned)*2))
    {
      std::cout << "got message" << std::endl;
      //handle message
      if (buffer[0] == 0)
      {
        unsigned id = *reinterpret_cast<unsigned*>(buffer+1);
        unsigned row = *reinterpret_cast<unsigned*>(buffer+ 1 +sizeof(unsigned));
        //check if its in the database
        if (db.size <= id)
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
          *reinterpret_cast<unsigned*>(buffer) = id;
          *reinterpret_cast<unsigned*>(buffer+sizeof(unsigned)) = row;
          memcpy(buffer+sizeof(unsigned)*2, data, size);
          stack.Send(buffer, size + 2 * sizeof(unsigned), &from, flags);
          delete [] data;
        }
      }
      else if (buffer[0] == 1)
      {
        //set the data
        unsigned id = *reinterpret_cast<unsigned*>(buffer+1);
        unsigned row = *reinterpret_cast<unsigned*>(buffer+ 1 +sizeof(unsigned));
        db.Set(id, row, buffer+ sizeof(unsigned)*2);
      }
      else if (buffer[0] == 2)
      {
        //create an object in the database and return the id
        unsigned id = db.Create();
        *reinterpret_cast<unsigned*>(buffer) = id;
        stack.Send(buffer, sizeof(unsigned), &from, flags);
      }
      else if (buffer[0] == 3)
      {
        std::cout << "got find request" << std::endl;
        //find
        unsigned row = *reinterpret_cast<unsigned*>(buffer+1);
        std::vector<unsigned> ids = db.Find(row, buffer+1+sizeof(unsigned));
        *reinterpret_cast<unsigned*>(buffer) = row;
        *(buffer+sizeof(unsigned)) = sizeof(int);
        //note that where the value to search for is in the same spot in the buffer
        //add the id's
        while(1)
        {
          unsigned num = std::max(ids.size(),(long unsigned)(100));
          for (unsigned i = 0; i < num; ++i)
          {
            *reinterpret_cast<unsigned*>(buffer+sizeof(unsigned)+1+sizeof(int)+ sizeof(unsigned)*i) = ids[i];
          }
          stack.Send(buffer, sizeof(unsigned)+1+sizeof(int)+ sizeof(unsigned)*num, &from, flags);
          //if there was more than 100 need to send another one, remove the first 100
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
      }
    }
    stack.Update();
  }
  return 420;
}