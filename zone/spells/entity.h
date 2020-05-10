/**
 * @brief The effect is the result of when a skill is used on an entity
 */

#ifndef ENTITY__H
#define ENTITY__H

#include <list>
#include "spell_value.h"
#include "signals.h"

class Entity
{
  public:
  enum Stats
  {
    Health,
    CurrentHealth,
    Mana,
    CurrentMana,
    Strength,
    Intelegence,
    num
  };
  int semaphore;
  float base_stat[Stats::num];
  float current_stat[Stats::num];
  //points to either temp1 or temp2
  std::list<SpellValue*> buffs[Stats::num];
  //gets a stat with all buffs applied
  float getStat(Stats stat);

  Signals<float &> damaged_signal;
  Signals<float &> healed_signal;
  Signals<float &> mana_burned_signal;
  Signals<float &> mana_recovered_signal;
  void damage(float amount);
  void heal(float amount);
  void burn_mana(float amount);
  void recover_mana(float amount);
};

#endif