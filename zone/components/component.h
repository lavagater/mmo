/**
 * @brief Base class for components
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include <istream>
#include <ostream>

class GameObject;

class Component
{
public:
  virtual void Init() {}
  //initialize the component data from a stream
  virtual void Load(std::istream &) {}
  //used to create a new level file for if the server has to be killed
  virtual void Write(std::ostream &) {}
  virtual ~Component() {}
  GameObject *game_object;
};

#endif