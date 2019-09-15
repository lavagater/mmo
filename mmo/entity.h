/**
 * @brief Enitiys with stats and applying spells to the enitities to change the stats. designed for top real time mmo with random spells
 */
#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <unordered_map>
#include <memory>
#include "signals.h"
#include "entity_actions.h"

class Effect
{
public:
  int self_activator=0;//activator for when something happens to me
  int remote_activator=0;//activator for when i do something to someone
  int action=0;
  int scalar=0;
  double value=0;
  double duration=0;
  int target_type=0;
};

class Spell
{
public:
  short visual=0; //this is the target type and the art
  char cc=0;//what type of crowd control does this spell have
  double cc_duration=0;//the duration of said crowd control
  Effect effect1;
  Effect effect2;
  double mana_cost=0;
  double cool_down=0;
  double cast_time=0;
  double range=0;//does not effect anything internally
};

class Entity
{
public:
  ~Entity();
  std::vector<std::pair<Effect, Entity*> > buffs;
  std::vector<std::pair<Effect, Entity*> > current_buffs;
  //todo cant use a vector here...
  std::vector<std::shared_ptr<Entity>> fake_entities;

  //stats
  double max_hp;
  double current_hp;
  double max_mana;
  double current_mana;
  double strength;
  double intelligence;
  double defense;
  double armour;
  double cool_down_reduction;
  double life_steal;

  //deltas
  double total_damage_taken;
  double total_healing_recieved;
  double total_mana_spent;
  double total_mana_recovered;

  //signals, these are signalled during updateDeltas and tell exterior components that this entity has had something happen to it
  Signals<double> damage;
  Signals<double> heal;
  Signals<double> spend_mana;
  Signals<double> recover_mana;

  //signal that this entity has been destroyed, not killed like hp = 0, but the objects destructor has been called and
  //this object is invalid
  Signals<Entity*, std::shared_ptr<Entity>> destroyed;
  std::unordered_map<Entity*, Connection> destroyed_connections;

  //should get called about once a second, applies things like over time effects and removes buffs when they expire
  void Update(double dt);

  //readys the entity to apply an effect and sets all the deltas to 0
  void resetDeltas();
  //checks if any deltas are non zero and does the change to the stats andsignals that the change has happened
  void updateDeltas();

  void EntityDestroyed(Entity *entity, std::shared_ptr<Entity> fake);
};

//helper functon that gets called by use spell
void ApplyEffect(Effect &effect, Entity &caster, Entity &target);

void UseSpell(Spell &spell, Entity &caster, Entity &target);

//helper that gets an effects amount
double get_amount(Effect &effect, Entity &caster, Entity &target, double &activation_amount);
double get_stat(int stat, Entity &entity);

//helper that loops through all the buffs and applies them
void CheckBuffs(int action, Entity &caster, Entity &target, double &amount);

//helper function that does the generic stuff that happens in most the effect function
//specificly the ones that have an activation_amount that scales instead of ones that change the activationm amount
double EffectHelper(Effect &effect, Entity &caster, Entity &target, double &activation_amount);
void Damage(Effect &effect, Entity &caster, Entity &target, double &activation_amount);
void Heal(Effect &effect, Entity &caster, Entity &target, double &activation_amount);
void RecoverMana(Effect &effect, Entity &caster, Entity &target, double &activation_amount);
void SpendMana(Effect &effect, Entity &caster, Entity &target, double &activation_amount);
void NoEffect(Effect &effect, Entity &caster, Entity &target, double &activation_amount);

//modifiers change the the incoming amount (i.e. reduce the damage taken)
void DamageModifier(Effect &effect, Entity &caster, Entity &target, double &damage);
void HealModifier(Effect &effect, Entity &caster, Entity &target, double &heal);
void RecoverManaModifier(Effect &effect, Entity &caster, Entity &target, double &mana);
void SpendManaModifier(Effect &effect, Entity &caster, Entity &target, double &mana);

//array of all the different actions that can happen to an entity
extern void (*actions[Actions::total_actions])(Effect &, Entity &, Entity &, double &);

#endif