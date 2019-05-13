#ifndef NETWORK_DLL_H
#define NETWORK_DLL_H

#ifdef NETWORK_DLL_EXPORT
#define NETWORK_DLL_API __declspec(dllexport) 
#else
#define NETWORK_DLL_API __declspec(dllimport) 
#endif

#include <vector>
#include "network_stack.h"
#include "protocol.h"
#include "logger.h"
#include "asymetric_encryption.h"
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
#include "protocol.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <thread_safe_queue.h>

//contains the things that the client wants to know about
class session
{
public:
  session(char *ip, int port);
  ~session()
  {
    Close(sock, true);
    running = false;
    recieve_thread->join();
    delete recieve_thread;
    delete stack;
  }
  void ThreadLoop()
  {
    sockaddr_in from;
    while (running)
    {
      //check for sending
      if (can_send)
      {
        LOG("Can send");
        while (!send_queue.empty())
        {
          std::vector<char> packet = send_queue.pop();
          LOG("sending " << packet.size() << " bytes");
          int n = stack->Send(packet.data(), packet.size(), &server, flags);
          LOG(n << "bytes error = " << GetError());
        }
        //might fail due to race condition but dont really care
        can_send = false;
      }
      //check for messages
      unsigned total = 0;
      while (1)
      {
        int n = stack->Receive(buffer, MAXPACKETSIZE, &from);
        if (n <= 0)
        {
          break;
        }
        total += n;
        LOG("Recieved message length " << n);
        if (!(from == server))
        {
          LOGW("recieved message from somewhere wierd!!");
        }
        //special case for encryption to set encryption bit
        MessageType type = 0;
        memcpy(&type, buffer, sizeof(MessageType));
        if (type == proto.LookUp("EncryptionKey"))
        {
          LOG("Setting encryption");
          flags.SetBit(EncryptFlag);
        }
        std::vector<char> packet(&buffer[0], &buffer[n]);
        recieved_queue.push(packet);
      }
	  if (total > 0)
	  {
		  LOG("Read " << total << "bytes");
	  }
      //update the stack
      stack->Update();
    }
  }
  //this function is called from the game thread, but Im pretty sure it wont harm the recieve thread
  //even if there is a pssible race, its the client who cares
  void SendEncryption()
  {
    //generate a symmmetric key between 16 and 32 long
    int length = rand() % 16 + 16;
    unsigned key[32];
    for (int i = 0; i < length; ++i)
    {
      key[i] = rand();
    }
    LOG("Key of length " << length << " = " << key[0] << ", " << key[1] << ", " << key[2] << " ... " << key[length - 1]);
    //set the encryption
    ((Encryption*)stack->layers[2])->blowfish[server] = BlowFish(key, length);

    //call some function to encrypt this message with an asymetric encryption
    char buf[MAXPACKETSIZE];
    AsymetricEncryption as;
    length = CreateEncryptionMessage(proto, buf, (char*)key, length, as);
    std::vector<char> packet(&buf[0], &buf[length]);
    //add to send queue
    send_queue.push(packet);
  }
  double GetPing()
  {
	  return stack->connections[server].ping;
  }
  ThreadSafeQueue<std::vector<char>> send_queue;
  ThreadSafeQueue<std::vector<char>> recieved_queue;
  bool can_send;
  bool running;
private:
  //the flags for sending messages
  BitArray<HEADERSIZE> flags;
  //buffer for receiving messages
  char buffer[MAXPACKETSIZE];
  sockaddr_in local;
  sockaddr_in server;
  SOCKET sock;
  ProtocolLoader proto;
  NetworkStack *stack;
  std::thread *recieve_thread;
};

extern std::vector<session*> dll_sessions;
extern std::vector<int> dll_unused_ids;


extern "C" {
  NETWORK_DLL_API void Update(int id);
  NETWORK_DLL_API double GetPing(int id);
  NETWORK_DLL_API int CreateSession(char *ip, int port);
  NETWORK_DLL_API void DeleteSession(int id);
  NETWORK_DLL_API int SessionSend(int id, char *data, int length);
  NETWORK_DLL_API int SessionRecieve(int id, char *data, int length);
  NETWORK_DLL_API void SendEncryptionRequest(int id);
}


#endif //header guard