/*!
  \author
    Wyatt Lavigueure
  \date
    3/04/2019
  \brief
    Dispatcher used to send all the messages from one place.
*/
#ifndef DISPATCHER_H
#define DISPATCHER_H
#include <functional>
#include <queue>

class Dispatcher
{
public:
  //add a message to the dispatcher queue
  void Dispatch(std::function<void()> message);
  //start the dispatcher loop
  void Run();
  //instead of using the dispatcher loop, this function can be polled
  void Update();
private:
  std::queue<std::function<void()> > messages;
};
#endif