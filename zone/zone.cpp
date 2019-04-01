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
#include "terrain_component.h"
#include "transform_component.h"
#include "static_object_component.h"

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
  flags[players_database].SetBit(ReliableFlag);
  CreateAddress(static_cast<std::string>(config.properties["players_db_ip"]).c_str(),static_cast<int>(config.properties["players_db_port"]),&players_database);
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
  network_signals.signals[protocol.LookUp("Query")].Connect(std::bind(&Zone::QueryResponse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  network_signals.signals[protocol.LookUp("Login")].Connect(std::bind(&Zone::Login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  //load level file
  std::fstream level("resources/level.txt");
  GameObject *obj;
  while(!level.eof())
  {
    std::string comp_name;
    level >> comp_name;
    if (comp_name == "")
      break;
    if (comp_name == "TransformComponent")
    {
      ADDCOMP(obj, TransformComponent)->Load(level);
    }
    else if (comp_name == "TerrainComponent")
    {
       ADDCOMP(obj, TerrainComponent)->Load(level);
    }
    else if (comp_name == "StaticObjectComponent")
    {
      ADDCOMP(obj, StaticObjectComponent)->Load(level);
    }
    else
    {
      //if the name is not a component then its the next gameobject
      obj = CreateGameObject();
      //read the object type
      level >> obj->type;
    }
    
  }

  //30 updates a second
  dispatcher.Dispatch(std::bind(&Zone::GameUpdate, this, std::placeholders::_1), 1.0/30.0);
}

GameObject *Zone::CreateGameObject()
{
  GameObject *obj = new GameObject();
  if (unused_gameobject_ids.size() > 0)
  {
    obj->id = unused_gameobject_ids.back();
    unused_gameobject_ids.pop_back();
  }
  else
  {
    obj->id = next_gameobject_id++; 
  }
  obj->zone = this;
  all_objects.insert(obj);
  return obj;
}

void Zone::RemoveGameObject(GameObject *obj)
{
  auto it = all_objects.find(obj);
  if (it != all_objects.end())
  {
    unused_gameobject_ids.push_back(obj->id);
    delete obj;
    all_objects.erase(it);
  }
}

void Zone::PlayerJoined(unsigned id, sockaddr_in client_addr, sockaddr_in lb_addr, char *player_data, unsigned size)
{
  if (players.find(id) != players.end())
  {
    GameObject *player = players[id];
    players.erase(players.find(id));
    RemoveGameObject(player);
  }
  LOG("New player id = " << id);
  //this should be done using a player archtype
  players[id] = CreateGameObject();
  ADDCOMP(players[id], PlayerControllerComponent);
  ADDCOMP(players[id], TransformComponent);
  GETCOMP(players[id], PlayerControllerComponent)->lb_addr = lb_addr;
  GETCOMP(players[id], PlayerControllerComponent)->client_addr = client_addr;
  GETCOMP(players[id], PlayerControllerComponent)->id = id;
  if (size == sizeof(double)*2)
  {
    GETCOMP(players[id], TransformComponent)->position = Eigen::Vector2d(*reinterpret_cast<double*>(player_data), *reinterpret_cast<double*>(player_data+sizeof(double)));
  }
  else
  {
    LOG("size of query return is " << size);
  }
  //signal that the new player joined
  player_joined_signal(players[id]);
}

void Zone::Login(char *buffer, unsigned n, sockaddr_in *addr)
{
  (void)n;
  //got a new player
  buffer += sizeof(MessageType);
  unsigned id = *reinterpret_cast<unsigned*>(buffer);
  buffer += sizeof(id);
  sockaddr_in client_addr = *reinterpret_cast<sockaddr_in*>(buffer);
  
  //get the players info from the database
  query_id += 1;
  query_callbacks[query_id] = std::bind(&Zone::PlayerJoined, this, id, client_addr, *addr, std::placeholders::_1, std::placeholders::_2);
  int len = CreateQueryMessage(protocol, query_id, &buffer[0], STRINGIZE(
    main(unsigned id)
    {
      print("Gettung player info\n");
      //get the id
      vector res = find(0, id);
      print(res, "\n");
      if (Size(res) == 0)
      {
        print("Player does not exist, this should not happen\n");
        //for now we will ignore the rest
        return vector(0,0);
      }
      //if there was a player get the x and y positions
      double x_pos = get(res[0], 2);
      print("x pos = ", x_pos, "\n");
      double y_pos = get(res[0], 3);
      print("y pos = ", y_pos, "\n");
      return vector(x_pos, y_pos);
    }
  ), id);
  stack.Send(&buffer[0], len, &players_database, flags[players_database]);
}
void Zone::QueryResponse(char *buffer, unsigned n, sockaddr_in *addr)
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
    LOG("id " << id << " not in the callbacks");
  }
  else
  {
    query_callbacks[id](data, size);
    //remove the id to save memory
    query_callbacks.erase(query_callbacks.find(id));
  }
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
  if (type == 2)
  {
    LOG(ToHexString(data.get(), size));
  }
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