/*****************************************************************************/
/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  The entry point for the database executable. The database trusts its users (since the users have full control of the data) so it
  does minimal error checking with packets

*/
/*****************************************************************************/

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
  //load in config file
  Config config;
  config.Init("database.conf");
  //load the database
  std::string file = static_cast<std::string>(config.properties["table"])+".tbl";
  Database db(file.c_str());
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
  //main loop
  while(true)
  {
    //check for messages
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
    //make sure the message is big enough 1 byte for message type 2 unsigned's
    if (n > int(1 + sizeof(unsigned)*2))
    {
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
      else
      {
        //create an object in the database and return the id
        unsigned id = db.Create();
        *reinterpret_cast<unsigned*>(buffer) = id;
        stack.Send(buffer, sizeof(unsigned), &from, flags);
      }
    }
    stack.Update();
  }
  return 420;
}