#include "spell_generator.h"
#include <cmath>
#include <iostream>

float short_range = 2;
float long_range = 15;
float short_cast = 0.5;
float long_cast = 15;
float cool_down = 300;
float average_score = 100;
float max_duration = 120;
float min_duration = 5;

int CreateSpell(Spell &spell, int desired_score)
{
  //zero out spell values
  spell = Spell();

  int saved = desired_score;
  double percentage;
  //visual completly random
  spell.visual = rand();//this value will be modded on the client by the total number of visuals

  //range
  percentage = rand() / (double)(RAND_MAX);
  spell.range = (long_range-short_range)*percentage+short_range;
  saved *= std::max((1-percentage)*2, 0.5);

  //cooldown
  double desired_cooldown = cool_down;
  if (desired_score < cool_down || rand()%10)
  {
    desired_cooldown = desired_score;
  }
  spell.cool_down = rand() % (int)desired_cooldown;
  if (rand()%desired_score < average_score)
  {
    //for spells with scores closer to average make the cooldown shorter
    percentage = rand() / (double)(RAND_MAX);
    spell.cool_down *= percentage;
  }
  saved *= std::sqrt(spell.cool_down) / 2;

  //figure out what kind of cc the spell should do
  percentage = rand() / (double)(RAND_MAX);
  //the smaller saved is(i.e. the stronger the current buff) the less likely to hav cc
  if (percentage < saved / (double)(saved+desired_score))
  {
    for (unsigned i = CrowdControl::no_cc+1; i < CrowdControl::num_cc; ++i)
    {
      percentage = rand() / (double)(RAND_MAX);
      //now the chance is increased if the save is higher to increase the chance of getting a weaker cc
      if (percentage  > saved / (double)(desired_score + saved))
      {
        spell.cc = i;
        //random the duration(make it more likely to be smaller the stronger the cc is is)
        percentage = rand() / (double)(RAND_MAX);
        for (unsigned j = 0; j < rand() % (CrowdControl::num_cc - i); ++j)
        {
          percentage += rand() / (double)(RAND_MAX);
        }
        spell.cc_duration = rand()%10;
        spell.cc_duration *= percentage;
        if (rand()%(CrowdControl::num_cc - i))
        {
          spell.cc_duration += rand() % 15;
        }
        if (spell.cc_duration < 5)
          spell.cc_duration = 5;

        saved *= 1-spell.cc / CrowdControl::num_cc;
        saved /= spell.cc_duration;
        break;
      }
    }
  }

  //cast time
  int loops = rand()%(int)long_cast * 2;
  if (saved > desired_score)
  {
    loops = rand()%(int)long_cast;
  }
  for (int i = 0; i < loops; ++i)
  {
    spell.cast_time += rand() / (double)(RAND_MAX);
  }
  if (spell.cast_time < short_cast)
    spell.cast_time = short_cast;

  //min cast time the score is 0.05(0.5s) - 0.5(1sec) - 3(2sec) - 22(5 sec)
  saved *= (spell.cast_time - short_cast+0.1);

  percentage = rand() / (double)(RAND_MAX);
  int score = CreateEffect(spell.effect1, saved * (percentage));
  score += CreateEffect(spell.effect2, saved-score);
  score *= desired_score / (double)(saved+1);
  
  percentage = rand() / (double)(RAND_MAX);
  if (score > desired_score)
  {
    //make the mana cost high
    loops = score/(double)desired_score * 2;
    for (int i = 0; i < loops; ++i)
    {
      percentage += rand() / (double)(RAND_MAX);
    }
    //percantage should be about score/desired_score
    spell.mana_cost = score * percentage;
    score /= percentage;
  }
  else
  {
    //make the mana cost low
    loops = desired_score/(double)score * 2;
    for (int i = 0; i < loops; ++i)
    {
      percentage += rand() / (double)(RAND_MAX);
    }
    //percantage should be about desired_score/score
    spell.mana_cost = score / percentage / 2.0;
    score *= percentage;
  }
  //prevent insane mana costs
  spell.mana_cost = std::min(spell.mana_cost, (score + desired_score)*2.0f);
  //mana cost should not go below zero
  spell.mana_cost = std::max(spell.mana_cost, 0.0f);
  
  return score;
}

