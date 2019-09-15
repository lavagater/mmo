#include <unordered_set>
#include <functional>

#include "entity.h"
#include "logger.h"

//the entities used in UseSpell, its global because im lazy...
static std::unordered_set<Entity*> involved_entities;

void (*actions[Actions::total_actions])(Effect &, Entity &, Entity &, double &) = 
{
NoEffect, //none
Damage, //damage
Heal, //heal
RecoverMana, //recover_mana
SpendMana, //spend_mana
DamageModifier, //damage_modifier
HealModifier, //heal_modifier
RecoverManaModifier, //recover_mana_modifier
SpendManaModifier, //spend_mana_modifier
NoEffect, //scalars
NoEffect, //scale_caster_current_hp
NoEffect, //scale_caster_max_hp
NoEffect, //scale_caster_current_mana
NoEffect, //scale_caster_max_mana
NoEffect, //scale_caster_strength
NoEffect, //scale_caster_intelligence
NoEffect, //scale_caster_armour
NoEffect, //scale_caster_defense
NoEffect, //scale_caster_cooldown_reduction
NoEffect, //scale_caster_life_steal
NoEffect, //scale_caster_end
NoEffect, //scale_target_current_hp
NoEffect, //scale_target_max_hp
NoEffect, //scale_target_current_mana
NoEffect, //scale_target_max_mana
NoEffect, //scale_target_strength
NoEffect, //scale_target_intelligence
NoEffect, //scale_target_armour
NoEffect, //scale_target_defense
NoEffect, //scale_target_cooldown_reduction
NoEffect //scale_target_life_steal
};

Entity::~Entity()
{
  //make sure there are people that care if i die, if not then i am probablky a copy(infinite death loop)
  if (destroyed)
  {
    //make a copy of this entity
    std::shared_ptr<Entity> fake(new Entity);
    //make the fake
    *fake.get() = *this;
    //fake will not have the signals saying it took damage/healed/etc
    fake->damage.Clear();
    fake->heal.Clear();
    fake->spend_mana.Clear();
    fake->recover_mana.Clear();
    fake->destroyed.Clear();
    //tell people who care that i have died and to use my clone
    destroyed(this, fake);
  }
}

void Entity::EntityDestroyed(Entity *entity, std::shared_ptr<Entity> fake)
{
  bool add_fake = false;
  for (unsigned i = 0; i < buffs.size(); ++i)
  {
    if (buffs[i].second == entity)
    {
      add_fake = true;
      buffs[i].second = fake.get();
    }
  }
  if (add_fake)
  {
    fake_entities.push_back(fake);
  }
}

void Entity::resetDeltas()
{
  //reset all deltas
  total_damage_taken = 0;
  total_healing_recieved = 0;
  total_mana_spent = 0;
  total_mana_recovered = 0;
  //copy the buffs
  current_buffs = buffs;
}
void Entity::updateDeltas()
{
  if (total_damage_taken != 0)
  {
    current_hp -= total_damage_taken;
    LOG(this << " took " << total_damage_taken << " damage");
    //todo check for dead and add a dead signal
  }
  if (total_healing_recieved != 0)
  {
    current_hp += total_healing_recieved;
    LOG(this << " healed " << total_healing_recieved << " damage");
    //check for buffs that change hp, current hp cant be changes by buffs
    Effect hp_getter;
    hp_getter.scalar = scale_caster_max_hp;
    hp_getter.value = 1;
    double buffed_hp = get_amount(hp_getter, *this, *this, max_hp);
    LOG(this << " hp =  " << current_hp << " / " << buffed_hp);
    if (current_hp > buffed_hp)
    {
      current_hp = buffed_hp;
    }
  }
  if (total_mana_spent != 0)
  {
    current_mana -= total_mana_spent;
    LOG(this << " spent " << total_mana_spent << " mana");
    if (current_mana < 0)
    {
      current_mana = 0;
    }
  }
  if (total_mana_recovered != 0)
  {
    current_mana += total_mana_recovered;
    LOG(this << " recovered " << total_mana_recovered << " mana");
    //check for buffs that change mana
    Effect mp_getter;
    mp_getter.scalar = scale_caster_max_mana;
    mp_getter.value = 1;
    double buffed_mp = get_amount(mp_getter, *this, *this, max_hp);
    if (current_mana > buffed_mp)
    {
      current_mana = buffed_mp;
    }
  }
}

