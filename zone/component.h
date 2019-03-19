/**
 * @brief Base class for components
 */

#ifndef COMPONENT_H
#define COMPONENT_H

class GameObject;

class Component
{
public:
  virtual void Init() {};
  virtual ~Component() {};
  GameObject *game_object;
};

#endif