//effect with no activators
static int CreateActiveEffect(Effect &effect, int desired_score)
{
  int score = 0;
  int saved = desired_score;
  //damage, heal, recovermana, spend mana
  effect.action = rand()%(Actions::spend_mana-1)+1;

  //is it selfcast
  double percentage = rand() / (double)(RAND_MAX);
  if (percentage > 0.8)
  {
    effect.target_type = TargetType::buffer;
    //if we are hurting ourselfs then the score is much less
    if (effect.action == Actions::damage || effect.action == Actions::spend_mana)
    {
      score -= saved / 2;
    }
  }
  else
  {
    effect.target_type = TargetType::buffed;
  }

  //is it over time?
  percentage = rand() / (double)(RAND_MAX);
  if (percentage > 0.8)
  {
    effect.duration = rand() % (int)(max_duration-min_duration+0.5) + min_duration;
    effect.duration *= rand() / (double)(RAND_MAX);
    effect.duration += 1;//should be at least one second
    saved /= effect.duration;
    //the fact that it is an over time spell makes it stronger
    saved /= 2;
  }

  //scalar
  percentage = rand() / (double)(RAND_MAX);
  if (percentage > 0.5)
  {
    
    effect.scalar = rand()%(Entity::num_stats*2) + Actions::total_actions;
    //when scaling with the opponent stats that is stronger than your own so make the score bigger
    if (effect.scalar > Actions::total_actions+Entity::num_stats)
    {
      saved /= 2;
    }
    if (saved > average_score)
    {
      //above average spell
      int loops = saved / average_score * 2;
      percentage = rand() / (double)(RAND_MAX);
      for (int i = 0; i < loops; ++i)
      {
        percentage += rand() / (double)(RAND_MAX);
      }
      //0.25 is %25 that sounds pretty average
      effect.value = 0.25 * sqrt(percentage);
      if (effect.value < 0.01)
      {
        effect.value = 0.01;
      }
      score += percentage * average_score;
    }
    else
    {
      //below average spell
      int loops = average_score / (saved+1) * 2;
      percentage = rand() / (double)(RAND_MAX);
      for (int i = 0; i < loops; ++i)
      {
        percentage += rand() / (double)(RAND_MAX);
      }
      //0.25 is %25 that sounds pretty average
      effect.value = 0.25 / sqrt(percentage);
      if (effect.value < 0.01)
      {
        effect.value = 0.01;
      }
      score += average_score / percentage;
    }
  }
  else
  {
    //flat value
    percentage = 0;
    for (unsigned i = 0; i < 10; ++i)
    {
      percentage += rand() / (double)(RAND_MAX);
    }
    percentage /= 5;
    //do at least 1
    effect.value = std::max(saved * percentage, 1.0);
    score += effect.value;
  }
  return score * desired_score/((double)saved+1);
}