void Entity::Update(double dt)
{
  //vector of buffs to remove
  std::vector<unsigned> remove_list;
  //loop through each buff
  for (unsigned i =0; i < buffs.size(); ++i)
  {
    //apply over time effects
    if (buffs[i].first.self_activator == none && buffs[i].first.remote_activator == none)
    {
      resetDeltas();
      buffs[i].second->resetDeltas();
      involved_entities.clear();
      involved_entities.insert(this);
      involved_entities.insert(buffs[i].second);
      //apply buff
      double zero = 0;
      //the amount is a per second, but could be a decimal so for the last second it might only be a partial damage
      double time_scale = dt;
      if (buffs[i].first.duration - dt < 0)
      {
        time_scale = buffs[i].first.duration;
      }
      //multiply by dt because the amount is a per second amount
      double amount = get_amount(buffs[i].first, *buffs[i].second, *this, zero) * time_scale;
      actions[buffs[i].first.action](buffs[i].first, *buffs[i].second, *this, amount);
      for (auto it = involved_entities.begin(); it != involved_entities.end(); ++it)
      {
        (*it)->updateDeltas();
      }
    }
    //check if buff has expired
    buffs[i].first.duration -= dt;
    if (buffs[i].first.duration <= 0)
    {
      remove_list.push_back(i);
    }
  }
  //remove the expired buffs
  for (int i = remove_list.size() - 1; i >= 0; --i)
  {
    auto buf_to_remove = buffs.begin()+remove_list[i];
    Entity *buffer_entity = buf_to_remove->second;
    buffs.erase(buf_to_remove);
    bool can_disconnect = true;
    //stop listening for the death of the buffer if there are no other buffs from that caster
    for (unsigned i = 0; i < buffs.size(); ++i)
    {
      if (buffs[i].second == buffer_entity)
      {
        can_disconnect = false;
      }
    }
    if (can_disconnect)
      destroyed_connections[buffer_entity].Disconnect();
  }
}

void ApplyEffect(Effect &effect, Entity &caster, Entity &target)
{
  //if the effect does not have an activator then its not a buff
  if (effect.self_activator == none && effect.remote_activator == none)
  {
    double zero = 0;
    double amount;
    if (effect.target_type == buffer) //self cast
    {
      amount = get_amount(effect, caster, caster, zero);
      actions[effect.action](effect, caster, caster, amount);
      //if the effect is over time add it as a buff
      if (effect.duration > 0)
      {
        caster.buffs.push_back(std::make_pair(effect, &caster));
        if (caster.destroyed_connections.find(&caster) == caster.destroyed_connections.end())
        {
          caster.destroyed_connections[&caster] = caster.destroyed.Connect(std::bind(&Entity::EntityDestroyed, &caster, std::placeholders::_1, std::placeholders::_2));
        }
      }
    }
    else //buffed and caster are both non-self cast
    {
      amount = get_amount(effect, caster, target, zero);      
      actions[effect.action](effect, caster, target, amount);
      //if the effect is over time add it as a buff
      if (effect.duration > 0)
      {
        target.buffs.push_back(std::make_pair(effect, &caster));
        if (target.destroyed_connections.find(&caster) == target.destroyed_connections.end())
        {
          target.destroyed_connections[&caster] = caster.destroyed.Connect(std::bind(&Entity::EntityDestroyed, &target, std::placeholders::_1, std::placeholders::_2));
        }
      }
    }

  }
  else
  {
    LOG("Addding new buff to " << &target);
    //add the buff to the target, self cast applies to the buf
    target.buffs.push_back(std::make_pair(effect, &caster));
    target.current_buffs.push_back(std::make_pair(effect, &caster));
    if (target.destroyed_connections.find(&caster) == target.destroyed_connections.end())
    {
      target.destroyed_connections[&caster] = caster.destroyed.Connect(std::bind(&Entity::EntityDestroyed, &target, std::placeholders::_1, std::placeholders::_2));
    }
  }
}

//todo this could have multiple targets depending on what kind of spell it was
void UseSpell(Spell &spell, Entity &caster, Entity &target)
{
  if (caster.current_mana < spell.mana_cost)
  {
    return;
  }
  involved_entities.clear();
  caster.resetDeltas();
  target.resetDeltas();
  involved_entities.insert(&caster);
  involved_entities.insert(&target);
  //spen the mana for the spell
  double amount = spell.mana_cost;
  Effect mana_cost;
	mana_cost.self_activator = none;
	mana_cost.remote_activator = none;
	mana_cost.action = spend_mana;
	mana_cost.scalar = none;
	mana_cost.value = amount;
	mana_cost.duration = 0;
	mana_cost.target_type = buffer;

  ApplyEffect(mana_cost, caster, caster);
  ApplyEffect(spell.effect1, caster, target);
  ApplyEffect(spell.effect2, caster, target);

  for (auto it = involved_entities.begin(); it != involved_entities.end(); ++it)
  {
    (*it)->updateDeltas();
  }
}

