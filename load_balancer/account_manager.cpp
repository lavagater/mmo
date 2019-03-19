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
  load_balancer->network_signals.signals[load_balancer->protocol.LookUp("CreateAccount")].Connect(std::bind(&AccountManager::CreateAccount, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  load_balancer->network_signals.signals[load_balancer->protocol.LookUp("Login")].Connect(std::bind(&AccountManager::Login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  load_balancer->network_signals.signals[load_balancer->protocol.LookUp("ChangePassword")].Connect(std::bind(&AccountManager::ChangePassword, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  load_balancer->network_signals.signals[load_balancer->protocol.LookUp("LookUp")].Connect(std::bind(&AccountManager::GetUsername, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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
void AccountManager::SendLoginMessage(sockaddr_in addr, char *data, unsigned size)
{
  if (size != sizeof(int))
  {
    LOGW("Incorrect size = " << size);
  }
  int id = *reinterpret_cast<int*>(data);
  LOG("player id = " << id);
  //the reply to the client for the login/create account/change password
  //message type is Login
  *reinterpret_cast<MessageType*>(load_balancer->buffer) = load_balancer->protocol.LookUp("Login");
  *reinterpret_cast<int*>(load_balancer->buffer+sizeof(MessageType)) = id;
  //TODO free these when the client disconnects
  load_balancer->clients[addr] = id;
  load_balancer->clients_by_id[id] = addr;
  //TODO make a better system for this
  //tell the zone about the new player
  load_balancer->stack.Send(load_balancer->buffer, sizeof(MessageType) + sizeof(id), &addr, load_balancer->flags[addr]);
  load_balancer->stack.Send(load_balancer->buffer, sizeof(MessageType) + sizeof(id), &(load_balancer->zone_array[0]), load_balancer->flags[load_balancer->zone_array[0]]);
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