//effect that applies a buff
static int CreateBuffEffect(Effect &effect, int desired_score)
{
  int score = 0;
  int saved = desired_score;

  //who the buff applies to
  double percentage = rand() / (double)(RAND_MAX);
  if (percentage > 0.8)
  {
    effect.target_type = TargetType::buffer;
  }
  else if(percentage > 0.5)
  {
    effect.target_type = TargetType::caster;
  }
  else
  {
    effect.target_type = TargetType::buffed;
  }
  
  //damage, heal, recovermana, spend mana, modifiers
  effect.action = rand()%(Actions::spend_mana_modifier-1)+1;
  do
  {
    effect.self_activator = rand()%(Actions::total_actions+Entity::num_stats*2-1)+1;
  }
  while (effect.self_activator >= Actions::damage_modifier && effect.self_activator <= Actions::spend_mana_modifier);
  //if the action is a modifier the activator has to be what is being modified
  if (effect.self_activator < Actions::total_actions && effect.action >= Actions::damage_modifier && effect.action <= Actions::spend_mana_modifier)
  {
    effect.self_activator = effect.action - Actions::damage_modifier + Actions::damage;
  }
  if (effect.self_activator >= Actions::total_actions)
  {
    effect.action = Actions::none;
  }
  //save a temp so we dont have to check in the future if we are self or remote
  int activator = effect.self_activator;
  //is the buff self or remote 25/75 split
  if (rand()%4)
  {
    effect.remote_activator = effect.self_activator;
    effect.self_activator = 0;
  }

  //how long does the buff last
  effect.duration = rand() % (int)(max_duration-min_duration+0.5) + min_duration;
  effect.duration *= rand() / (double)(RAND_MAX);
  if (effect.duration < 5)
  {
    effect.duration = 5;
  }
  saved /= effect.duration;

  //scalar
  percentage = rand() / (double)(RAND_MAX);
  if (percentage > 0.6)
  {
    effect.scalar = rand()%(Entity::num_stats*2) + Actions::total_actions;
    //check if it should scale with the activator(i.e. on heal do something with the heal amount)
    if (rand()%2 && activator <= Actions::spend_mana)
    {
      effect.scalar = activator;
    }
    //when scaling with the opponent stats that is stronger than your own so make the score bigger
    if (effect.scalar > Actions::total_actions + Entity::num_stats)
    {
      saved /= 2;
    }
    if (saved > average_score)
    {
      //above average spell
      int loops = saved / average_score * 2;
      percentage = rand() / (double)(RAND_MAX);
      for (int i = 0; i < loops; ++i)
      {
        percentage += rand() / (double)(RAND_MAX);
      }
      //0.1 is %25 that sounds pretty average
      effect.value = 0.25 * sqrt(percentage);
      score += percentage * average_score;
    }
    else
    {
      //below average spell
      int loops = average_score / (saved+1) * 2;
      percentage = rand() / (double)(RAND_MAX);
      for (int i = 0; i < loops; ++i)
      {
        percentage += rand() / (double)(RAND_MAX);
      }
      //0.1 is %25 that sounds pretty average
      effect.value = 0.25 / sqrt(percentage);
      score += average_score / percentage;
    }
  }
  else
  {
    //flat value
    percentage = 0;
    for (int i = 0; i < 10; ++i)
    {
      percentage += rand() / (double)(RAND_MAX);
    }
    percentage /= 5;
    //do at least 1
    effect.value = std::max(saved * percentage, 1.0);
    score += effect.value;
  }
  //if the effect is a modifier, or a scalar then the value can be negative(debuf)
  if (effect.action >= Actions::damage_modifier || effect.action == Actions::none)
  {
    if (rand()%2)
    {
      effect.value *= -1;
    }
    //the target type has to be buffed, other ones dont make sense
    effect.target_type = TargetType::buffed;
  }
  return score * desired_score/((double)saved+1);
}

int CreateEffect(Effect &effect, int desired_score)
{
  std::cout << "desired effect score = " << desired_score << std::endl;
  if (rand()%10 > 4)
  {
    return CreateActiveEffect(effect, desired_score);
  }
  return CreateBuffEffect(effect, desired_score);  
}

std::string CreateDescription(Spell &spell)
{
  std::string ret;
  if (spell.cc != CrowdControl::no_cc)
  {
    switch(spell.cc)
    {
      case CrowdControl::little_slow:
        ret = "Slow target by 33%";
        break;
      case CrowdControl::big_slow:
        ret = "Slow target by 66%";
        break;
      case CrowdControl::taunt:
        ret = "Make all spells cast by the target hit you instead of the desired target";
        break;
      case CrowdControl::invert_walk:
        ret = "Makes target walk backwards";
        break;
      case CrowdControl::sleeping:
        ret = "Puts target to sleep(will wake up when damaged)";
        break;
      case CrowdControl::silence:
        ret = "Silences target canceling the spell they are casting and preventing them from casting spells";
        break;
      case CrowdControl::root:
        ret = "Root target";
        break;
      case CrowdControl::random_target:
        ret = "Makes the target target randomly";
        break;
      case CrowdControl::stun:
        ret = "Stun target";
        break;
      case CrowdControl::target:
        ret = "Make everyone target the target";
        break;
      case CrowdControl::cleanse:
        ret = "Remove all crowd control on target and prevents target from being crowed controlled";
        break;
    }
    ret += " for " + std::to_string(spell.cc_duration) + " seconds.";
  }
  ret += CreateDescription(spell.effect1);
  ret += ". Then ";
  ret += CreateDescription(spell.effect2);
  return ret;
}