double get_stat(int stat, Entity &entity)
{
  switch(stat)
  {
    case scale_caster_current_hp:
    case scale_target_current_hp:
      return entity.current_hp;
      break;
    case scale_caster_max_hp:
    case scale_target_max_hp:
      return entity.max_hp;
      break;
    case scale_caster_current_mana:
    case scale_target_current_mana:
      return entity.current_mana;
      break;
    case scale_caster_max_mana:
    case scale_target_max_mana:
      return entity.max_mana;
      break;
    case scale_caster_strength:
    case scale_target_strength:
      return entity.strength;
      break;
    case scale_caster_intelligence:
    case scale_target_intelligence:
      return entity.intelligence;
      break;
    case scale_caster_armour:
    case scale_target_armour:
      return entity.armour;
      break;
    case scale_caster_defense:
    case scale_target_defense:
      return entity.defense;
      break;
    case scale_caster_cooldown_reduction:
    case scale_target_cooldown_reduction:
      return entity.cool_down_reduction;
      break;
    case scale_caster_life_steal:
    case scale_target_life_steal:
      return entity.cool_down_reduction;
      break;
    default:
      return 0;
  }
}

void ModifyStat(Effect& effect, Entity &caster, Entity &target, double &stat)
{
  double amount = get_amount(effect, caster, target, stat);

  //change the stat
  stat += amount;
  if (stat < 0)
  {
    stat = 0;
  }
}

double get_amount(Effect &effect, Entity &caster, Entity &target, double &activation_amount)
{
  double ret = effect.value;
  if (effect.scalar != Actions::none)
  {
    //if the scalar is an action then scale with the activation amount
    if (effect.scalar < scalars)
    {
      ret *= activation_amount;
    }
    else
    {
      double stat = 0;
      //if the scalar is scaling with caster stats
      if (effect.scalar < scale_caster_end)
      {
        stat = get_stat(effect.scalar, caster);
        //check for buffs that deal with this stat
        for (unsigned i = 0; i < caster.current_buffs.size(); ++i)
        {
          if (caster.current_buffs[i].first.self_activator == effect.scalar || caster.current_buffs[i].first.remote_activator == effect.scalar)
          {
            std::pair<Effect, Entity*> save = caster.current_buffs[i];
            //remove the buff
            caster.current_buffs.erase(caster.current_buffs.begin()+i);
            ModifyStat(save.first, caster, caster, stat);
            //add the buff back on
            caster.current_buffs.insert(caster.current_buffs.begin()+i, save);
          }
        }
        LOG("Caster " << &caster << " stat, " << effect.scalar << " = " << stat);
      }
      else
      {
        //get stat will do the conversion of scalar
        stat = get_stat(effect.scalar, target);
        //apply the buffs that effect that stat
        for (unsigned i = 0; i < target.current_buffs.size(); ++i)
        {
          if (target.current_buffs[i].first.self_activator == effect.scalar || target.current_buffs[i].first.remote_activator == effect.scalar)
          {
            std::pair<Effect, Entity*> save = target.current_buffs[i];
            //remove the buff
            target.current_buffs.erase(target.current_buffs.begin()+i);
            ModifyStat(save.first, target, target, stat);
            //add the buff back on
            target.current_buffs.insert(target.current_buffs.begin()+i, save);
          }
        }
        LOG("Target " << &target << " stat, " << effect.scalar << " = " << stat);
      }

      ret *= stat;
    }
  }
  return ret;
}

