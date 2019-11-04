#ifndef ZONE_H
#define ZONE_H

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
#include "load_balancer_protocol.h"
#include "logger.h"
#include "dispatcher.h"
#include "network_signals.h"
#include "game_object.h"
#include "utils.h"
#include <unordered_set>
#include <map>
#include <memory>

class Zone
{
public:
  Zone(Config &config);
  void run();
  void OnRecieve(std::shared_ptr<char> data, unsigned size, sockaddr_in addr, BitArray<HEADERSIZE> sent_flags);
  GameObject *CreateGameObject();
  void RemoveGameObject(GameObject *obj);
  //network messages


  void Login(char *buffer, unsigned n, sockaddr_in *addr);
  void PlayerJoined(unsigned id, sockaddr_in client_addr, sockaddr_in lb_addr, char *player_data, unsigned size);
  void QueryResponse(char *buffer, unsigned n, sockaddr_in *addr);
  void GameUpdate(double dt);
  std::unordered_map<unsigned, GameObject*> players;

  //collision things
  std::vector<std::unordered_map<std::pair<int, int>, std::unordered_set<GameObject*>, pair_hash>> colliders;
  std::vector<std::vector<unsigned>> collision_groups;

  std::unordered_set<GameObject*> all_objects;
  std::unordered_map<unsigned, GameObject*> object_by_id;
  unsigned next_gameobject_id=0;
  std::vector<unsigned> unused_gameobject_ids;

  /********************************************/

  Config &config;
  SOCKET sock;
  Dispatcher dispatcher;
  NetworkStack stack;
  std::unordered_map<sockaddr_in, BitArray<HEADERSIZE>, SockAddrHash> flags;
  std::unordered_map<unsigned, std::function<void(char *, unsigned)> > query_callbacks;
  unsigned query_id;
  char buffer[MAXPACKETSIZE];
  sockaddr_in from;
  sockaddr_in account_database;
  sockaddr_in players_database;
  sockaddr_in spells_database;
  //set of the known addresses from start, i.e. databases
  std::unordered_set<sockaddr_in, SockAddrHash> known_addr;
  //set of load balancers
  std::unordered_set<sockaddr_in, SockAddrHash> load_balancers;
  AsymetricEncryption encryptor;
  ProtocolLoader protocol;
  NetworkSignals network_signals;
  Signals<double> update_signal;
  Signals<GameObject*> player_joined_signal;
};

#endif