static std::string CreateAmountDescription(Effect &effect, bool is_spell)
{
  std::string me = "your";
  std::string target = "targets";
  if (!is_spell)
  {
    me = "the one that applied the effects";
    target = "your";
  }
  std::string amount_desc;
  if (effect.scalar)
  {
    amount_desc = " " + std::to_string(std::abs(effect.value) * 100) + "%";
    switch(effect.scalar)
    {
      case Actions::damage:
        amount_desc += " of damage taken";
        break;
      case Actions::heal:
        amount_desc += " of amount healed";
        break;
      case Actions::recover_mana:
        amount_desc += " of mana recovered";
        break;
      case Actions::spend_mana:
        amount_desc += " of mana lost";
        break;
      case Actions::total_actions + Entity::current_hp:
        amount_desc += " of "+me+" current hp";
        break;
      case Actions::total_actions + Entity::max_hp:
        amount_desc += " of "+me+" max hp";
        break;
      case Actions::total_actions + Entity::num_stats + Entity::current_hp:
        amount_desc += " of "+target+" current hp";
        break;
      case Actions::total_actions + Entity::num_stats + Entity::max_hp:
        amount_desc += " of "+target+" max hp";
        break;
      case Actions::total_actions + Entity::current_mana:
        amount_desc += " of "+me+" current mana";
        break;
      case Actions::total_actions + Entity::max_mana:
        amount_desc += " of "+me+" max mana";
        break;
      case Actions::total_actions + Entity::num_stats+ Entity::current_mana:
        amount_desc += " of "+target+" current mana";
        break;
      case Actions::total_actions + Entity::num_stats+ Entity::max_mana:
        amount_desc += " of "+target+" max mana";
        break;
      case Actions::total_actions + Entity::strength:
        amount_desc += " of "+me+" strength";
        break;
      case Actions::total_actions + Entity::num_stats+ Entity::strength:
        amount_desc += " of "+target+" strength";
        break;
      case Actions::total_actions + Entity::intelligence:
        amount_desc += " of "+me+" intelligence";
        break;
      case Actions::total_actions + Entity::num_stats+ Entity::intelligence:
        amount_desc += " of "+target+" intelligence";
        break;
      case Actions::total_actions + Entity::armor:
        amount_desc += " of "+me+" armor";
        break;
      case Actions::total_actions + Entity::num_stats+ Entity::armor:
        amount_desc += " of "+target+" armor";
        break;
      case Actions::total_actions + Entity::defense:
        amount_desc += " of "+me+" defense";
        break;
      case Actions::total_actions + Entity::num_stats+ Entity::defense:
        amount_desc += " of "+target+" defense";
        break;
      case Actions::total_actions + Entity::cdr:
        amount_desc += " of "+me+" cooldown reduction";
        break;
      case Actions::total_actions + Entity::num_stats+ Entity::cdr:
        amount_desc += " of "+target+" cooldown reduction";
        break;
      case Actions::total_actions + Entity::life_steal:
        amount_desc += " of "+me+" life steal";
        break;
      case Actions::total_actions + Entity::num_stats+ Entity::life_steal:
        amount_desc += " of "+target+" life steal";
        break;
    }
  }
  else
  {
    amount_desc = " " + std::to_string(abs(effect.value));
  }
  return amount_desc;
}

