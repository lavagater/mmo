/**
 * @brief Factory for effects
 */

#ifndef EFFECT_FACTORY_H
#define EFFECT_FACTORY_H

#include <string>
#include <spell.h>
#include <spell_factory.h>
#include <effect.h>


class EffectFactory
{
  public:
  virtual ~EffectFactory(){}
  //creates a random spell
  virtual Effect *Create(SpellData *data) = 0;
  //recreates spell from data
  virtual Effect *Create(SpellData *data, Stream &) = 0;
  char type;
};

#endif