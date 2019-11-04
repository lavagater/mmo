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
  float value=0;
  float duration=0;
  int target_type=0;
};

class Spell
{
public:
  char visual=0; //this which art to use
  char type=0; //this is what kind of spell it is, instant cast, projectile, chain, explosion, ect
  char cc=0;//what type of crowd control does this spell have
  float cc_duration=0;//the duration of said crowd control
  Effect effect1;
  Effect effect2;
  float mana_cost=0;
  float cool_down=0;
  float cast_time=0;
  float range=0;//does not effect anything internally
};


//packs the spell into data to be sent to db
unsigned SpellPacker(Spell &spell, char *data, unsigned size);
//unpacks spell from data sebt from db
Spell SpellUnpacker(char *data, unsigned size);

class Entity
{
public:
  ~Entity();
  std::vector<std::pair<Effect, Entity*> > buffs;
  std::vector<std::pair<Effect, Entity*> > current_buffs;
  //todo cant use a vector here...
  std::vector<std::shared_ptr<Entity>> fake_entities;

  //stats
  const static int num_stats = 10;
  enum CommonStats
  {
    current_hp,
    max_hp,
    current_mana,
    max_mana,

    //temporary, todo move these into a seperate enum
    strength,
    intelligence,
    defense,
    armor,
    life_steal,
    cdr
  };
  float stats[num_stats];

  //deltas
  float total_damage_taken;
  float total_healing_recieved;
  float total_mana_spent;
  float total_mana_recovered;

  //signals, these are signalled during updateDeltas and tell exterior components that this entity has had something happen to it
  Signals<float> damage;
  Signals<float> heal;
  Signals<float> spend_mana;
  Signals<float> recover_mana;

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
float get_amount(Effect &effect, Entity &caster, Entity &target, float &activation_amount);
float get_stat(int stat, Entity &entity);

//helper that loops through all the buffs and applies them
void CheckBuffs(int action, Entity &caster, Entity &target, float &amount);

//helper function that does the generic stuff that happens in most the effect function
//specificly the ones that have an activation_amount that scales instead of ones that change the activationm amount
float EffectHelper(Effect &effect, Entity &caster, Entity &target, float &activation_amount);
void Damage(Effect &effect, Entity &caster, Entity &target, float &activation_amount);
void Heal(Effect &effect, Entity &caster, Entity &target, float &activation_amount);
void RecoverMana(Effect &effect, Entity &caster, Entity &target, float &activation_amount);
void SpendMana(Effect &effect, Entity &caster, Entity &target, float &activation_amount);
void NoEffect(Effect &effect, Entity &caster, Entity &target, float &activation_amount);

//modifiers change the the incoming amount (i.e. reduce the damage taken)
void DamageModifier(Effect &effect, Entity &caster, Entity &target, float &damage);
void HealModifier(Effect &effect, Entity &caster, Entity &target, float &heal);
void RecoverManaModifier(Effect &effect, Entity &caster, Entity &target, float &mana);
void SpendManaModifier(Effect &effect, Entity &caster, Entity &target, float &mana);

//array of all the different actions that can happen to an entity
extern void (*actions[Actions::total_actions])(Effect &, Entity &, Entity &, float &);

#endif