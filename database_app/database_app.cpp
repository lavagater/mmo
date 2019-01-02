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

#include "database_protocol.h"
#include "database_app.h"

DatabaseApp::DatabaseApp(Config &conf, std::string dbfile)
: config(conf),
  db(dbfile.c_str()),
  query(db),
  sock(CreateSocket(IPPROTO_UDP)),
  stack(sock)
{
  std::cout << "start" << std::endl;
  //setup the network stack
  sockaddr_in local;
  CreateAddress(0,static_cast<int>(config.properties["port"]),&local);
  Bind(sock, &local);
  SetNonBlocking(sock);

  Channel *channel = new Channel();
  Reliability *reliability = new Reliability();
  Encryption *encryption = new Encryption();
  stack.AddLayer(channel);
  stack.AddLayer(reliability);
  stack.AddLayer(encryption);
  protocol.LoadProtocol();
  network_signals.signals[protocol.LookUp("Query")].Connect(std::bind(&DatabaseApp::QueryCall, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
void DatabaseApp::QueryCall(char *buffer, int n, sockaddr_in *addr)
{
  LOG("Query Call");
  (void)addr;
  std::string script;
  unsigned id;
  std::vector<Value> parameters;
  ParseQueryMessage(buffer, n, id, script, parameters);
  LOG("id = " << id);
  LOG("Script = " << script);
  //execute the script
  Value returnValue;
  query.Compile(script, parameters, returnValue);
  //send back the return value
  n = CreateQueryResponse(protocol, id, buffer, returnValue);
  stack.Send(buffer, n, addr, flags[*addr]);
}
void DatabaseApp::run()
{
  LOG("run");
  while(true)
  {
    //check for messages
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
    flags[from].SetBit(ReliableFlag);
    if (n >= (int)sizeof(MessageType))
    {
      LOG("Recieved message of length " << n);
      MessageType type = 0;
      memcpy(&type, buffer, sizeof(MessageType));
      LOG("Recieved message type " << protocol.LookUp(type) << ":" << type);
      network_signals.signals[type](buffer, n, &from);
    }
    else if (n != EBLOCK && n != 0)
    {
      LOGW("recv Error code " << n);
    }
    stack.Update();
  }
}

int main()
{
  Init();
  LOG("START");
  //load in config file
  Config config;
  config.Init("resources/database.conf");
  std::string file = std::string("resources/")+static_cast<std::string>(config.properties["table"])+".tbl";
  LOG("Setup");
  DatabaseApp db_app(config, file);
  db_app.run();
  return 420;
}