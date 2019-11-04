/**
 * @brief component for players and monsters that has their stats and skills
 */
#ifndef ENTITY_COMP_H
#define ENTITY_COMP_H

#include "component.h"
#include "signals.h"
#include "entity.h"
#include <Eigen/Dense>

class EntityComponent : public Component
{
public:
  void Init();
  //call this to cast a spell
  void UseSpell(int spell, GameObject *target);
  void Interupt();
  void OnUpdate(double dt);

private:
  void OnCastSpell(EntityComponent *target);
  void onProjectileCollision(GameObject * projectile, GameObject *other, Spell spell);
  //callbacks for when my entity has taken damage, healed, ect
  void OnDamaged(double damage);
  void OnHealed(double heal);
  void OnManaRecovered(double mana);
  void OnManaDrained(double mana);
  void SendHpChange(double hp);
  void SendManaChange(double mana);

  //function to query the spell database to get the spell information for a given spell slot
  void SendSpellQuery(unsigned slot_num);
  //sets the spell for the given slot
  void SendSpellQuery(Spell &spell, unsigned slot_num);
  //database query responses
  void OnSpellDatabaseResponse(char *data, unsigned size, unsigned slot);
public:
  //all the spells the entity can cast
  std::vector<Spell> spells;
  //default spell if slot is empty
  Spell basic_attack;
  //each spell in spells has a corrisponding entry in cool_downs
  //the value in cool_downs is when the spell was last used 
  std::vector<double> cool_downs;
  //which spell is being casted right now, -1 if no spell
  int casting;
  //the last place this entity was to calculate the direction its going
  Eigen::Vector2d previous_position;
  //direction the entity is/was moving
  Eigen::Vector2d facing;
  TimedMessage cast_connection;
  std::unordered_map<GameObject *, Connection> projectiles;
  Connection update_connection;
  Connection target_died_connection;
  //connections for entity signals
  Connection entity_damaged_connection;
  Connection entity_healed_connection;
  Connection entity_recover_mana_connection;
  Connection entity_lost_mana_connection;
  //the entity
  Entity entity;
};

#endif