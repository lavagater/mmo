/**
 * @brief Base class for buffs and effects that last for a certain amount of time.
 */

#ifndef OVERTIME_EFFECT_H
#define OVERTIME_EFFECT_H

#include <string>
#include <spell.h>
#include "entity.h"
#include "zone.h"
#include "stat_scale.h"
#include "effect.h"

class OverTimeEffect : public Effect 
{
  public:
  virtual ~OverTimeEffect();
  /**
   * @brief creates a copy of the OverTimeEffect that will aplly the effect for the duration and then delete itself.
   */
  void cast(EntityComponent *target);

  /**
   * @brief called after the effect is copied in the cast function. This is when the derived class can start applying its effects.
   */
  virtual void startEffect(EntityComponent *target) = 0;

  Dispatcher *dispatcher = 0;
  float duration;
  TimedMessage effect_end;
  Connection target_died;
  private:
  void onDurationEnd();
};

#endif