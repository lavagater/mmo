#include "dispatcher.h"


void Dispatcher::Dispatch(std::function<void()> message)
{
  messages.push(message);
}
TimedMessage Dispatcher::Dispatch(std::function<void(double)> message, double time)
{
  TimedMessage tm;
  tm.start_time = timer.GetTotalTime();
  tm.wait_time = time;
  tm.message = message;
  timed_messages.push_back(tm);
  return tm;
}
void Dispatcher::RemoveTimedMessage(TimedMessage &tm)
{
  for (auto it = timed_messages.begin(); it != timed_messages.end(); ++it)
  {
    //this should be fine since the start times will almost be different for sure
    if (tm.start_time == it->start_time && tm.wait_time == it->wait_time)
    {
      timed_messages.erase(it);
      break;
    }
  }
}
void Dispatcher::Dispatch(std::function<void()> message, void *id)
{
  other_messages.push_back(std::make_pair(message, id));
}
bool Dispatcher::RemoveMessageById(void *id)
{
  bool ret = false;
  for (auto it = other_messages.begin(); it != other_messages.end();)
  {
    if (it->second == id)
    {
      it = other_messages.erase(it);
      ret = true;
    }
    else
    {
      ++it;
    }
  }
  return ret;
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
      messages.front()();
      messages.pop();
  }
  if (other_messages.size() > 0)
  {
    other_messages.front().first();
    //ah its so inefficiant to pop front
    other_messages.pop_front();
  }
}