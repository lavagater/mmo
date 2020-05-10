/**
 * @brief The first spell to test if the spell system is working
 */

#ifndef TEST_SPELL_H
#define TEST_SPELL_H

#include <string>
#include <spell.h>
#include "entity.h"
#include "zone.h"
#include "stat_scale.h"
#include "overtime_effect.h"
#include "stream.h"
#include "player_controller_component.h"
#include "entity_component.h"

class TestSpell : public Spell
{
  public:
  TestSpell()
  {
    type = 0;
  }
  ~TestSpell(){}
  void cast(EntityComponent *target)
  {
    //send packet to all the players that the entity has fired the spell
    char buffer[500];
    Stream stream(buffer, 500);
    stream << zone->protocol.LookUp("FireSpell") << caster->game_object->id << target->game_object->id << type << 0;
    zone->SendToAllPlayers(stream);
  }
  std::string getDescription()
  {
    return "test spell";
  }
  Stream &ToData(Stream &stream)
  {
    return stream;
  }

  Effect *copy(){return this;}

  //zone for sending packets
  Zone *zone;
  EntityComponent *caster;
};

#endif