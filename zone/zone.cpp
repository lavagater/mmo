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
  network_signals.signals[protocol.LookUp("ZoneTest")].Connect(std::bind(&Zone::OnZoneTest, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
void Zone::OnZoneTest(char *buffer, unsigned n, sockaddr_in *addr)
{
  if (n < sizeof(MessageType) + sizeof(unsigned) + sizeof(double) + sizeof(double))
  {
    LOG("Player update invalid size");
    return;
  }
  (void)addr;
  (void)n;
  char *saved = buffer;
  //set the new destination
  buffer += sizeof(MessageType);
  unsigned id = *reinterpret_cast<unsigned*>(buffer);
  LOG("Got message from player " << id);
  buffer += sizeof(unsigned);
  players[id].x_dest = *reinterpret_cast<double*>(buffer);
  buffer += sizeof(double);
  players[id].y_dest = *reinterpret_cast<double*>(buffer);
  buffer += sizeof(double);
  //tell each player about the new destination
  *reinterpret_cast<double*>(buffer) = players[id].x_pos;
  buffer += sizeof(double);
  *reinterpret_cast<double*>(buffer) = players[id].y_pos;
  buffer += sizeof(double);
  unsigned size = buffer - saved;
  for (auto it = players.begin(); it != players.end(); ++it)
  {
    LOG("Sending to player " << it->first);
    char msg[500];
    unsigned message_size = size;
    //TODO: maybe forward mesasage can send to multiple places???
    CreateForwardMessage(protocol, saved, message_size, 0, it->first, msg);
    LOG("new message size = " << message_size << " sending to " << &it->second.lb_addr);
    stack.Send(msg, message_size, &it->second.lb_addr, flags[it->second.lb_addr]);
    LOG("sent");
  }
}
void Zone::Login(char *buffer, unsigned n, sockaddr_in *addr)
{
  (void)n;
  //got a new player
  buffer += sizeof(MessageType);
  unsigned id = *reinterpret_cast<unsigned*>(buffer);
  LOG("New player id = " << id);
  players[id].lb_addr = *addr;
  players[id].id = id;
  //tell the player about ever other player
}
void Zone::GameUpdate(double dt)
{
  //move like the players or something
  for (auto it = players.begin(); it != players.end(); ++it)
  {
    double x_delta = it->second.x_dest - it->second.x_pos;
    double y_delta = it->second.y_dest - it->second.y_pos;
    if (x_delta == 0 && y_delta == 0)
    {
      break;
    }
    double dist = std::sqrt(x_delta * x_delta + y_delta * y_delta);
    it->second.x_pos += x_delta / dist * dt;
    it->second.y_pos += y_delta / dist * dt;
  }
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
    GameUpdate(stack.timer.GetPrevTime());
  }
}

//void Zone::run()
//{
//  //main loop
//  while(true)
//  {
//    //check for messages
//    int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
//    //if from is a new address at it to the load balancers
//    if (known_addr.find(from) == known_addr.end())
//    {
//      load_balancers.insert(from);
//    }
//    flags[from].SetBit(ReliableFlag);
//    if (n >= (int)sizeof(MessageType))
//    {
//      LOG("Recieved message of length " << n);
//      MessageType type = 0;
//      memcpy(&type, buffer, sizeof(MessageType));
//      LOG("Recieved message type " << protocol.LookUp(type) << ":" << type);
//      network_signals.signals[type](buffer, n, &from);
//    }
//    else if (n != EBLOCK && n != 0)
//    {
//      LOGW("recv Error code " << n);
//    }
//    stack.Update();
//    GameUpdate(stack.timer.GetPrevTime());
//  }
//}

int main()
{
  Init();
  Config config;
  config.Init("resources/zone.conf");
  Zone zone(config);
  zone.run();
  return 69;
}