#include "entity.h"
#include "logger.h"

void (*actions[Actions::total])(Effect &, Entity &, Entity &, double &) = 
{
NoEffect,
Damage,
Heal,
RecoverMana,
SpendMana,
DamageModifier,
HealModifier,
RecoverManaModifier,
SpendManaModifier
};

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
    if (current_hp > max_hp)
    {
      current_hp = max_hp;
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
    if (current_mana > max_mana)
    {
      current_mana = max_mana;
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
    if (buffs[i].self_activator == none && buffs[i].remote_activator == none)
    {
      resetDeltas();
      //apply buff
      double zero = 0;
      //the amount is a per second, but could be a decimal so for the last second it might only be a partial damage
      double time_scale = dt;
      if (buffs[i].duration - dt < 0)
      {
        time_scale = buffs[i].duration;
      }
      //multiply by dt because the amount is a per second amount
      double amount = get_amount(buffs[i], *others[i], *this, zero) * time_scale;
      actions[buffs[i].action](buffs[i], *this, *this, amount);
      updateDeltas();
    }
    //check if buff has expired
    buffs[i].duration -= dt;
    if (buffs[i].duration <= 0)
    {
      remove_list.push_back(i);
    }
  }
  //remove the expired buffs
  for (int i = remove_list.size() - 1; i >= 0; --i)
  {
    buffs.erase(buffs.begin()+remove_list[i]);
    others.erase(remove_list[i]);
  }
}

void ApplyEffect(Effect &effect, Entity &caster, Entity &target)
{
  //if the effect does not have an activator then its not a buff
  if (effect.self_activator == none && effect.remote_activator == none)
  {
    double zero = 0;
    double amount;
    if (effect.self_cast)
    {
      amount = get_amount(effect, caster, caster, zero);
      actions[effect.action](effect, caster, caster, amount);
      //if the effect is over time add it as a buff
      if (effect.duration > 0)
      {
        caster.buffs.push_back(effect);
        caster.others[caster.buffs.size()-1] = &caster;
      }
    }
    else
    {
      amount = get_amount(effect, caster, target, zero);      
      actions[effect.action](effect, caster, target, amount);
      //if the effect is over time add it as a buff
      if (effect.duration > 0)
      {
        target.buffs.push_back(effect);
        target.others[target.buffs.size()-1] = &target;
      }
    }

  }
  else
  {
    LOG("Addding new buff to " << &target);
    //add the buff to the target, self cast applies to the buf
    target.buffs.push_back(effect);
    target.current_buffs.push_back(effect);
  }
}

//todo this could have multiple targets depending on what kind of spell it was
void UseSpell(Spell &spell, Entity &caster, Entity &target)
{
  if (caster.current_mana < spell.mana_cost)
  {
    return;
  }
  caster.resetDeltas();
  target.resetDeltas();
  //spen the mana for the spell
  double amount = spell.mana_cost;
  Effect mana_cost;
	mana_cost.self_activator = none;
	mana_cost.remote_activator = none;
	mana_cost.action = spend_mana;
	mana_cost.scalar = none;
	mana_cost.value = amount;
	mana_cost.duration = 0;
	mana_cost.time = 0;
	mana_cost.self_cast = true;
  actions[Actions::spend_mana](mana_cost, caster, caster, amount);

  ApplyEffect(spell.effect1, caster, target);
  ApplyEffect(spell.effect2, caster, target);

  caster.updateDeltas();
  //dont update the the same entity twice if the caster is targeting itself
  if (&caster != &target)
  {
    target.updateDeltas();
  }
}

double get_amount(Effect &effect, Entity &caster, Entity &target, double &activation_amount)
{
  double ret = effect.value;
  if (effect.scalar != Actions::none)
  {
    if (effect.scalar == effect.self_activator || effect.scalar == effect.remote_activator)
    {
      ret *= activation_amount;
    }
    else
    {
      //scalar is a stat, and that not supported yet ):
      (void)(caster);
      (void)(target);
    }
  }
  return ret;
}

void CheckBuffs(Effect &effect, Entity &caster, Entity &target, double &amount)
{
  for (unsigned i = 0; i < target.current_buffs.size(); ++i)
  {
    //get the buff
    Effect buf = target.current_buffs[i];
    if (buf.self_activator == effect.action)
    {
      //remove the buff
      target.current_buffs.erase(target.current_buffs.begin()+i);
      //apply the buff, the target is now the caster
      if (buf.self_cast)
      {
        actions[buf.action](buf, target, target, amount);
      }
      else
      {
        actions[buf.action](buf, target, caster, amount);
      }
      //add the buff back on
      target.current_buffs.insert(target.current_buffs.begin()+i, buf);
    }
  }
}

double EffectHelper(Effect &effect, Entity &caster, Entity &target, double &activation_amount)
{
  double amount = get_amount(effect, caster, target, activation_amount);

  CheckBuffs(effect, caster, target, amount);

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
  double amount = effect.value;
  //check if its a percent modifier
  if (effect.scalar == Actions::damage_modifier)
  {
    amount *= damage;
  }
  //need check for stat scaling
  
  //send the reduction through the buffs(looking for)
  CheckBuffs(effect, caster, target, amount);

  //change the damage
  damage += amount;
  if (damage < 0)
  {
    damage = 0;
  }
}

void HealModifier(Effect &effect, Entity &caster, Entity &target, double &heal)
{
  double amount = effect.value;
  //check if its a percent modifier
  if (effect.scalar == Actions::heal_modifier)
  {
    amount *= heal;
  }
  //need check for stat scaling
  
  //send the reduction through the buffs(looking for)
  CheckBuffs(effect, caster, target, amount);

  //change the heal
  heal += amount;
  if (heal < 0)
  {
    heal = 0;
  }
}
void RecoverManaModifier(Effect &effect, Entity &caster, Entity &target, double &mana)
{
  double amount = effect.value;
  //check if its a percent modifier
  if (effect.scalar == Actions::recover_mana_modifier)
  {
    amount *= mana;
  }
  //need check for stat scaling
  
  //send the reduction through the buffs(looking for)
  CheckBuffs(effect, caster, target, amount);

  //change the mana
  mana += amount;
  if (mana < 0)
  {
    mana = 0;
  }
}
void SpendManaModifier(Effect &effect, Entity &caster, Entity &target, double &mana)
{
  double amount = effect.value;
  //check if its a percent modifier
  if (effect.scalar == Actions::spend_mana_modifier)
  {
    amount *= mana;
  }
  //need check for stat scaling
  
  //send the reduction through the buffs(looking for)
  CheckBuffs(effect, caster, target, amount);

  //change the mana
  mana -= amount;
  if (mana < 0)
  {
    mana = 0;
  }
}
