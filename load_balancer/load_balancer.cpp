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
#include "query.h"


LoadBalancer::LoadBalancer()
  :sock(CreateSocket(IPPROTO_UDP)),
   stack(sock)
{
  config.Init("resources/load_balancer.conf");
  LOG("config loaded" << std::endl);
  sockaddr_in local;
  CreateAddress(0,static_cast<int>(config.properties["port"]),&local);
  CreateAddress(static_cast<std::string>(config.properties["account_db_ip"]).c_str(),static_cast<int>(config.properties["account_db_port"]),&account_database);
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
  network_signals.signals[protocol.LookUp("CreateAccount")].Connect(std::bind(&LoadBalancer::CreateAccount, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  network_signals.signals[protocol.LookUp("Login")].Connect(std::bind(&LoadBalancer::Login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  network_signals.signals[protocol.LookUp("ChangePassword")].Connect(std::bind(&LoadBalancer::ChangePassword, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  network_signals.signals[protocol.LookUp("Query")].Connect(std::bind(&LoadBalancer::QueryResponse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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
void LoadBalancer::CreateAccount(char *buffer, unsigned n, sockaddr_in *addr)
{
  //create account is formed with
  //message type first sizeof(MessageType) bytes
  //16 bytes for the username
  //40 bytes for password hash
  //n must be exactly 56 + sizeof(MessageType) bytes then
  if (n < 56 + sizeof(MessageType))
  {
    LOGW("Create account message only length " << n);
  }
  //move past message type since we already know its a createaccount
  buffer += sizeof(MessageType);
  //read username, TODO:double the size to account for the wide characters
  char username[17] = {0};
  //TODO make a blobstruct for the username
  memcpy(username, buffer, 16);
  //read the password
  char password[41] = {0};
  memcpy(password, buffer+16, 40);
  LOG("Creating account " << username << " password hash = " << password);
  for (int i = 0; i < 40; ++i)
  {
    LOG("password[" << i << "] = " << password[i]);
  }
  //send query to database
  query_id += 1;
  query_callbacks[query_id] = std::bind(&LoadBalancer::SendLoginMessage, this, *addr, std::placeholders::_1, std::placeholders::_2);
  int len = CreateQueryMessage(protocol, query_id, buffer, STRINGIZE(
    main(string username, string password)
    {
      print("This is the script\n");
      print(username, "\n");
      print(password, "\n");
      vector res = find(0, username);
      print(res, "\n");
      if (Size(res) > 0)
      {
        return int(-1);
      }
      int id = create();
      set(id, 0, username);
      set(id, 1, password);
      print("Script finished account id = ", id);
      return int(id);
    }
  ), std::string(username), std::string(password));
  stack.Send(buffer, len, &account_database, flags[account_database]);
}
void LoadBalancer::Login(char *buffer, unsigned n, sockaddr_in *addr)
{
  //create account is formed with
  //message type first sizeof(MessageType) bytes
  //16 bytes for the username
  //40 bytes for password hash
  //n must be exactly 56 + sizeof(MessageType) bytes then
  if (n < 56 + sizeof(MessageType))
  {
    LOGW("login message only length " << n);
  }
  //move past message type since we already know its a createaccount
  buffer += sizeof(MessageType);
  //read username
  char username[17] = {0};
  memcpy(username, buffer, 16);
  //read the password
  char password[41] = {0};
  memcpy(password, buffer+16, 40);
  LOG("loging onto account " << username << " password hash = " << password);
  for (int i = 0; i < 40; ++i)
  {
    LOG("password[" << i << "] = " << password[i]);
  }
  //send query to database
  query_id += 1;
  query_callbacks[query_id] = std::bind(&LoadBalancer::SendLoginMessage, this, *addr, std::placeholders::_1, std::placeholders::_2);
  int len = CreateQueryMessage(protocol, query_id, buffer, STRINGIZE(
    main(string username, string password)
    {
      print("Login\n");
      print(username, "\n");
      print(password, "\n");
      vector res = find(0, username);
      print(res, "\n");
      if (Size(res) == 0)
      {
        print("Not found");
        return int(-1);
      }
      string saved_pass = get(res[0], 1);
      print("saved password = ", saved_pass, "\n");
      if (saved_pass != password)
      {
        print("passwords dont match");
        return int(-1);
      }
      print("Script finished account id = ", res[0]);
      return int(res[0]);
    }
  ), std::string(username), std::string(password));
  stack.Send(buffer, len, &account_database, flags[account_database]);
}
void LoadBalancer::ChangePassword(char *buffer, unsigned n, sockaddr_in *addr)
{
  //create account is formed with
  //message type first sizeof(MessageType) bytes
  //16 bytes for the username
  //40 bytes for old password hash
  //40 bytes for new password hash
  //n must be exactly 96 + sizeof(MessageType) bytes then
  if (n < 96 + sizeof(MessageType))
  {
    LOGW("Change password message only length " << n);
  }
  //move past message type since we already know its a createaccount
  buffer += sizeof(MessageType);
  //read username
  char username[17] = {0};
  memcpy(username, buffer, 16);
  //read the password
  char password[41] = {0};
  memcpy(password, buffer+16, 40);
  char newpassword[41] = {0};
  memcpy(newpassword, buffer+56, 40);
  LOG("change password on account " << username << " password hash = " << password << " new password = " << newpassword);
  //send query to database
  query_id += 1;
  query_callbacks[query_id] = std::bind(&LoadBalancer::SendLoginMessage, this, *addr, std::placeholders::_1, std::placeholders::_2);
  int len = CreateQueryMessage(protocol, query_id, buffer, STRINGIZE(
    main(string username, string password, string new_password)
    {
      print("change password\n");
      print(username, "\n");
      print(password, "\n");
      print(new_password, "\n");
      vector res = find(0, username);
      print(res, "\n");
      if (Size(res) == 0)
      {
        print("Not found");
        return int(-1);
      }
      string saved_pass = get(res[0], 1);
      print("saved password = ", saved_pass, "\n");
      if (saved_pass != password)
      {
        print("passwords dont match");
        return int(-1);
      }
      set(res[0], 1, new_password);
      print("Script finished account id = ", res[0]);
      return int(res[0]);
    }
  ), std::string(username), std::string(password), std::string(newpassword));
  stack.Send(buffer, len, &account_database, flags[account_database]);
}
void LoadBalancer::QueryResponse(char *buffer, unsigned n, sockaddr_in *addr)
{
  (void)addr;
  unsigned id;
  unsigned size;
  char *data = 0;
  ParseQueryResponse(buffer, n, id, data, size);
  LOG("id = " <<id<<" size = " << size);
  if (size == sizeof(int))
  {
    LOG("return value = " << *reinterpret_cast<int*>(data));
  }
  //here we should signal based on the id
  query_callbacks[id](data, size);
}
void LoadBalancer::SendLoginMessage(sockaddr_in addr, char *data, unsigned size)
{
  if (size != sizeof(int))
  {
    LOGW("Incorrect size = " << size);
  }
  int is_valid = *reinterpret_cast<int*>(data);
  //the reply to the client for the login/create account/change password
  //message type is Login
  *reinterpret_cast<MessageType*>(buffer) = protocol.LookUp("Login");
  //first byte is wether it was successful or not, -1 not successfull, 1 successfull
  buffer[sizeof(MessageType)] = is_valid;
  stack.Send(buffer, sizeof(MessageType) + 1, &addr, flags[from]);
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