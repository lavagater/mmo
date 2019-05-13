#include "network_stack_dll.h"

std::vector<session*> dll_sessions;
std::vector<int> dll_unused_ids;

session::session(char*ip,int port) :proto("protocol/")
{
	LOG("Creating session");
	Init();
	sock = CreateSocket(IPPROTO_UDP);
	//client address id and port dont matter
	CreateAddress(0, 0, &local);
	Bind(sock, &local);
	SetNonBlocking(sock);

	proto.LoadProtocol();
	stack = new NetworkStack(sock);
	Channel *channel = new Channel();
	Reliability *reliability = new Reliability();
	Encryption *encryption = new Encryption();
	stack->AddLayer(channel);
	stack->AddLayer(reliability);
	stack->AddLayer(encryption);

	//create the addres for the database
	CreateAddress(ip, port, &server);

	//set the session flags to reliable
	flags.SetBit(ReliableFlag);

	//make a thread for this session
	running = true;
	recieve_thread = new std::thread(&session::ThreadLoop, this);
}

extern "C" {
  void Update(int id)
  {
	dll_sessions[id]->ThreadLoop();
  }
  double GetPing(int id)
  {
	  return dll_sessions[id]->GetPing();
  }
  int CreateSession(char *ip, int port)
  {
    //get the id for the network stack
    int id;
    if (dll_unused_ids.size() != 0)
    {
      id = dll_unused_ids.back();
      dll_unused_ids.pop_back();
	  //make a new session
	  dll_sessions[id] = new session(ip, port);
    }
    else
    {
      id = dll_sessions.size();
      dll_sessions.push_back(new session(ip, port));
    }
    return id;
  }
  void DeleteSession(int id)
  {
	  //TODO Clean up the thread and mutex...
	  LOG("Deleting session " << id);
    //check if the id is not valid
    if (id < 0 || id > dll_sessions.size())
    {
      return;
    }
	//signal to the thread to end
	dll_sessions[id]->running = false;
    //free the memory
    delete dll_sessions[id];
	//zero out so that we know its empty
	dll_sessions[id] = 0;
    dll_unused_ids.push_back(id);
  }
  int SessionSend(int id, char *data, int length)
  {
	std::vector<char> packet(data, data + length);
	dll_sessions[id]->send_queue.push(packet);
	dll_sessions[id]->can_send = true;
	return length;
  }
  int SessionRecieve(int id, char *data, int length)
  {
	//dll_sessions[id].Recive(length);
	if (dll_sessions[id]->recieved_queue.size())
	{
		std::vector<char> packet = dll_sessions[id]->recieved_queue.pop();
		memcpy(data, packet.data(), packet.size());
		return packet.size();
	}
	return -1;
  }
  void SendEncryptionRequest(int id)
  {
    dll_sessions[id]->SendEncryption();
	dll_sessions[id]->can_send = true;
  }
}