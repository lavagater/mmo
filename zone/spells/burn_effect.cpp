#include <entity_component.h>
#include "burn_effect.h"
#include "spell_factory.h"
#include "spell_value_factory.h"
#include "player_controller_component.h"

BurnEffect::~BurnEffect()
{
  delete dps;
  if (dispatcher)
  {
    dispatcher->RemoveTimedMessage(damage_tick);
  }
}

void BurnEffect::startEffect(EntityComponent *target)
{
  //send status packet to this entity if its a player
  PlayerControllerComponent *player_controller = GETCOMP(target->game_object, PlayerControllerComponent);
  if (player_controller)
  {
    char buffer[500];
    Stream stream(buffer, 500);
    //send the status icon, duration, description
    stream << target->game_object->zone->protocol.LookUp("Status") << (char)0 << duration;
    std::string desc = "take damage every second";
    stream.write(desc.data(), desc.length());
    target->game_object->zone->SendTo(stream, player_controller->id);
  }
  damage_tick = dispatcher->Dispatch([this, target](double){tick(target);}, 1);
}

void BurnEffect::tick(EntityComponent *target)
{
  //deal the damage
  target->entity.damage(dps->get());
  //got to do it again in another second, the loop will end when the effect is deleted
  damage_tick = dispatcher->Dispatch([this, target](double){tick(target);}, 1);
}

OverTimeEffect *BurnEffect::Create(Stream &stream, SpellData *spell_data)
{
  BurnEffect *ret = new BurnEffect();
  char dps_type;
  stream >> dps_type;
  ret->dps = spell_data->spell_value_factories[dps_type]->Create(spell_data, stream);
  return ret;
}

std::string BurnEffect::getDescription()
{
  return "burns target dealing " + dps->getDescription() + " damage per second for " + std::to_string(duration) + " seconds";
}

Stream &BurnEffect::ToData(Stream &stream)
{
  stream << type;
  dps->ToData(stream);
  return stream;
}

Effect *BurnEffect::copy()
{
  BurnEffect *ret = new BurnEffect(*this);
  ret->dps = dps->copy();
  return ret;
}