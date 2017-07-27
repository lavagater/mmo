#include "event.h"

Event::Event()
{
   mEvents.resize(EventType::Num);
}

Event::~Event()
{
  for (unsigned i = 0; i < EventType::Num; ++i)
  {
    for (auto it = mEvents[i].begin(); it != mEvents[i].end(); ++it)
    {
      delete *it;
    }
  }
}

void Event::SendEvent(EventType type, void* data)
{
   for (auto it = mEvents[type].begin(); it != mEvents[type].end(); ++it)
   {
      (*it)->Callback(data);
   }
}