static std::string CreateActivationDescription(Effect &effect, std::string direction, std::string amount_desc, bool is_spell)
{
  std::string activation_desc;
  if (is_spell)
  {
    activation_desc = "apply an effect on the target so that when the target";
  }
  else
  {
    //for when the buff is applied already, description about how the buff is effecting you
    activation_desc = "when the you";
  }
  
  int activator = effect.self_activator;
  if (effect.remote_activator)
  {
    activator = effect.remote_activator;
  }
  if (activator >= Actions::total_actions)
  {
    if (is_spell)
    {
      activation_desc = "the targets";
    }
    else
    {
      activation_desc = "your";
    }
  }
  if (effect.self_activator != Actions::none)
  {
    switch(activator)
    {
      case Actions::damage:
        activation_desc += " takes damage";
        break;
      case Actions::heal:
        activation_desc += " recovers health";
        break;
      case Actions::recover_mana:
        activation_desc += " recovers mana";
        break;
      case Actions::spend_mana:
        activation_desc += " loses mana";
        break;
      case Actions::total_actions + Entity::current_hp:
      case Actions::total_actions + Entity::max_hp:
      case Actions::total_actions + Entity::num_stats + Entity::current_hp:
      case Actions::total_actions + Entity::num_stats + Entity::max_hp:
        activation_desc += " max hp is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::current_mana:
      case Actions::total_actions + Entity::max_mana:
      case Actions::total_actions + Entity::num_stats + Entity::current_mana:
      case Actions::total_actions + Entity::num_stats + Entity::max_mana:
        activation_desc += " max mana is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::strength:
      case Actions::total_actions + Entity::num_stats + Entity::strength:
        activation_desc += " strength is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::intelligence:
      case Actions::total_actions + Entity::num_stats + Entity::intelligence:
        activation_desc += " intelegence is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::armor:
      case Actions::total_actions + Entity::num_stats + Entity::armor:
        activation_desc += " armor is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::defense:
      case Actions::total_actions + Entity::num_stats + Entity::defense:
        activation_desc += " defense is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::cdr:
      case Actions::total_actions + Entity::num_stats + Entity::cdr:
        activation_desc += " cooldown reduction is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::life_steal:
      case Actions::total_actions + Entity::num_stats + Entity::life_steal:
        activation_desc += " life steal is " + direction + "d by"  + amount_desc;
        break;
    }
  }
  else if (effect.remote_activator != Actions::none)
  {
    switch(activator)
    {
      case Actions::damage:
        activation_desc += " deals damage";
        break;
      case Actions::heal:
        activation_desc += " heals someone";
        break;
      case Actions::recover_mana:
        activation_desc += " recovers someones mana";
        break;
      case Actions::spend_mana:
        activation_desc += " drains someones mana";
        break;
      case Actions::total_actions + Entity::current_hp:
      case Actions::total_actions + Entity::max_hp:
      case Actions::total_actions + Entity::num_stats + Entity::current_hp:
      case Actions::total_actions + Entity::num_stats + Entity::max_hp:
        activation_desc += " max hp is " + direction + "d by" +amount_desc;
        break;
      case Actions::total_actions + Entity::current_mana:
      case Actions::total_actions + Entity::max_mana:
      case Actions::total_actions + Entity::num_stats + Entity::current_mana:
      case Actions::total_actions + Entity::num_stats + Entity::max_mana:
        activation_desc += " max mana is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::strength:
      case Actions::total_actions + Entity::num_stats + Entity::strength:
        activation_desc += " strength is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::intelligence:
      case Actions::total_actions + Entity::num_stats + Entity::intelligence:
        activation_desc += " intelegence is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::armor:
      case Actions::total_actions + Entity::num_stats + Entity::armor:
        activation_desc += " armor is " + direction + "d by" + amount_desc;
        break;
      case Actions::total_actions + Entity::defense:
      case Actions::total_actions + Entity::num_stats + Entity::defense:
        activation_desc += " defense is " + direction + "d by" + amount_desc;
        break;
      case Actions::total_actions + Entity::cdr:
      case Actions::total_actions + Entity::num_stats + Entity::cdr:
        activation_desc += " cooldown reduction is " + direction + "d by"  + amount_desc;
        break;
      case Actions::total_actions + Entity::life_steal:
      case Actions::total_actions + Entity::num_stats + Entity::life_steal:
        activation_desc += " life steal is " + direction + "d by"  + amount_desc;
        break;
    }
  }
  else
  {
    //no activation return empty
    return "";
  }
  
  return activation_desc;
} 

