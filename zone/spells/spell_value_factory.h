/**
 * @brief Base class for all the spell value factories, used to create spells values
 */

#ifndef SPELL_VALUE_FACTORY_H
#define SPELL_VALUE_FACTORY_H

#include <string>
#include <spell.h>
#include <spell_factory.h>
#include <spell_value.h>

class SpellValueFactory
{
  public:
  //creates a random spell
  virtual SpellValue *Create(SpellData *data) = 0;
  //recreates spell from data
  virtual SpellValue *Create(SpellData *data, Stream &stream) = 0;
  char type;
};

#endif