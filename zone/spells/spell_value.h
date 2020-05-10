/**
 * @brief Value is a class that returns a value, could be a flat value or a value based on stats or anything
 */

#ifndef VALUE_H
#define VALUE_H

#include <string>
#include "spell.h"
#include "stream.h"

class Entity;

class SpellValue
{
  public:
  virtual ~SpellValue(){}
  virtual float get() = 0;
  virtual Stream &ToData(Stream &stream) = 0;
  virtual std::string getDescription() = 0;
  //for over time buffs
  virtual SpellValue *copy() = 0;
  //the index into the spell factory
  char type;
};

#endif