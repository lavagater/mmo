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
  CreateAddress(static_cast<std::string>(config.properties["players_db_ip"]).c_str(),static_cast<int>(config.properties["players_db_port"]),&players_database);
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
  network_signals.signals[protocol.LookUp("Forward")].Connect(std::bind(&LoadBalancer::ForwardResponse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  stack.disconnected.Connect(std::bind(&LoadBalancer::OnClientDisconnect, this, std::placeholders::_1));

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

void LoadBalancer::ForwardResponse(char *buffer, unsigned n, sockaddr_in *addr, BitArray<HEADERSIZE> sent_flags)
{
  (void)addr;
  unsigned id;
  int dest;
  LOG("Parse forward message");
  buffer = ParseForwardMessage(buffer, n, dest, id);
  if (buffer == 0)
  {
    LOGW("Forawrd message malformed");
    return;
  }
  LOG("Parsed forward message");
  MessageType type = 0;
  memcpy(&type, buffer, sizeof(MessageType));
  if (dest == 0)
  {
    LOG("forwarding message to client with id " << id << " message type = " << protocol.LookUp(type));
  }
  else
  {
    LOG("forwarding message to zone " << GetZone(&zone_array[id]) << " message type = " << protocol.LookUp(type));
  }
  if (dest == 0)
  {
    addr = &clients_by_id[id];
  }
  else
  {
    //when forwarding messages to the zone servers we add where the message came from to the end
    *reinterpret_cast<sockaddr_in*>(buffer+n) = *addr;
    n += sizeof(sockaddr_in);
    addr = &zone_array[id];
  }
  //send the message
  BitArray<HEADERSIZE> new_flags = flags[*addr];
  //keep the reliability of the message being forwarded
  new_flags.SetBit(ReliableFlag, sent_flags[ReliableFlag]);
  stack.Send(buffer, n, addr, new_flags);
  LOG("sent");
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
    query_callbacks.erase(query_callbacks.find(id));
  }
}
void LoadBalancer::OnRecieve(std::shared_ptr<char> data, unsigned size, sockaddr_in addr, BitArray<HEADERSIZE> sent_flags)
{
  LOG("Recieved message of length " << size);
  MessageType type = 0;
  memcpy(&type, data.get(), sizeof(MessageType));
  LOG("Recieved message type " << protocol.LookUp(type) << ":" << type);
  network_signals.signals[type](data.get(), size, &addr, sent_flags);
}
void LoadBalancer::OnClientDisconnect(const sockaddr_in *addr)
{
  //use dispatcher so that all the Client disconnect callbacks can still access clients
  dispatcher.Dispatch(std::bind(&LoadBalancer::RemoveClient, this, *addr));
}
void LoadBalancer::RemoveClient(sockaddr_in addr)
{
  if (clients.find(addr) != clients.end())
  {
    unsigned id = clients[addr];
    LOG("Removing client " << id);
    clients.erase(clients.find(addr));
    clients_by_id.erase(clients_by_id.find(id));
  }
}
void LoadBalancer::run()
{
  //main loop
  while(true)
  {
    dispatcher.Update();
    //check for messages
    BitArray<HEADERSIZE> sent_flags;
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from, sent_flags);
    if (n >= (int)sizeof(MessageType))
    {
      flags[from].SetBit(ReliableFlag);
      if (clients.find(from) != clients.end())
      {
        LOG("Recieved message from client " << clients[from]);
      }
      else if (zones.find(from) != zones.end())
      {
        LOG("Redived message from " << GetZone(&from));
      }
      else
      {
        LOG("Recieved message from unknown");
      }
      
      std::shared_ptr<char> data(new char[MAXPACKETSIZE], array_deleter<char>());
      memcpy(data.get(), buffer, n);
      dispatcher.Dispatch(std::bind(&LoadBalancer::OnRecieve, this, data, n, from, sent_flags));
    }
    else if (n != EBLOCK && n != 0)
    {
      LOGW("recv Error code " << n);
    }
    stack.Update();
  }
}

sockaddr_in LoadBalancer::GetZone(std::string zone_name)
{
  for (auto it = zones.begin(); it != zones.end(); ++it)
  {
    if (it->second == zone_name)
    {
      return it->first;
    }
  }
  LOG("Zone " << zone_name << " not found, sending to " << zones.begin()->second);
  return zones.begin()->first;
}
std::string LoadBalancer::GetZone(sockaddr_in *addr)
{
  return zones[*addr];
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