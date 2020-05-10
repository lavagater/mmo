/**
 * @brief The first spell to test if the spell system is working
 */

#ifndef FIREBALL_SPELL_H
#define FIREBALL_SPELL_H

#include <string>
#include <spell.h>
#include <collider_component.h>
#include <signals.h>
#include "entity.h"
#include "zone.h"
#include "stat_scale.h"
#include "overtime_effect.h"
#include "dispatcher.h"
#include "stream.h"

class FireBall : public Spell
{
  public:
  ~FireBall();
  void cast(EntityComponent *target);
  std::string getDescription();
  static Spell *Create(Stream &stream, SpellData *spell_data);
  Stream &ToData(Stream &stream);
  Effect *copy();
  void CreateFireball(EntityComponent *target);
  void ProjectileCollided(GameObject *other, GameObject *projectile);
  void ApplyEffect(GameObject*);

  //zone for getting nearby targets
  Zone *zone = 0;
  EntityComponent *caster = 0;
  SpellValue *damage_amount = 0;
  OverTimeEffect *burn = 0;
  Connection collision_connection;
  TimedMessage time_out;
};

#endif