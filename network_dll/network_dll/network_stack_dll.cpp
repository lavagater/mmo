#include "network_stack_dll.h"

#include "channel.h"
#include "network_stack.h"
#include "blowfish.h"
#include "network_stack.h"
#include "bit_array.h"
#include "reliability.h"
#include "prioritization.h"
#include "encryption.h"
#include "protocol.h"
#include "logger.h"
#include "load_balancer_protocol.h"
#include "protocol.h"
#include "asymetric_encryption.h"

std::vector<session> dll_sessions;
std::vector<int> dll_unused_ids;

extern "C" {
  void Update(int id)
  {
    dll_sessions[id].stack->Update();
  }
  int CreateSession(char *ip, int port)
  {
    Init();
    SOCKET sock = CreateSocket(IPPROTO_UDP);
    sockaddr_in local;
    //client address id and port dont matter
    CreateAddress(0, 0, &local);
    Bind(sock, &local);
    SetNonBlocking(sock);
    //get the id for the network stack
    int id;
    if (dll_unused_ids.size() != 0)
    {
      id = dll_unused_ids.back();
      dll_unused_ids.pop_back();
	  //make a new session
	  dll_sessions[id] = session();
    }
    else
    {
      id = dll_sessions.size();
      dll_sessions.emplace_back();
    }
    dll_sessions[id].sock = sock;
	dll_sessions[id].proto.LoadProtocol();
    dll_sessions[id].stack = new NetworkStack(sock);
    Channel *channel = new Channel();
    Reliability *reliability = new Reliability();
    Encryption *encryption = new Encryption();
    dll_sessions[id].stack->AddLayer(channel);
    dll_sessions[id].stack->AddLayer(reliability);
    dll_sessions[id].stack->AddLayer(encryption);

    //create the addres for the database
    sockaddr_in server;
    CreateAddress(ip, port, &server);

    //set the session flags to reliable
    dll_sessions[id].flags.SetBit(ReliableFlag);
    dll_sessions[id].local = local;
    dll_sessions[id].server= server;

	if (dll_sessions[id].flags[EncryptFlag])
	{
		LOGW("Encryption flag already set!!!");
	}

    return id;
  }
  void DeleteSession(int id)
  {
	  LOG("Deleting session " << id);
    //check if the id is not valid
    if (id < 0 || id > dll_sessions.size())
    {
      return;
    }
    //close the socket
    Close(dll_sessions[id].sock, true);
    delete dll_sessions[id].stack;
    dll_unused_ids.push_back(id);
  }
  int SessionSend(int id, char *data, int length)
  {
	  LOG("Sending " << length << " bytes id " << id);
    length = dll_sessions[id].stack->Send(data, length, &dll_sessions[id].server, dll_sessions[id].flags);
	  LOG("Sent " << length << " bytes");
	if (length == 0)
	{
	  LOGW("Sending message of length zero!!");
	}
	return length;
  }
  int SessionRecieve(int id, char *data, int length)
  {
    length = dll_sessions[id].stack->Receive(data, length, &dll_sessions[id].server);
	if (length == 0)
	{
		LOGW("Recieving message of length zero!!");
	}
	if (length > 0)
	{
		//special case for encryption to set encryption bit
		MessageType type = 0;
		memcpy(&type, data, sizeof(MessageType));
		if (type == dll_sessions[id].proto.LookUp("EncryptionKey"))
		{
			LOG("Setting encryption for session " << id << " length = " << length);
			dll_sessions[id].flags.SetBit(EncryptFlag);
		}
	}
	return length;
  }
  void SendEncryptionRequest(int id)
  {
	LOGW("Sending Encryption request!!!");
    //generate a symmmetric key between 16 and 32 long
    int length = rand() % 16 + 16;
    unsigned key[32];
    for (int i = 0; i < length; ++i)
    {
      key[i] = rand();
    }
	LOG("Key of length " << length << " = " << key[0] << ", " << key[1] << ", " << key[2] << " ... " << key[length-1]);
    //set the encryption
    ((Encryption*)dll_sessions[id].stack->layers[2])->blowfish[dll_sessions[id].server] = BlowFish(key, length);

    //call some function to encrypt this message with an asymetric encryption
	char buffer[MAXPACKETSIZE];
	AsymetricEncryption as;
	length = CreateEncryptionMessage(dll_sessions[id].proto, buffer, (char*)key, length, as);
	dll_sessions[id].stack->Send(buffer, length, &dll_sessions[id].server, dll_sessions[id].flags);
	//in the recieve call we will check for the response and set the encryption bit
  }
}