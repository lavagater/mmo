#include "dispatcher.h"


void Dispatcher::Dispatch(std::function<void()> message)
{
  messages.push(message);
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
  if (messages.size() > 0)
  {
      messages.back()();
      messages.pop();
  }
}