void CheckBuffs(int action, Entity &caster, Entity &target, double &amount)
{
  //check caster for buffs
  for (unsigned i = 0; i < caster.current_buffs.size(); ++i)
  {
    //get the buff
    std::pair<Effect, Entity*> save = caster.current_buffs[i];
    Effect &buf = save.first;
    //for the caster need to check the remote activator
    if (buf.remote_activator == action)
    {
      //add buffer to the involved entities
      if (involved_entities.find(save.second) == involved_entities.end())
      {
        save.second->resetDeltas();
        involved_entities.insert(save.second);
      }
      //remove the buff
      caster.current_buffs.erase(caster.current_buffs.begin()+i);
      //apply the buff, the buffer is the caster
      if (buf.target_type == buffer)
      {
        actions[buf.action](buf, *save.second, *save.second, amount);
      }
      else if (buf.target_type == buffed)
      {
        actions[buf.action](buf, *save.second, caster, amount);
      }
      else
      {
        actions[buf.action](buf, *save.second, target, amount);
      }
      //add the buff back on
      caster.current_buffs.insert(caster.current_buffs.begin()+i, save);
    }
  }
  //check target for buffs
  for (unsigned i = 0; i < target.current_buffs.size(); ++i)
  {
    //get the buff
    std::pair<Effect, Entity*> save = target.current_buffs[i];
    Effect &buf = save.first;
    if (buf.self_activator == action)
    {
      //add buffer to the involved entities
      if (involved_entities.find(save.second) == involved_entities.end())
      {
        save.second->resetDeltas();
        involved_entities.insert(save.second);
      }
      //remove the buff
      target.current_buffs.erase(target.current_buffs.begin()+i);
      //apply the buff, the buffer is the caster
      if (buf.target_type == buffer)
      {
        actions[buf.action](buf, *save.second, *save.second, amount);
      }
      else if (buf.target_type == buffed)
      {
        actions[buf.action](buf, *save.second, target, amount);
      }
      else
      {
        actions[buf.action](buf, *save.second, caster, amount);
      }
      //add the buff back on
      target.current_buffs.insert(target.current_buffs.begin()+i, save);
    }
  }
}

double EffectHelper(Effect &effect, Entity &caster, Entity &target, double &activation_amount)
{
  double amount = get_amount(effect, caster, target, activation_amount);

  CheckBuffs(effect.action, caster, target, amount);

  return amount;
}

void Damage(Effect &effect, Entity &caster, Entity &target, double &activation_amount)
{
  double amount = EffectHelper(effect, caster, target, activation_amount);
  LOG("Entity " << &target << " amount = " << amount);
  target.total_damage_taken += amount;
}
void Heal(Effect &effect, Entity &caster, Entity &target, double &activation_amount)
{
  double amount = EffectHelper(effect, caster, target, activation_amount);
  LOG("Entity " << &target << " amount = " << amount);
  target.total_healing_recieved += amount;
}
void RecoverMana(Effect &effect, Entity &caster, Entity &target, double &activation_amount)
{
  double amount = EffectHelper(effect, caster, target, activation_amount);
  LOG("Entity " << &target << " amount = " << amount);
  target.total_mana_recovered += amount;
}
void SpendMana(Effect &effect, Entity &caster, Entity &target, double &activation_amount)
{
  double amount = EffectHelper(effect, caster, target, activation_amount);
  LOG("Entity " << &target << " amount = " << amount);
  target.total_mana_spent += amount;
}

void NoEffect(Effect &effect, Entity &caster, Entity &target, double &activation_amount)
{
  double amount = EffectHelper(effect, caster, target, activation_amount);
  LOG("Entity " << &target << " amount = " << amount);
}

void DamageModifier(Effect &effect, Entity &caster, Entity &target, double &damage)
{
  double amount = get_amount(effect, caster, target, damage);
  //check if its a percent modifier
  if (effect.scalar == Actions::damage_modifier)
  {
    amount = damage * effect.value;
  }
  
  //send the reduction through the buffs(looking for)
  CheckBuffs(effect.action, caster, target, amount);

  //change the damage
  damage += amount;
  if (damage < 0)
  {
    damage = 0;
  }
}

void HealModifier(Effect &effect, Entity &caster, Entity &target, double &heal)
{
  double amount = get_amount(effect, caster, target, heal);
  //check if its a percent modifier
  if (effect.scalar == Actions::heal_modifier)
  {
    amount = heal * effect.value;
  }
  
  //send the reduction through the buffs(looking for)
  CheckBuffs(effect.action, caster, target, amount);

  //change the heal
  heal += amount;
  if (heal < 0)
  {
    heal = 0;
  }
}
void RecoverManaModifier(Effect &effect, Entity &caster, Entity &target, double &mana)
{
  double amount = get_amount(effect, caster, target, mana);
  //check if its a percent modifier
  if (effect.scalar == Actions::recover_mana_modifier)
  {
    amount = mana * effect.value;
  }
  
  //send the reduction through the buffs(looking for)
  CheckBuffs(effect.action, caster, target, amount);

  //change the mana
  mana += amount;
  if (mana < 0)
  {
    mana = 0;
  }
}
void SpendManaModifier(Effect &effect, Entity &caster, Entity &target, double &mana)
{
  double amount = get_amount(effect, caster, target, mana);
  //check if its a percent modifier
  if (effect.scalar == Actions::spend_mana_modifier)
  {
    amount = mana * effect.value;
  }
  
  
  //send the reduction through the buffs(looking for)
  CheckBuffs(effect.action, caster, target, amount);

  //change the mana
  mana -= amount;
  if (mana < 0)
  {
    mana = 0;
  }
}