#ifndef ENTITY_ACTIONS
#define ENTITY_ACTIONS
//enum of all the different things a spell can do
//make sure the order of these and the action array are correct
class Actions
{
  public:
  enum
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
    total_actions
    //total_actions + the index into the stat array is the scale with caster
    //total_actions + num_stats + stats is scale with targets stat
  };
};

//these are ordered in how strong they are kinda
class CrowdControl
{
public:
  enum
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
};

//the types of targets
class TargetType
{
public:
  enum
  {
    buffer,
    buffed,
    caster,
    num_target_type
  };
};

#endif