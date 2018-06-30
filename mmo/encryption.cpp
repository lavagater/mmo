#include <string.h>
#include <math.h>

#include "blowfish.h"
#include "encryption.h"
#include "logger.h"

int Encryption::Send(char* buffer, int bytes, const sockaddr_in* dest, BitArray<HEADERSIZE> &flags)
{
  if (flags[EncryptFlag])
  {
    //if there is no key set for this encryption then dont send it
    if (blowfish.find(*dest) == blowfish.end())
    {
      //set error
      if (stack)
        stack->last_error = NOENCRYPTIONKEY;
      return NOENCRYPTIONKEY;
    }
    //save old size to put in packet
    int old_size = bytes;
    //round bytes to the nearest power of 8
    bytes = ceil(bytes / 8.0) * 8;
    //encrypt the buffer 8 bytes at a time
    for (int i = 0; i < bytes; i += 8)
    {
      blowfish[*dest].encrypt(*reinterpret_cast<unsigned *>(buffer + i), *reinterpret_cast<unsigned *>(buffer + i + 4));
    }
    //shift the buffer over 2 bytes to add the message size, the bytes wont be larger than 65536
    for (int i = bytes-1; i >= 0; --i)
    {
      buffer[i+2] = buffer[i];
    }
    memcpy(buffer, &old_size, 2);
    return bytes;
  }
  else
  {
    //not encypted do nothing
    return bytes;
  }
}
int Encryption::Receive(char* buffer, int bytes, sockaddr_in* location, BitArray<HEADERSIZE> &flags)
{
  if (flags[EncryptFlag])
  {
    //if there is no key set for this encryption then we cant do anything with it...
    if (blowfish.find(*location) == blowfish.end())
    {
      //set error
      if (stack)
        stack->last_error = NOENCRYPTIONKEY;
      LOGW("NOENCRYPTIONKEY" << std::endl);
      return NOENCRYPTIONKEY;
    }
    //get the number of bytes of original message
    int og_bytes = 0;
    memcpy(&og_bytes, buffer, 2);
    //shift the packet over by 2 bytes
    for (int i = 0; i < bytes - 2; ++i)
    {
      buffer[i] = buffer[i+2];
    }
    //decrypt the buffer
    for (int i = 0; i < bytes; i += 8)
    {
      blowfish[*location].decrypt(*reinterpret_cast<unsigned *>(buffer + i), *reinterpret_cast<unsigned *>(buffer + i + 4));
    }
    //ignore pad bytes
    return og_bytes;
  }
  else
  {
    //not encypted do nothing
    return bytes;
  }
}
void Encryption::Update(__attribute__((unused))double dt)
{
}
void Encryption::RemoveConnection(const sockaddr_in*addr)
{
  //remove from the map
  if (blowfish.find(*addr) != blowfish.end())
  {
    blowfish.erase(blowfish.find(*addr));
  }
}