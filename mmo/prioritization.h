/*!
  \author
    Wyatt Lavigueure
  \date
    5/24/2017
  \brief
    Prioritization layer tries to keep the network under control by holding packets back when there is high traffic
*/
#ifndef PRIO_H
#define PRIO_H
#include <vector>

#include "reliability.h" /*for packet save */

/*!
  \brief
    When sending things this will keep the bandwidth usage inside the desired bandwidth
*/
class Prioritization : public NetworkLayer
{
public:
  /*!
    \brief
      Initailze the bandwdth cap
    \param bandwidth
      The number of bytes per second we can send stuff at
  */
  Prioritization(double bandwidth);
  /*!
    \brief
      Frees memory of saved packets
  */
  ~Prioritization();
  /*!
    \brief
      Checks if the bandwidth can handle sending this packet right now, if so it sends it other wise it saves it to be sent later
  */
  int Send(char* buffer, int bytes, const sockaddr_in* dest, BitArray<HEADERSIZE> &flags);
  /*!
    \brief
      does nothing
  */
  int Receive(char* buffer, int bytes, sockaddr_in* location, BitArray<HEADERSIZE> &flags);
  /*!
    \brief
      trys sending packets again
  */
  void Update(double dt);
  void RemoveConnection(const sockaddr_in*addr);
private:
  double bandwidth;
};

#endif