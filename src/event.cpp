#include "event.h"

Event::Event()
{
   mEvents.resize(EventType::Num);
}

void Event::SendEvent(EventType type, void* data)
{
   for (auto it = mEvents[type].begin(); it != mEvents[type].end(); ++it)
   {
      (*it)->Callback(data);
   }
}

