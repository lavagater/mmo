/**
 * @brief Deal damage over time
 */

#ifndef BURN_EFFECT_H
#define BURN_EFFECT_H

#include <string>
#include "overtime_effect.h"
#include "spell_value.h"
#include "stream.h"

class BurnEffect : public OverTimeEffect
{
  public:
  ~BurnEffect();
  void startEffect(EntityComponent *target);
  static OverTimeEffect *Create(Stream &stream, SpellData *spell_data);
  std::string getDescription();
  Stream &ToData(Stream &stream);
  Effect *copy();
  void tick(EntityComponent *target);
  SpellValue *dps = 0;
  TimedMessage damage_tick;
};

#endif