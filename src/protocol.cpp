#include <string.h>

#include "protocol.h"

int CreateSetMessage(char *buffer, unsigned id, unsigned row, void *data, int size)
{
  buffer[0] = DatabaseSet;
  *reinterpret_cast<unsigned *>(buffer+1) = id;
  *reinterpret_cast<unsigned *>(buffer+1+sizeof(unsigned)) = row;
  memcpy(buffer+1+sizeof(unsigned)*2, data, size);
  return 1+sizeof(unsigned)*2 + size;
}
int CreateFindMessage(char *buffer, unsigned row, void *data, int size)
{
  buffer[0] = DatabaseFind;
  *reinterpret_cast<unsigned *>(buffer+1) = row;
  memcpy(buffer+1+sizeof(unsigned), data, size);
  return 1+sizeof(unsigned) + size;
}

int CreateGetMessage(char *buffer, unsigned id, unsigned row)
{
  buffer[0] = DatabaseGet;
  *reinterpret_cast<unsigned *>(buffer+1) = id;
  *reinterpret_cast<unsigned *>(buffer+1+sizeof(unsigned)) = row;
  return 1+sizeof(unsigned)*2;
}

int CreateCreateMessage(char *buffer, unsigned nonce)
{
  buffer[0] = DatabaseCreate;
  *reinterpret_cast<unsigned *>(buffer+1) = nonce;
  return 1+sizeof(unsigned);
}

int CreateDeleteMessage(char *buffer, unsigned id)
{
  buffer[0] = DatabaseDelete;
  *reinterpret_cast<unsigned *>(buffer+1) = id;
  return 1+sizeof(unsigned);
}