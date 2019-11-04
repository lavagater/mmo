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
#include "collider_component.h"
#include "gate_component.h"
#include "movement_component.h"
#include "interactive_component.h"
#include "entity_component.h"

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
  CreateAddress(static_cast<std::string>(config.properties["spells_db_ip"]).c_str(),static_cast<int>(config.properties["spells_db_port"]),&spells_database);
  flags[spells_database].SetBit(ReliableFlag);
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

  //setup collision groups
  std::fstream collision_grid("resources/collision_groups.txt");
  while(!collision_grid.eof())
  {
    colliders.emplace_back();
    std::string line;
    std::getline(collision_grid, line);
    std::stringstream ss(line);
    std::vector<unsigned> temp_group;
    int group = 0;
    while(!ss.eof())
    {
      int is_checked;
      ss >> is_checked;
      if (is_checked)
      {
        temp_group.push_back(group);
      }
      group += 1;
    }
    collision_groups.push_back(temp_group);
  }

  //load level file, find a way to decouple this from zone
  //maybe have a base class of a component loader and have a vector
  //of component loaders like system components, shooter components
  //and each of them know how to load specific components and an virtual function
  //is called with the component name
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
    else if (comp_name == "ColliderComponent")
    {
      ADDCOMP(obj, ColliderComponent)->Load(level);
    }
    else if (comp_name == "GateComponent")
    {
      ADDCOMP(obj, GateComponent)->Load(level);
    }
    else
    {
      //if the name is not a component then its the next gameobject
      obj = CreateGameObject();
      obj->name = comp_name;
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
  object_by_id[obj->id] = obj;
  all_objects.insert(obj);
  return obj;
}

void Zone::RemoveGameObject(GameObject *obj)
{
  //make sure the game object does not have any messages in the dispatcher
  dispatcher.RemoveMessageById(obj);
  //remove object from map
  auto map_it = object_by_id.find(obj->id);
  if (map_it != object_by_id.end())
  {
    object_by_id.erase(map_it);
  }
  //remove object from set
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
    //TODO: set any important player info in db
  }
  LOG("New player id = " << id);
  //this should be done using a player archtype
  players[id] = CreateGameObject();
  ADDCOMP(players[id], TransformComponent);
  ADDCOMP(players[id], MovementComponent);//default speed 1 is good for now
  ADDCOMP(players[id], InteractiveComponent)->interaction_range = 1;
  ADDCOMP(players[id], ColliderComponent);
  GETCOMP(players[id], ColliderComponent)->shape = new Circle();
  ((Circle*)GETCOMP(players[id], ColliderComponent)->shape)->radius = 0.5;
  GETCOMP(players[id], ColliderComponent)->shape->object = players[id];
  ADDCOMP(players[id], PlayerControllerComponent);
  GETCOMP(players[id], PlayerControllerComponent)->lb_addr = lb_addr;
  GETCOMP(players[id], PlayerControllerComponent)->client_addr = client_addr;
  GETCOMP(players[id], PlayerControllerComponent)->id = id;
  //add entity after player controller so it can do db query on init
  ADDCOMP(players[id], EntityComponent);
  players[id]->name = "player" + std::to_string(id);
  if (size == sizeof(double)*2)
  {
    GETCOMP(players[id], TransformComponent)->position = Eigen::Vector2d(*reinterpret_cast<double*>(player_data), *reinterpret_cast<double*>(player_data+sizeof(double)));
    GETCOMP(players[id], MovementComponent)->destination = Eigen::Vector2d(*reinterpret_cast<double*>(player_data), *reinterpret_cast<double*>(player_data+sizeof(double)));
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
  std::cout << "dt = " << dt << "          \r" << std::flush;
  //send the update signal
  update_signal(dt);
}
void Zone::OnRecieve(std::shared_ptr<char> data, unsigned size, sockaddr_in addr, BitArray<HEADERSIZE> sent_flags)
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
  network_signals.signals[type](data.get(), size, &addr, sent_flags);
}
void Zone::run()
{
  //main loop
  while(true)
  {
    dispatcher.Update();
    //check for messages
    BitArray<HEADERSIZE> sent_flags;
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from, sent_flags);
    flags[from].SetBit(ReliableFlag);
    if (n >= (int)sizeof(MessageType))
    {
      std::shared_ptr<char> data(new char[MAXPACKETSIZE], array_deleter<char>());
      memcpy(data.get(), buffer, n);
      dispatcher.Dispatch(std::bind(&Zone::OnRecieve, this, data, n, from, sent_flags));
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