static std::string CreateTargetDescription(Effect &effect)
{
  std::string target_desc;
  int activator = effect.self_activator;
  if (activator == Actions::none)
  {
    activator = effect.remote_activator;
  }
  if (activator == Actions::none)
  {
    switch(effect.target_type)
    {
      case TargetType::buffer:
        target_desc = "self";
        break;
      default:
        target_desc = "target";
        break;
    }
  }
  else
  {
    //buff
    switch(effect.target_type)
    {
      case TargetType::buffer:
        target_desc = "self";
        break;
      case TargetType::buffed:
        target_desc = "target";
        break;
      case TargetType::caster:
        target_desc = "the caster";
        break;
    }
  }
  return target_desc;
}

//the target types are quite different
static std::string CreateTargetBuffDescription(Effect &effect)
{
  std::string target_desc;

  switch(effect.target_type)
  {
    case TargetType::buffer:
      target_desc = "the one that applied the effect";
      break;
    case TargetType::buffed:
      target_desc = "self";
      break;
    case TargetType::caster:
      target_desc = "the caster";
      break;
  }
  return target_desc;
}

static std::string CreateActionDescription(Effect &effect, std::string target_desc, std::string amount_desc, std::string direction)
{
  std::string action_desc;
  switch(effect.action)
  {
    case Actions::damage:
      if (effect.scalar)
      { 
        action_desc = "deal damage to " + target_desc + " equal to" + amount_desc;
      }
      else
      {
        action_desc = "deal" + amount_desc + " damage to " + target_desc;
      }
      break;
    case Actions::heal:
      action_desc = "heal " + target_desc + " for" + amount_desc;
      break;
    case Actions::recover_mana:
      action_desc = "recover mana to " + target_desc + " equal to" + amount_desc;
      break;
    case Actions::spend_mana:
      if (effect.scalar)
      {
        action_desc = "drain mana from " + target_desc + " equal to" + amount_desc;
      }
      else
      {
        action_desc = "drain" + amount_desc + " mana from " + target_desc;
      }
      break;
    case Actions::damage_modifier:
      action_desc = direction + " the damage by" + amount_desc;
      break;
    case Actions::heal_modifier:
      action_desc = direction + " the heal by" + amount_desc;
      break;
    case Actions::recover_mana_modifier:
      action_desc = direction + " the mana recovered by" + amount_desc;
      break;
    case Actions::spend_mana_modifier:
      action_desc = direction + " the mana lost by" + amount_desc;
      break;
  }
  return action_desc;
}

std::string CreateDescription(Effect &effect)
{
  std::string ret;
  std::string direction = "increase";
  if (effect.value < 0)
  {
    direction = "decrease";
  }

  std::string amount_desc = CreateAmountDescription(effect, true);
  std::string target_desc = CreateTargetDescription(effect);
  
  std::string activation_desc = CreateActivationDescription(effect, direction, amount_desc, true);
  if (activation_desc != "")
  {
    activation_desc += " ";
  }

  std::string action_desc = CreateActionDescription(effect, target_desc, amount_desc, direction);

  std::string duration_desc;
  if (effect.duration > 0)
  {
    if (effect.self_activator == Actions::none && effect.remote_activator == Actions::none)
    {
      duration_desc = " per second for " + std::to_string(effect.duration) + " seconds";
    }
    else
    {
      duration_desc = " effect lasts for " + std::to_string(effect.duration) + " seconds";
    }
  }
  ret += activation_desc + action_desc + duration_desc; 
  return ret;
}

std::string CreateBuffDescription(Effect &effect)
{
  std::string ret;
  std::string direction = "increase";
  if (effect.value < 0)
  {
    direction = "decrease";
  }

  std::string amount_desc = CreateAmountDescription(effect, false);
  std::string target_desc = CreateTargetBuffDescription(effect);
  
  std::string activation_desc = CreateActivationDescription(effect, direction, amount_desc, false);
  if (activation_desc != "")
  {
    activation_desc += " ";
  }

  std::string action_desc = CreateActionDescription(effect, target_desc, amount_desc, direction);

  //duration will be seperate for these descriptions
  ret += activation_desc + action_desc; 
  return ret;
}