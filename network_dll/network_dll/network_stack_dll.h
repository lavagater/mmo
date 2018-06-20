#ifndef NETWORK_DLL_H
#define NETWORK_DLL_H

#ifdef NETWORK_DLL_EXPORT
#define NETWORK_DLL_API __declspec(dllexport) 
#else
#define NETWORK_DLL_API __declspec(dllimport) 
#endif

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
  NETWORK_DLL_API void Update(int id);
  NETWORK_DLL_API int CreateSession(char *ip, int port);
  NETWORK_DLL_API void DeleteSession(int id);
  NETWORK_DLL_API int SessionSend(int id, char *data, int length);
  NETWORK_DLL_API int SessionRecieve(int id, char *data, int length);
  NETWORK_DLL_API void EnableEncryption(int id);
}


#endif //header guard