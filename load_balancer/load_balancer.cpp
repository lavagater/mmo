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


LoadBalancer::LoadBalancer()
  :sock(CreateSocket(IPPROTO_UDP)),
   stack(sock)
{
  config.Init("resources/load_balancer.conf");
  LOG("config loaded" << std::endl);
  sockaddr_in local;
  CreateAddress(0,static_cast<int>(config.properties["port"]),&local);
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
    return;
  }
  ((Encryption*)(stack.layers[2]))->blowfish[from] = BlowFish((unsigned int *)key, length*sizeof(unsigned int));
  flags[from].SetBit(EncryptFlag);
  //send back a message saying that i got the key
  stack.Send(buffer, sizeof(MessageType), addr, flags[from]);
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
void LoadBalancer::run()
{
  //main loop
  while(true)
  {
    //check for messages
    int n = stack.Receive(buffer, MAXPACKETSIZE, &from);
    flags[from].SetBit(ReliableFlag);
    if (n >= (int)sizeof(MessageType))
    {
      LOG("Recieved message of length " << n);
      MessageType type = 0;
      memcpy(&type, buffer, sizeof(MessageType));
      LOG("Recieved message type " << protocol.LookUp(type) << ":" << type);
      network_signals.signals[type](buffer, n, &from);
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

  LoadBalancer lb;
  lb.run();
  return 420;
}