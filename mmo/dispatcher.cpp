#include "dispatcher.h"


void Dispatcher::Dispatch(std::function<void()> message)
{
  messages.push(message);
}
void Dispatcher::Dispatch(std::function<void(double)> message, double time)
{
  TimedMessage tm;
  tm.start_time = timer.GetTotalTime();
  tm.wait_time = time;
  tm.message = message;
  timed_messages.push_back(tm);
}
void Dispatcher::Run()
{
  while(1)
  {
    Update();
  }
}
void Dispatcher::Update()
{
  //check timed messages
  for (unsigned i = 0; i < timed_messages.size(); ++i)
  {
    if (timer.GetTotalTime() > timed_messages[i].start_time + timed_messages[i].wait_time)
    {
      TimedMessage tm = timed_messages[i];
      //remove message before calling message because the message might add a timed message
      timed_messages.erase(timed_messages.begin()+i);
      //call function
      tm.message(timer.GetTotalTime() - tm.start_time);
      //leave funtion, only want to call one message per update call
      return;
    }
  }
  if (messages.size() > 0)
  {
      messages.back()();
      messages.pop();
  }
}