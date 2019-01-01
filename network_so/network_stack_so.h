#ifndef NETWORK_SO_H
#define NETWORK_SO_H

#include <vector>
#include "network_stack.h"

//contains the things that the client wants to know about
class session
{
public:
  //the flags for sending messages
  BitArray<HEADERSIZE> flags;
  //buffer for receiving messages
  char buffer[MAXPACKETSIZE];
  sockaddr_in local;
  sockaddr_in server;
  SOCKET sock;
  NetworkStack *stack;
};

extern std::vector<session> dll_sessions;
extern std::vector<int> dll_unused_ids;


extern "C" {
  void Update(int id);
  int CreateSession(char *ip, int port);
  void DeleteSession(int id);
  int SessionSend(int id, char *data, int length);
  int SessionRecieve(int id, char *data, int length);
  void EnableEncryption(int id);
}


#endif //header guard