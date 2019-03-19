#include "game_object.h"

GameObject::~GameObject()
{
  for (auto it = components.begin(); it != components.end(); ++it)
  {
    if (it->second)
    {
        delete it->second;
    }
  }
  components.clear();
}