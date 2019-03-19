/*****************************************************************************/
/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  The entry point for the zone executable

*/
/*****************************************************************************/
#include <math.h>
#include <iostream>

#include "logger.h"
#include "database_protocol.h"
#include "query.h"
#include "zone.h"
#include "utils.h"
#include "player_controller_component.h"
#include "transform_component.h"

Zone::Zone(Config &config)
  :config(config),
   sock(CreateSocket(IPPROTO_UDP)),
   stack(sock),
   protocol(static_cast<std::string>(config.properties["proto_dir"]))
{
  LOG("config loaded" << std::endl);
  sockaddr_in local;
  CreateAddress(0,static_cast<int>(config.properties["port"]),&local);
  CreateAddress(static_cast<std::string>(config.properties["account_db_ip"]).c_str(),static_cast<int>(config.properties["account_db_port"]),&account_database);
  flags[account_database].SetBit(ReliableFlag);
  Bind(sock, &local);
  SetNonBlocking(sock);
  Channel *channel = new Channel();
  Reliability *reliability = new Reliability();
  Encryption *encryption = new Encryption();
  stack.AddLayer(channel);
  stack.AddLayer(reliability);
  stack.AddLayer(encryption);
  protocol.LoadProtocol();
  network_signals.signals[protocol.LookUp("Login")].Connect(std::bind(&Zone::Login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  //30 updates a second
  dispatcher.Dispatch(std::bind(&Zone::GameUpdate, this, std::placeholders::_1), 1.0/30.0);
}

GameObject *Zone::CreateGameObject()
{
  GameObject *obj = new GameObject();
  obj->zone = this;
  all_objects.insert(obj);
  return obj;
}

void Zone::RemoveGameObject(GameObject *obj)
{
  auto it = all_objects.find(obj);
  if (it != all_objects.end())
  {
    delete obj;
    all_objects.erase(it);
  }
}

void Zone::Login(char *buffer, unsigned n, sockaddr_in *addr)
{
  (void)n;
  //got a new player
  buffer += sizeof(MessageType);
  unsigned id = *reinterpret_cast<unsigned*>(buffer);
  LOG("New player id = " << id);
  //this should be done using a player archtype
  players[id] = CreateGameObject();
  ADDCOMP(players[id], PlayerControllerComponent);
  ADDCOMP(players[id], TransformComponent);
  GETCOMP(players[id], PlayerControllerComponent)->lb_addr = *addr;
  GETCOMP(players[id], PlayerControllerComponent)->id = id;
  //signal that the new player joined
  player_joined_signal(players[id]);
}
void Zone::GameUpdate(double dt)
{
  //call the update again
  dispatcher.Dispatch(std::bind(&Zone::GameUpdate, this, std::placeholders::_1), 1.0/30.0);
  //send the update signal
  update_signal(dt);
}
void Zone::OnRecieve(std::shared_ptr<char> data, unsigned size, sockaddr_in addr)
{
  //if from is a new address at it to the load balancers
  if (known_addr.find(from) == known_addr.end())
  {
    load_balancers.insert(from);
  }
  LOG("Recieved message of length " << size);
  MessageType type = 0;
  memcpy(&type, data.get(), sizeof(MessageType));
  LOG("Recieved message type " << protocol.LookUp(type) << ":" << type);
  network_signals.signals[type](data.get(), size, &addr);
}
void Zone::run()
{
  //main loop
  while(true)
  {
    dispatcher.Update();
    //check for messages
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
    flags[from].SetBit(ReliableFlag);
    if (n >= (int)sizeof(MessageType))
    {
      std::shared_ptr<char> data(new char[MAXPACKETSIZE], array_deleter<char>());
      memcpy(data.get(), buffer, n);
      dispatcher.Dispatch(std::bind(&Zone::OnRecieve, this, data, n, from));
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
  Config config;
  config.Init("resources/zone.conf");
  Zone zone(config);
  zone.run();
  return 69;
}