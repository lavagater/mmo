#include "entity.h"

float Entity::getStat(Stats stat)
{
  //using the lightswitch pattern, this is not multi threaded though
  if (semaphore == 0)
  {
    //clear the current stat for all the stats
    for (unsigned i = 0; i < Stats::num; ++i)
    {
      current_stat[i] = 0;
    }
  }
  semaphore += 1;

  //if the stat has already been calulated return it
  if (current_stat[stat] == 0)
  {
    current_stat[stat] = base_stat[stat];
    //apply buffs
    for (auto it = buffs[stat].begin(); it != buffs[stat].end(); ++it)
    {
      current_stat[stat] += (*it)->get();
    }
  }

  semaphore -= 1;
  return current_stat[stat];
}

void Entity::damage(float amount)
{
  damaged_signal(amount);
  base_stat[Stats::CurrentHealth] -= amount;
}
void Entity::heal(float amount)
{
  healed_signal(amount);
  base_stat[Stats::CurrentHealth] += amount;
}
void Entity::burn_mana(float amount)
{
  mana_burned_signal(amount);
  base_stat[Stats::CurrentMana] -= amount;
}
void Entity::recover_mana(float amount)
{
  mana_recovered_signal(amount);
  base_stat[Stats::CurrentMana] += amount;
}