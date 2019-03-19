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

#include "frame_rate.h"
#include <functional>
#include <queue>

struct TimedMessage
{
  double start_time;
  double wait_time;
  std::function<void(double)> message;
};

class Dispatcher
{
public:
  //add a message to the dispatcher queue
  void Dispatch(std::function<void()> message);
  //add a message that will after a certain amount of time
  void Dispatch(std::function<void(double)> message, double time);
  //start the dispatcher loop
  void Run();
  //instead of using the dispatcher loop, this function can be polled
  void Update();
  FrameRate timer;
private:
  std::queue<std::function<void()> > messages;
  std::vector<TimedMessage> timed_messages;
};
#endif