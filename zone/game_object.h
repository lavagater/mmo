/**
 * @brief Class for all objects in the zone
 */

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "component.h"
#include "wyatt_sock.h"
#include "signals.h"
#include <unordered_map>

//macros to make make function calls nicer (because cant go from tempate to char *)
#define GETCOMP(gameobject, comp) reinterpret_cast<comp*>((gameobject)->components[#comp])
#define ADDCOMP(gameobject, comp) (gameobject)->AddComponent<comp>(#comp)
#define RMCOMP(gameobject, comp) (gameobject)->RemoveComponent(#comp)

class Zone;

class GameObject
{
public:
  ~GameObject();
  template<typename T>
  T *AddComponent(std::string name)
  {
    components[name] = new T();
    //do component init stuff here
    components[name]->game_object = this;
    components[name]->Init();
    return (T*)components[name];
  }
  void RemoveComponent(std::string name)
  {
    auto it = components.find(name);
    if (it != components.end())
    {
      if (it->second)
      {
        delete it->second;
      }
      components.erase(it);
    }
  }
  //send message saying this object is deleted
  void SendDeleteMessage();
  //zone specific id
  unsigned id;
  //a number that the client uses to determine what kind of object it is
  //its pretty much just a hard coded hack
  unsigned type;
  //for debugging
  std::string name;
  //pointer to the zone, for all the fun signals and what not
  Zone *zone;
  //map of all the components of this game object
  std::unordered_map<std::string, Component*> components;
  //signal that this object is removed, so that anywhere that has pointers to this object can clean up
  Signals<GameObject*> destroy_signal;
};

#endif