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
  total_actions
};