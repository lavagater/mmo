/**
 * @brief Makes a random spell. Also has a way of scoring the spell on how good it is(very rough estimate).
 *        There are come global values that can be set to change the limits of the spell, used globals instead of constants
 *        so the restraints can be changed at runtime but can also be used as consts by changing the defaults.
 */
#ifndef SPELL_GENERATOR_H
#define SPELL_GENERATOR_H

#include "entity.h"
#include "entity_actions.h"

/**
 * @brief Shortest range a spell can have
 */
extern float short_range;

/**
 * @brief Longest range a spell can have
 */
extern float long_range;

/**
 * @brief shortest possible cast time
 */
extern float short_cast;

/**
 * @brief longest possible cast time
 */
extern float long_cast;

/**
 * @brief longest possible cooldown
 */
extern float cool_down;

/**
 * @brief For scaling spells I need a score that is considered average.
 */
extern float average_score;

/**
 * @brief Maximum duration for spells/buffs.
 */
extern float max_duration;

/**
 * @brief Minimum duration for spells/buffs.
 */
extern float min_duration;

/**
 * @brief Creates a random spell with a score close to the desired score.
 * @return the actual score of the spell.
 */
int CreateSpell(Spell &ret, int desired_score);

/**
 * @brief Creates a random Effect with a score close to the desired score.
 * @return the actual score of the spell.
 */
int CreateEffect(Effect &ret, int desired_score);

/**
 * @brief Creates a human readable description of what the spell does.
 */
std::string CreateDescription(Spell &spell);
/**
 * @brief helper creat description that creates a huuman readable description of an individual effect
 */
std::string CreateDescription(Effect &spell);

/**
 * @brief Creates a description for an effect that has been applied to someone, or is an item passive
 */
std::string CreateBuffDescription(Effect &spell);

#endif