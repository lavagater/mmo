#ifndef ENTITY_ACTIONS
#define ENTITY_ACTIONS
//enum of all the different things a spell can do
//make sure the order of these and the action array are correct
enum Actions
{
  none,
  damage,
  heal,
  recover_mana,
  spend_mana,
  damage_modifier,
  heal_modifier,
  recover_mana_modifier,
  spend_mana_modifier,
  //after this are no longer actions but values for scalar to scale with stats
  //these will still have action functions that modify the stat(i.e. increase str by 30)
  scalars,
  scale_caster_current_hp,
  scale_caster_max_hp,
  scale_caster_current_mana,
  scale_caster_max_mana,
  scale_caster_strength,
  scale_caster_intelligence,
  scale_caster_armour,
  scale_caster_defense,
  scale_caster_cooldown_reduction,
  scale_caster_life_steal,
  scale_caster_end, //this is so that with one check I can determine if the scaled value is caster or target
  scale_target_current_hp,
  scale_target_max_hp,
  scale_target_current_mana,
  scale_target_max_mana,
  scale_target_strength,
  scale_target_intelligence,
  scale_target_armour,
  scale_target_defense,
  scale_target_cooldown_reduction,
  scale_target_life_steal,
  total_actions
};

//these are ordered in how strong they are kinda
enum CrowdControls
{
no_cc,
little_slow,//since the spell cant have how much the target is slowed by
big_slow,//slow more than little slow
taunt, //make this person only target the taunter
sleeping, //a stun but the cc ends when damage is taken
invert_walk, //make person walk oppisite the way they want
silence, //makes character unable to cast spells, also interupts current spell
root,//makes player unable to move
fear, //make person walk away from you
random_target, //makes the player target a random person instead of their target
stun, //combination or root and silence
target, //makes this character the target to all spells cast withen his range
cleanse, //removes cc and prevents future cc
num_cc
};

//the types of targets
enum TargetType
{
  buffer,
  buffed,
  caster,
  num_target_type
};

#endif