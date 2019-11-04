#include <unordered_set>
#include <functional>

#include "entity.h"
#include "logger.h"

//the entities used in UseSpell, its global because im lazy...
static std::unordered_set<Entity*> involved_entities;

void (*actions[Actions::total_actions])(Effect &, Entity &, Entity &, float &) = 
{
NoEffect, //none
Damage, //damage
Heal, //heal
RecoverMana, //recover_mana
SpendMana, //spend_mana
DamageModifier, //damage_modifier
HealModifier, //heal_modifier
RecoverManaModifier, //recover_mana_modifier
SpendManaModifier //spend_mana_modifier
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
  //damaghe
  if (total_damage_taken != 0)
  {
    stats[current_hp] -= total_damage_taken;
    LOG(this << " took " << total_damage_taken << " damage");
    damage(total_damage_taken);
    //todo check for dead and add a dead signal
  }

  //heals
  stats[current_hp] += total_healing_recieved;
  //check for buffs that change hp, current hp cant be changes by buffs
  Effect hp_getter;
  hp_getter.scalar = Actions::total_actions + max_hp; //scale with my max hp
  hp_getter.value = 1;
  float buffed_hp = get_amount(hp_getter, *this, *this, stats[max_hp]);
  LOG(this << " hp =  " << current_hp << " / " << buffed_hp);
  if (stats[current_hp] > buffed_hp)
  {
    total_healing_recieved -= stats[current_hp] - buffed_hp;
    stats[current_hp] = buffed_hp;
  }
  if (total_healing_recieved != 0)
  {
    LOG(this << " healed " << total_healing_recieved << " damage");
    heal(total_healing_recieved);
  }

  //mana spent
  stats[current_mana] -= total_mana_spent;
  if (stats[current_mana] < 0)
  {
    total_mana_spent = stats[current_mana];
    stats[current_mana] = 0;
  }
  if (total_mana_spent != 0)
  {
    LOG(this << " spent " << total_mana_spent << " mana");
    spend_mana(total_mana_spent);
  }

  //mana recovered
  stats[current_mana] += total_mana_recovered;
  //check for buffs that change mana
  Effect mp_getter;
  mp_getter.scalar = Actions::total_actions + max_mana;
  mp_getter.value = 1;
  float buffed_mp = get_amount(mp_getter, *this, *this, stats[max_mana]);
  if (stats[current_mana] > buffed_mp)
  {
    total_mana_recovered -= stats[current_mana] - buffed_mp;
    stats[current_mana] = buffed_mp;
  }
  if (total_mana_recovered != 0)
  {
    LOG(this << " recovered " << total_mana_recovered << " mana");
    recover_mana(total_mana_recovered);
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
    if (buffs[i].first.self_activator == Actions::none && buffs[i].first.remote_activator == Actions::none)
    {
      resetDeltas();
      buffs[i].second->resetDeltas();
      involved_entities.clear();
      involved_entities.insert(this);
      involved_entities.insert(buffs[i].second);
      //apply buff
      float zero = 0;
      //the amount is a per second, but could be a decimal so for the last second it might only be a partial damage
      double time_scale = dt;
      if (buffs[i].first.duration - dt < 0)
      {
        time_scale = buffs[i].first.duration;
      }
      //multiply by dt because the amount is a per second amount
      float amount = get_amount(buffs[i].first, *buffs[i].second, *this, zero) * time_scale;
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
  if (effect.self_activator == Actions::none && effect.remote_activator == Actions::none)
  {
    float zero = 0;
    float amount;
    if (effect.target_type == TargetType::buffer) //self cast
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
  involved_entities.clear();
  caster.resetDeltas();
  target.resetDeltas();
  involved_entities.insert(&caster);
  involved_entities.insert(&target);
  //spend the mana for the spell
  float amount = spell.mana_cost;
  Effect mana_cost;
	mana_cost.self_activator = Actions::none;
	mana_cost.remote_activator = Actions::none;
	mana_cost.action = Actions::spend_mana;
	mana_cost.scalar = Actions::none;
	mana_cost.value = amount;
	mana_cost.duration = 0;
	mana_cost.target_type = TargetType::buffer;

  ApplyEffect(mana_cost, caster, caster);
  ApplyEffect(spell.effect1, caster, target);
  ApplyEffect(spell.effect2, caster, target);

  for (auto it = involved_entities.begin(); it != involved_entities.end(); ++it)
  {
    (*it)->updateDeltas();
  }
}

float get_stat(int stat, Entity &entity)
{
  return entity.stats[(stat - Actions::total_actions)%entity.num_stats];
}

void ModifyStat(Effect& effect, Entity &caster, Entity &target, float &stat)
{
  float amount = get_amount(effect, caster, target, stat);

  //change the stat
  stat += amount;
  if (stat < 0)
  {
    stat = 0;
  }
}

float get_amount(Effect &effect, Entity &caster, Entity &target, float &activation_amount)
{
  float ret = effect.value;
  if (effect.scalar != Actions::none)
  {
    //if the scalar is an action then scale with the activation amount
    if (effect.scalar < Actions::total_actions)
    {
      ret *= activation_amount;
    }
    else
    {
      float stat = 0;
      //if the scalar is scaling with caster stats
      if (effect.scalar < Actions::total_actions+Entity::num_stats)
      {
        stat = get_stat(effect.scalar, caster);
        //check for buffs that deal with this stat
        for (unsigned i = 0; i < caster.current_buffs.size(); ++i)
        {
          if (caster.current_buffs[i].first.self_activator == effect.scalar || caster.current_buffs[i].first.remote_activator == effect.scalar
             || caster.current_buffs[i].first.self_activator == effect.scalar+Entity::num_stats || caster.current_buffs[i].first.remote_activator == effect.scalar+Entity::num_stats)
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
          if (target.current_buffs[i].first.self_activator == effect.scalar || target.current_buffs[i].first.remote_activator == effect.scalar
            ||target.current_buffs[i].first.self_activator == effect.scalar-Entity::num_stats || target.current_buffs[i].first.remote_activator == effect.scalar-Entity::num_stats)
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

void CheckBuffs(int action, Entity &caster, Entity &target, float &amount)
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
      if (buf.target_type == TargetType::buffer)
      {
        actions[buf.action](buf, *save.second, *save.second, amount);
      }
      else if (buf.target_type == TargetType::buffed)
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
      if (buf.target_type == TargetType::buffer)
      {
        actions[buf.action](buf, *save.second, *save.second, amount);
      }
      else if (buf.target_type == TargetType::buffed)
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

float EffectHelper(Effect &effect, Entity &caster, Entity &target, float &activation_amount)
{
  float amount = get_amount(effect, caster, target, activation_amount);

  CheckBuffs(effect.action, caster, target, amount);

  return amount;
}

void Damage(Effect &effect, Entity &caster, Entity &target, float &activation_amount)
{
  float amount = EffectHelper(effect, caster, target, activation_amount);
  LOG("Entity " << &target << " amount = " << amount);
  target.total_damage_taken += amount;
}
void Heal(Effect &effect, Entity &caster, Entity &target, float &activation_amount)
{
  float amount = EffectHelper(effect, caster, target, activation_amount);
  LOG("Entity " << &target << " amount = " << amount);
  target.total_healing_recieved += amount;
}
void RecoverMana(Effect &effect, Entity &caster, Entity &target, float &activation_amount)
{
  float amount = EffectHelper(effect, caster, target, activation_amount);
  LOG("Entity " << &target << " amount = " << amount);
  target.total_mana_recovered += amount;
}
void SpendMana(Effect &effect, Entity &caster, Entity &target, float &activation_amount)
{
  float amount = EffectHelper(effect, caster, target, activation_amount);
  LOG("Entity " << &target << " amount = " << amount);
  target.total_mana_spent += amount;
}

void NoEffect(Effect &effect, Entity &caster, Entity &target, float &activation_amount)
{
  float amount = EffectHelper(effect, caster, target, activation_amount);
  LOG("Entity " << &target << " amount = " << amount);
}

void DamageModifier(Effect &effect, Entity &caster, Entity &target, float &damage)
{
  float amount = get_amount(effect, caster, target, damage);
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

void HealModifier(Effect &effect, Entity &caster, Entity &target, float &heal)
{
  float amount = get_amount(effect, caster, target, heal);
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
void RecoverManaModifier(Effect &effect, Entity &caster, Entity &target, float &mana)
{
  float amount = get_amount(effect, caster, target, mana);
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
void SpendManaModifier(Effect &effect, Entity &caster, Entity &target, float &mana)
{
  float amount = get_amount(effect, caster, target, mana);
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

unsigned SpellPacker(Spell &spell, char *data, unsigned size)
{
  //hand calculated size of spell
  if (size < 79)
  {
    LOGW("SpellPacker size to small size = " << size);
    return 0;
  }
  unsigned offset = 0;
  //1 byte char spell visual, not sorted
  *reinterpret_cast<char*>(data+offset) = spell.visual;
  offset += 1;
  //1 byte char spell type, sorted
  *reinterpret_cast<char*>(data+offset) = spell.type;
  offset += 1;
  //1 byte char spell cc type, sorted
  *reinterpret_cast<char*>(data+offset) = spell.cc;
  offset += 1;
  //4 bytes float cc duration, not sorted
  *reinterpret_cast<float*>(data+offset) = spell.cc_duration;
  offset += 4;
  //4 bytes int the self activator, sorted
  *reinterpret_cast<int*>(data+offset) = spell.effect1.self_activator;
  offset += 4;
  //4 bytes int the remote activator, sorted
  *reinterpret_cast<int*>(data+offset) = spell.effect1.remote_activator;
  offset += 4;
  //4 bytes int spell action, sorted
  *reinterpret_cast<int*>(data+offset) = spell.effect1.action;
  offset += 4;
  //4 bytes int scalar, sorted
  *reinterpret_cast<int*>(data+offset) = spell.effect1.scalar;
  offset += 4;
  //4 bytes float spell value, not sorted
  *reinterpret_cast<float*>(data+offset) = spell.effect1.value;
  offset += 4;
  //4 bytes float duration, not sorted
  *reinterpret_cast<float*>(data+offset) = spell.effect1.duration;
  offset += 4;
  //4 bytes int target type, not sorted
  *reinterpret_cast<int*>(data+offset) = spell.effect1.target_type;
  offset += 4;
  //4 bytes int the self activator, sorted
  *reinterpret_cast<int*>(data+offset) = spell.effect2.self_activator;
  offset += 4;
  //4 bytes int the remote activator, sorted
  *reinterpret_cast<int*>(data+offset) = spell.effect2.remote_activator;
  offset += 4;
  //4 bytes int spell action, sorted
  *reinterpret_cast<int*>(data+offset) = spell.effect2.action;
  offset += 4;
  //4 bytes int scalar, sorted
  *reinterpret_cast<int*>(data+offset) = spell.effect2.scalar;
  offset += 4;
  //4 bytes float spell value, not sorted
  *reinterpret_cast<float*>(data+offset) = spell.effect2.value;
  offset += 4;
  //4 bytes float duration, not sorted
  *reinterpret_cast<float*>(data+offset) = spell.effect2.duration;
  offset += 4;
  //4 bytes int target type, not sorted
  *reinterpret_cast<int*>(data+offset) = spell.effect2.target_type;
  offset += 4;
  //4 bytes float spell mana cost, sorted
  *reinterpret_cast<float*>(data+offset) = spell.mana_cost;
  offset += 4;
  //4 bytes float spell cool down, sorted
  *reinterpret_cast<float*>(data+offset) = spell.cool_down;
  offset += 4;
  //4 bytes float spell cast time, sorted
  *reinterpret_cast<float*>(data+offset) = spell.cast_time;
  offset += 4;
  //4 bytes float spell range, sorted
  *reinterpret_cast<float*>(data+offset) = spell.range;
  offset += 4;
  if (offset != 79)
  {
    LOGW("I did my math wrong or something");
  }
  LOG("Spell packed " << ToHexString(data, 79));
  return offset;
}
Spell SpellUnpacker(char *data, unsigned size)
{
  Spell ret;
  //hand calculated size of spell
  if (size < 79)
  {
    LOGW("SpellUnpacker size to small size = " << size);
    return ret;
  }
  LOG("unpacking spell " << ToHexString(data, 79));
  unsigned offset = 0;
  //1 byte char spell visual, not sorted
  ret.visual = *reinterpret_cast<char*>(data+offset);
  offset += 1;
  //1 byte char spell type, sorted
  ret.type = *reinterpret_cast<char*>(data+offset);
  offset += 1;
  //1 byte char spell cc type, sorted
  ret.cc = *reinterpret_cast<char*>(data+offset);
  offset += 1;
  //4 bytes float cc duration, not sorted
  ret.cc_duration = *reinterpret_cast<float*>(data+offset);
  offset += 4;
  //4 bytes int the self activator, sorted
  ret.effect1.self_activator = *reinterpret_cast<int*>(data+offset);
  offset += 4;
  //4 bytes int the remote activator, sorted
  ret.effect1.remote_activator = *reinterpret_cast<int*>(data+offset);
  offset += 4;
  //4 bytes int spell action, sorted
  ret.effect1.action = *reinterpret_cast<int*>(data+offset);
  offset += 4;
  //4 bytes int scalar, sorted
  ret.effect1.scalar = *reinterpret_cast<int*>(data+offset);
  offset += 4;
  //4 bytes float spell value, not sorted
  ret.effect1.value = *reinterpret_cast<float*>(data+offset);
  offset += 4;
  //4 bytes float duration, not sorted
  ret.effect1.duration = *reinterpret_cast<float*>(data+offset);
  offset += 4;
  //4 bytes int target type, not sorted
  ret.effect1.target_type = *reinterpret_cast<int*>(data+offset);
  offset += 4;
  //4 bytes int the self activator, sorted
  ret.effect2.self_activator = *reinterpret_cast<int*>(data+offset);
  offset += 4;
  //4 bytes int the remote activator, sorted
  ret.effect2.remote_activator = *reinterpret_cast<int*>(data+offset);
  offset += 4;
  //4 bytes int spell action, sorted
  ret.effect2.action = *reinterpret_cast<int*>(data+offset);
  offset += 4;
  //4 bytes int scalar, sorted
  ret.effect2.scalar = *reinterpret_cast<int*>(data+offset);
  offset += 4;
  //4 bytes float spell value, not sorted
  ret.effect2.value = *reinterpret_cast<float*>(data+offset);
  offset += 4;
  //4 bytes float duration, not sorted
  ret.effect2.duration = *reinterpret_cast<float*>(data+offset);
  offset += 4;
  //4 bytes int target type, not sorted
  ret.effect2.target_type = *reinterpret_cast<int*>(data+offset);
  offset += 4;
  //4 bytes float spell mana cost, sorted
  ret.mana_cost = *reinterpret_cast<float*>(data+offset);
  offset += 4;
  //4 bytes float spell cool down, sorted
  ret.cool_down = *reinterpret_cast<float*>(data+offset);
  offset += 4;
  //4 bytes float spell cast time, sorted
  ret.cast_time = *reinterpret_cast<float*>(data+offset);
  offset += 4;
  //4 bytes float spell range, sorted
  ret.range = *reinterpret_cast<float*>(data+offset);
  offset += 4;
  if (offset != 79)
  {
    LOGW("I did my math wrong or something");
  }
  return ret;
}