#include <string.h>

#include "database_protocol.h"

int CreateSetMessage(char *buffer, unsigned id, unsigned row, void *data, int size)
{
  Protocol proto = Protocol::DatabaseSet;
  memcpy(buffer, &proto, message_type_size);
  *reinterpret_cast<unsigned *>(buffer+message_type_size) = id;
  *reinterpret_cast<unsigned *>(buffer+message_type_size+sizeof(unsigned)) = row;
  memcpy(buffer+message_type_size+sizeof(unsigned)*2, data, size);
  return message_type_size+sizeof(unsigned)*2 + size;
}
int CreateFindMessage(char *buffer, unsigned row, void *data, int size)
{
  Protocol proto = Protocol::DatabaseFind;
  memcpy(buffer, &proto, message_type_size);
  *reinterpret_cast<unsigned *>(buffer+message_type_size) = row;
  memcpy(buffer+message_type_size+sizeof(unsigned), data, size);
  return message_type_size+sizeof(unsigned) + size;
}

int CreateGetMessage(char *buffer, unsigned id, unsigned row)
{
  Protocol proto = Protocol::DatabaseGet;
  memcpy(buffer, &proto, message_type_size);
  *reinterpret_cast<unsigned *>(buffer+message_type_size) = id;
  *reinterpret_cast<unsigned *>(buffer+message_type_size+sizeof(unsigned)) = row;
  return message_type_size+sizeof(unsigned)*2;
}

int CreateCreateMessage(char *buffer, unsigned nonce)
{
  Protocol proto = Protocol::DatabaseCreate;
  memcpy(buffer, &proto, message_type_size);
  *reinterpret_cast<unsigned *>(buffer+message_type_size) = nonce;
  return message_type_size+sizeof(unsigned);
}

int CreateDeleteMessage(char *buffer, unsigned id)
{
  Protocol proto = Protocol::DatabaseDelete;
  memcpy(buffer, &proto, message_type_size);
  *reinterpret_cast<unsigned *>(buffer+message_type_size) = id;
  return message_type_size+sizeof(unsigned);
}