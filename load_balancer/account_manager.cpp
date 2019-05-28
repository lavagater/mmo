#include "account_manager.h"
#include "logger.h"
#include "database_protocol.h"
#include "load_balancer_protocol.h"
#include "query.h"
#include "utils.h"
#include "load_balancer.h"

void AccountManager::SetUp(LoadBalancer *load_balancer)
{
  this->load_balancer = load_balancer;
  //the life time of the account manager is assumed to be the life time of the program
  load_balancer->network_signals.signals[load_balancer->protocol.LookUp("CreateAccount")].Connect(std::bind(&AccountManager::CreateAccount, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  load_balancer->network_signals.signals[load_balancer->protocol.LookUp("Login")].Connect(std::bind(&AccountManager::Login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  load_balancer->network_signals.signals[load_balancer->protocol.LookUp("ChangePassword")].Connect(std::bind(&AccountManager::ChangePassword, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  load_balancer->network_signals.signals[load_balancer->protocol.LookUp("LookUp")].Connect(std::bind(&AccountManager::GetUsername, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  load_balancer->stack.disconnected.Connect(std::bind(&AccountManager::OnClientDisconnect, this, std::placeholders::_1));
}
void AccountManager::OnClientDisconnect(const sockaddr_in *addr)
{
  if (load_balancer->clients.find(*addr) == load_balancer->clients.end() || load_balancer->clients[*addr] == -1)
  {
    LOG("Client that was not logged in has disconnected");
    return;
  }
  char buffer[MAXPACKETSIZE];
  //need to figure out which zone the client is in
  load_balancer->query_id += 1;
  load_balancer->query_callbacks[load_balancer->query_id] = std::bind(&AccountManager::SendDisconnectMessage, this, *addr, std::placeholders::_1, std::placeholders::_2);
  int len = CreateQueryMessage(load_balancer->protocol, load_balancer->query_id, &buffer[0], STRINGIZE(
    main(unsigned id)
    {
      vector res = find(0, id);
      if (Size(res) == 0)
      {
        print("player not found... account ", id);
      }
      string zone_name = get(res[0], 1);
      print("Client of id ", id, " disconnecting from zone ", zone_name, "\n");
      return zone_name;
    }
  ), load_balancer->clients[*addr]);
  load_balancer->stack.Send(&buffer[0], len, &load_balancer->players_database, load_balancer->flags[load_balancer->players_database]);
}
void AccountManager::SendDisconnectMessage(sockaddr_in client_addr, char *buffer, unsigned size)
{
  (void)(size);
  std::string zone = std::string(buffer);
  sockaddr_in zone_addr = load_balancer->GetZone(zone);
  char message[MAXPACKETSIZE];
  buffer = &message[0];
  *reinterpret_cast<MessageType*>(buffer) = load_balancer->protocol.LookUp("Disconnected");
  buffer += sizeof(MessageType);
  *reinterpret_cast<sockaddr_in*>(buffer) = client_addr;
  buffer += sizeof(sockaddr_in);
  load_balancer->stack.Send(message, buffer - &message[0], &zone_addr, load_balancer->flags[zone_addr]);
}
void AccountManager::CreateAccount(char *buffer, unsigned n, sockaddr_in *addr)
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
  //read username
  char username[33] = {0};
  memcpy(username, buffer, 32);
  BlobStruct user_blob;
  user_blob.data = username;
  user_blob.size = 32;
  buffer += 32;
  //read the password
  char password[81] = {0};
  memcpy(password, buffer, 80);
  BlobStruct pass_blob;
  pass_blob.data = password;
  pass_blob.size = 80;
  buffer += 80;
  //send query to database
  load_balancer->query_id += 1;
  load_balancer->query_callbacks[load_balancer->query_id] = std::bind(&AccountManager::SendLoginMessage, this, *addr, std::placeholders::_1, std::placeholders::_2);
  //reuse the databases static buffer
  int len = CreateQueryMessage(load_balancer->protocol, load_balancer->query_id, &load_balancer->buffer[0], STRINGIZE(
    main(blob username, blob password)
    {
      print("This is the script\n");
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
  ), user_blob, pass_blob);
  LOG("query message length = " << len);
  load_balancer->stack.Send(&load_balancer->buffer[0], len, &load_balancer->account_database, load_balancer->flags[load_balancer->account_database]);
}
void AccountManager::Login(char *buffer, unsigned n, sockaddr_in *addr)
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
  char username[33] = {0};
  memcpy(username, buffer, 32);
  BlobStruct user_blob;
  user_blob.data = username;
  user_blob.size = 32;
  buffer += 32;
  //read the password
  char password[81] = {0};
  memcpy(password, buffer, 80);
  BlobStruct pass_blob;
  pass_blob.data = password;
  pass_blob.size = 80;
  buffer += 80;
  //send query to database
  load_balancer->query_id += 1;
  load_balancer->query_callbacks[load_balancer->query_id] = std::bind(&AccountManager::SendLoginMessage, this, *addr, std::placeholders::_1, std::placeholders::_2);
  int len = CreateQueryMessage(load_balancer->protocol, load_balancer->query_id, &load_balancer->buffer[0], STRINGIZE(
    main(blob username, blob password)
    {
      print("Login\n");
      vector res = find(0, username);
      print(res, "\n");
      if (Size(res) == 0)
      {
        print("Not found");
        return int(-1);
      }
      blob saved_pass = get(res[0], 1);
      print("saved password = ", saved_pass, "\n");
      if (saved_pass != password)
      {
        print("passwords dont match");
        return int(-1);
      }
      print("Script finished account id = ", res[0]);
      return int(res[0]);
    }
  ), user_blob, pass_blob);
  load_balancer->stack.Send(&load_balancer->buffer[0], len, &load_balancer->account_database, load_balancer->flags[load_balancer->account_database]);
}
void AccountManager::ChangePassword(char *buffer, unsigned n, sockaddr_in *addr)
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
  char username[33] = {0};
  memcpy(username, buffer, 32);
  BlobStruct user_blob;
  user_blob.data = username;
  user_blob.size = 32;
  buffer += 32;
  //read the password
  char password[81] = {0};
  memcpy(password, buffer, 80);
  BlobStruct pass_blob;
  pass_blob.data = password;
  pass_blob.size = 80;
  buffer += 80;
  //read the new password
  char newpassword[81] = {0};
  memcpy(newpassword, buffer, 80);
  BlobStruct newpass_blob;
  newpass_blob.data = newpassword;
  newpass_blob.size = 80;
  buffer += 80;
  //send query to database
  load_balancer->query_id += 1;
  LOG("query_id = " << load_balancer->query_id);
  load_balancer->query_callbacks[load_balancer->query_id] = std::bind(&AccountManager::SendLoginMessage, this, *addr, std::placeholders::_1, std::placeholders::_2);
  int len = CreateQueryMessage(load_balancer->protocol, load_balancer->query_id, &load_balancer->buffer[0], STRINGIZE(
    main(blob username, blob password, blob new_password)
    {
      print("change password\n");
      vector res = find(0, username);
      print(res, "\n");
      if (Size(res) == 0)
      {
        print("Not found");
        return int(-1);
      }
      blob saved_pass = get(res[0], 1);
      if (saved_pass != password)
      {
        return int(-1);
      }
      set(res[0], 1, new_password);
      return int(res[0]);
    }
  ), user_blob, pass_blob, newpass_blob);
  load_balancer->stack.Send(&load_balancer->buffer[0], len, &load_balancer->account_database, load_balancer->flags[load_balancer->account_database]);
}
void AccountManager::AddClient(int id, sockaddr_in *addr)
{
  //remove old clients first
  sockaddr_in old_addr = load_balancer->clients_by_id[id].addr;
  auto it = load_balancer->clients.find(old_addr);
  if (it != load_balancer->clients.end())
  {
    load_balancer->clients.erase(it);
  }
  //add new client
  load_balancer->clients[*addr] = id;
  load_balancer->clients_by_id[id].addr = *addr;
}
void AccountManager::SendLoginMessage(sockaddr_in addr, char *data, unsigned size)
{
  if (size != sizeof(int))
  {
    LOGW("Incorrect size = " << size);
  }
  int id = *reinterpret_cast<int*>(data);
  LOG("player id = " << id);
  AddClient(id, &addr);
  if (id < 0)
  {
    SendLoginMessageQuery(id, "");
    return;
  }
  //do a db query to find out which zone the client is in
  load_balancer->query_id += 1;
  load_balancer->query_callbacks[load_balancer->query_id] = std::bind(&AccountManager::OnLoginMessageQuery, this, id, std::placeholders::_1, std::placeholders::_2);
  int len = CreateQueryMessage(load_balancer->protocol, load_balancer->query_id, &load_balancer->buffer[0], STRINGIZE(
    main(int id)
    {
      print("Getting players zone\n");
      //get the id
      vector res = find(0, int(id));
      print(res, "\n");
      if (Size(res) == 0)
      {
        print("Player does not exist,lets create it\n");
        int new_player = create();
        print("created\n");
        //set the account id
        set(new_player, 0, id);
        print("id set\n");
        set(new_player, 1, "first zone");
        print("zone set\n");
        //set the x and y position to zero
        set(new_player, 2, 0);
        print("x pos set\n");
        set(new_player, 3, 0);
        print("y pos set\n");
        return get(new_player, 1);
      }
      return get(res[0], 1);
    }
  ), id);
  load_balancer->stack.Send(load_balancer->buffer, len, &load_balancer->players_database, load_balancer->flags[load_balancer->players_database]);
}

void AccountManager::SendLoginMessageQuery(int id, std::string zone)
{
  LOG("Player " << id << " logging on to zone " << zone);
  *reinterpret_cast<MessageType*>(load_balancer->buffer) = load_balancer->protocol.LookUp("Login");
  *reinterpret_cast<int*>(load_balancer->buffer+sizeof(MessageType)) = id;
  load_balancer->stack.Send(load_balancer->buffer, sizeof(MessageType) + sizeof(id), &load_balancer->clients_by_id[id].addr, load_balancer->flags[load_balancer->clients_by_id[id].addr]);
  if (id >= 0)
  {
    //tell the zone about the new player
    //when sending to the zone add the clients address to the end
    sockaddr_in zone_addr = load_balancer->GetZone(zone);
    *reinterpret_cast<sockaddr_in*>(load_balancer->buffer+sizeof(MessageType)+sizeof(id)) = load_balancer->clients_by_id[id].addr;
    load_balancer->stack.Send(load_balancer->buffer, sizeof(MessageType) + sizeof(id)+sizeof(sockaddr_in), &zone_addr, load_balancer->flags[zone_addr]);
  }
}

void AccountManager::OnLoginMessageQuery(int id, char *data, unsigned size)
{
  (void)(size);
  std::string zone = std::string(data);
  //save the players zone, so forward messages can go there
  load_balancer->clients_by_id[id].zone = zone;
  SendLoginMessageQuery(id, zone);
}

void AccountManager::SendUsername(sockaddr_in addr, char *data, unsigned size, unsigned id)
{
  *reinterpret_cast<MessageType*>(load_balancer->buffer) = load_balancer->protocol.LookUp("LookUp");
  *reinterpret_cast<unsigned*>(load_balancer->buffer+sizeof(MessageType)) = id;
  memcpy(&load_balancer->buffer[0] + sizeof(MessageType) + sizeof(unsigned), data, size);
  load_balancer->stack.Send(load_balancer->buffer, sizeof(MessageType) + sizeof(id) + size, &addr, load_balancer->flags[addr]);
}

void AccountManager::GetUsername(char *buffer, unsigned n, sockaddr_in *addr)
{
  if (n < sizeof(int))
  {
    LOGW("Incorrect size = " << n);
    return;
  }
  buffer += sizeof(MessageType);
  unsigned id = *reinterpret_cast<int*>(buffer);
  load_balancer->query_id += 1;
  load_balancer->query_callbacks[load_balancer->query_id] = std::bind(&AccountManager::SendUsername, this, *addr, std::placeholders::_1, std::placeholders::_2, id);
  int len = CreateQueryMessage(load_balancer->protocol, load_balancer->query_id, &load_balancer->buffer[0], STRINGIZE(
    main(unsigned id)
    {
      print("looking up id ", unsigned(id));
      return get(unsigned(id), 0);
    }
  ), id);
  load_balancer->stack.Send(&load_balancer->buffer[0], len, &load_balancer->account_database, load_balancer->flags[load_balancer->account_database]);
}