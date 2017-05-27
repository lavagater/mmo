/*!
  \author
    Wyatt Lavigueure
  \date
    5/24/2017
  \brief
    Uses blowfish encryption to encrypt packet if encryption flag is set
*/
#ifndef ENCRY_H
#define ENCRY_H

#include "network_stack.h"

/*!
  \brief
    Uses blowfish encryption to encrypt packet if encryption flag is set
*/
class Encryption : public NetworkLayer
{
public:
  /*!
    \brief
      If the encrypt flag is set then the entire buffer is encrypted
  */
  int Send(char* buffer, int bytes, const sockaddr_in* dest, BitArray<HEADERSIZE> &flags);
  /*!
    \brief
      If the encryption flag is set then dycrypt the entire message
  */
  int Receive(char* buffer, int bytes, sockaddr_in* location, BitArray<HEADERSIZE> &flags);
  void Update(double dt);
  void RemoveConnection(const sockaddr_in*addr);
  /*!
    \brief
      The keys for each connections blowfish encryption, these should be retrieved from the client using an
      asymetric encryption that is not part of the network stack(it could be but its a one time thing per client)
  */
  std::unordered_map<sockaddr_in, BlowFish, SockAddrHash> blowfish;
};

#endif