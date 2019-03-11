/*****************************************************************************/
/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  The entry point for the load balancer executable

*/
/*****************************************************************************/
#include <math.h>
#include <iostream>

#include "load_balancer.h"
#include "logger.h"
#include "database_protocol.h"
#include "load_balancer_protocol.h"
#include "query.h"
#include "utils.h"


LoadBalancer::LoadBalancer(Config &config)
  :config(config),
   sock(CreateSocket(IPPROTO_UDP)),
   stack(sock),
   query_id(0),
   protocol(static_cast<std::string>(config.properties["proto_dir"]))
{
  sockaddr_in local;
  CreateAddress(0,static_cast<int>(config.properties["port"]),&local);
  CreateAddress(static_cast<std::string>(config.properties["account_db_ip"]).c_str(),static_cast<int>(config.properties["account_db_port"]),&account_database);
  //add zones from config to the zones
  LOG("reading zones from config");
  unsigned zone_id = 0;
  while(1)
  {
    std::stringstream ss;
    ss << "zone";
    ss << zone_id++;
    ss << "_";
    if (config.properties[ss.str() + "name"].type == type_null)
    {
      LOG("No Zones");
      break;
    }
    sockaddr_in zone_addr;
    CreateAddress(static_cast<std::string>(config.properties[ss.str()+"ip"]).c_str(),static_cast<int>(config.properties[ss.str()+"port"]),&zone_addr);
    zones[zone_addr] = static_cast<std::string>(config.properties[ss.str() + "name"]);
    zone_array.push_back(zone_addr);
  }
  LOG("finished reading zones from config");
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
  network_signals.signals[protocol.LookUp("EncryptionKey")].Connect(std::bind(&LoadBalancer::EncryptionKey, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  network_signals.signals[protocol.LookUp("Relay")].Connect(std::bind(&LoadBalancer::Relay, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  network_signals.signals[protocol.LookUp("Query")].Connect(std::bind(&LoadBalancer::QueryResponse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  network_signals.signals[protocol.LookUp("Forward")].Connect(std::bind(&LoadBalancer::ForwardResponse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  
  account_manager.SetUp(this);
}
//addr and from are going to be the same
void LoadBalancer::EncryptionKey(char *buffer, unsigned n, sockaddr_in *addr)
{
  LOG("Encryptoin Key");
  char key[MAXPACKETSIZE];
  short length = ReadEncryptionMessage(buffer, n, key, encryptor);
  //if the message was malformed it returns -1 length then we ignore this meddage
  if (length == -1)
  {
    LOGW("Bad key");
    return;
  }
  LOG("Key of length " << length << " = " << ((unsigned int *)key)[0] << ", " << ((unsigned int *)key)[1] << ", " << ((unsigned int *)key)[2] << " ... " << ((unsigned int *)key)[length-1]);
  ((Encryption*)(stack.layers[2]))->blowfish[from] = BlowFish((unsigned int *)key, length);
  flags[from].SetBit(EncryptFlag);
  //send back a message saying that i got the key
  *((MessageType*)buffer) = protocol.LookUp("EncryptionKey");
  int ret = stack.Send(buffer, sizeof(MessageType), addr, flags[from]);
  if (ret < 0)
  {
    LOGW("ret = " << ret);
  }
}
void LoadBalancer::Relay(char *buffer, unsigned n, sockaddr_in *addr)
{
  (void)addr;
  LOG("Got relay message");
  buffer[n] = 0;
  LOG("sending back: " << buffer << std::endl);
  int send_err = stack.Send(buffer, n, &from, flags[from]);
  if (send_err < 0)
  {
    LOGW("Send error code = " << send_err);
  }
}

void LoadBalancer::ForwardResponse(char *buffer, unsigned n, sockaddr_in *addr)
{
  (void)addr;
  unsigned id;
  int dest;
  buffer = ParseForwardMessage(buffer, n, dest, id);
  if (dest == 0)
  {
    LOG("forwarding message to client with id " << id);
  }
  else
  {
    LOG("forwarding message to zone " << zones[zone_array[id]]);
  }
  if (dest == 0)
  {
    addr = &clients_by_id[id];
  }
  else
  {
    addr = &zone_array[id];
  }
  //send the message
  stack.Send(buffer, n, addr, flags[*addr]);
}
void LoadBalancer::QueryResponse(char *buffer, unsigned n, sockaddr_in *addr)
{
  (void)addr;
  unsigned id;
  unsigned size;
  char *data = 0;
  ParseQueryResponse(buffer, n, id, data, size);
  LOG("id = " <<id<<" size = " << size);
  //here we should signal based on the id
  if (query_callbacks.find(id) == query_callbacks.end())
  {
    LOGW("id " << id << " not in the callbacks");
  }
  else
  {
    query_callbacks[id](data, size);
    //remove the id to save memory
    //query_callbacks.erase(query_callbacks.find(id));
  }
}
void LoadBalancer::OnRecieve(std::shared_ptr<char> data, unsigned size, sockaddr_in addr)
{
  LOG("Recieved message of length " << size);
  MessageType type = 0;
  memcpy(&type, data.get(), sizeof(MessageType));
  LOG("Recieved message type " << protocol.LookUp(type) << ":" << type);
  network_signals.signals[type](data.get(), size, &addr);
}
void LoadBalancer::run()
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
          if (clients.find(from) != clients.end())
    {
      LOG("Recieved message from client");
    }
    else if (zones.find(from) != zones.end())
    {
      LOG("Redived message from " << zones[from]);
    }
      std::shared_ptr<char> data(new char[MAXPACKETSIZE], array_deleter<char>());
      memcpy(data.get(), buffer, n);
      dispatcher.Dispatch(std::bind(&LoadBalancer::OnRecieve, this, data, n, from));
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
  //setup the network stack
  Init();
  Config config;
  config.Init("resources/load_balancer.conf");
  LOG("config loaded" << std::endl);
  LoadBalancer lb(config);
  lb.run();
  return 420;
}