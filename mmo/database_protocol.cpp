#include <string.h>

#include "database_protocol.h"

int CreateSetMessage(ProtocolLoader &protocol, char *buffer, unsigned id, unsigned row, void *data, int size)
{
  MessageType proto = protocol.LookUp("DatabaseSet");
  memcpy(buffer, &proto, sizeof(MessageType));
  *reinterpret_cast<unsigned *>(buffer+sizeof(MessageType)) = id;
  *reinterpret_cast<unsigned *>(buffer+sizeof(MessageType)+sizeof(unsigned)) = row;
  memcpy(buffer+sizeof(MessageType)+sizeof(unsigned)*2, data, size);
  return sizeof(MessageType)+sizeof(unsigned)*2 + size;
}
int CreateFindMessage(ProtocolLoader &protocol, char *buffer, unsigned row, void *data, int size)
{
  MessageType proto = protocol.LookUp("DatabaseFind");
  memcpy(buffer, &proto, sizeof(MessageType));
  *reinterpret_cast<unsigned *>(buffer+sizeof(MessageType)) = row;
  memcpy(buffer+sizeof(MessageType)+sizeof(unsigned), data, size);
  return sizeof(MessageType)+sizeof(unsigned) + size;
}

int CreateGetMessage(ProtocolLoader &protocol, char *buffer, unsigned id, unsigned row)
{
  MessageType proto = protocol.LookUp("DatabaseGet");
  memcpy(buffer, &proto, sizeof(MessageType));
  *reinterpret_cast<unsigned *>(buffer+sizeof(MessageType)) = id;
  *reinterpret_cast<unsigned *>(buffer+sizeof(MessageType)+sizeof(unsigned)) = row;
  return sizeof(MessageType)+sizeof(unsigned)*2;
}

int CreateCreateMessage(ProtocolLoader &protocol, char *buffer, unsigned nonce)
{
  MessageType proto = protocol.LookUp("DatabaseCreate");
  memcpy(buffer, &proto, sizeof(MessageType));
  *reinterpret_cast<unsigned *>(buffer+sizeof(MessageType)) = nonce;
  return sizeof(MessageType)+sizeof(unsigned);
}

int CreateDeleteMessage(ProtocolLoader &protocol, char *buffer, unsigned id)
{
  MessageType proto = protocol.LookUp("DatabaseDelete");
  memcpy(buffer, &proto, sizeof(MessageType));
  *reinterpret_cast<unsigned *>(buffer+sizeof(MessageType)) = id;
  return sizeof(MessageType)+sizeof(unsigned);
}