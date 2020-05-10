/**
 * @brief fire ball spell is gonna be a projectile that blows up and and burns entities in the area.
 */
#include <iostream>
#include <fireball_spell.h>
#include <game_object.h>
#include <unordered_set>
#include <transform_component.h>
#include <entity_component.h>
#include <component.h>
#include <projectile.h>
#include "spell_factory.h"
#include "spell_value_factory.h"
#include "effect_factory.h"

FireBall::~FireBall()
{
  delete damage_amount;
  if (zone)
  {
    zone->dispatcher.RemoveTimedMessage(time_out);
  }
}

void FireBall::cast(EntityComponent *target)
{
  //need to make a copy of the spell, its like an overtime effect but instead of being a buff it needs to listen for the projectile
  FireBall *temp = static_cast<FireBall*>(copy());
  //fireball will clean itself up in two minutes
  temp->time_out = zone->dispatcher.Dispatch([temp](double){delete temp;}, 120);

  temp->CreateFireball(target);
}
std::string FireBall::getDescription()
{
  //dont care about this yet
  return "shoot a fireball that explodes on impact dealing aoe damage equal to " + damage_amount->getDescription() + " and " + burn->getDescription();
}

Effect *FireBall::copy()
{
  FireBall *fire_ball = new FireBall(*this);
  fire_ball->damage_amount = damage_amount->copy();
  fire_ball->burn = static_cast<OverTimeEffect*>(burn->copy());
  return fire_ball;
}

Spell *FireBall::Create(Stream &stream, SpellData *spell_data)
{
  FireBall *ret = new FireBall();
  char type;
  stream >> type;
  ret->damage_amount = spell_data->spell_value_factories[type]->Create(spell_data, stream);
  stream >> type;
  ret->burn = static_cast<OverTimeEffect*>(spell_data->over_time_factories[type]->Create(spell_data, stream));
  return ret;
}

Stream &FireBall::ToData(Stream &stream)
{
  stream << type;
  damage_amount->ToData(stream);
  return stream;
}

void FireBall::CreateFireball(EntityComponent *target)
{
  //if we casted the spell on our self then just blow up
  if (target == caster)
  {
    ApplyEffect(caster->game_object);
    return;
  }
  
  //tell the clients this entity is casting a big boy
  char buffer[500];
  Stream stream(buffer, 500);
  stream << zone->protocol.LookUp("FireSpell") << caster->game_object->id << target->game_object->id << type << 0;
  zone->SendToAllPlayers(stream);

  //create a projectile to shoot at the baddies
  GameObject *projectile = zone->CreateGameObject();
  //add nessecary components to the fire ball
  ADDCOMP(projectile, TransformComponent)->position = GETCOMP(caster->game_object, TransformComponent)->position;
  ColliderComponent *collider = ADDCOMP(projectile, ColliderComponent);
  collider->shape = new Circle();
  //atm fireball has hard coded radius and speed, could be variable in the future or in other spells
  ((Circle*)collider->shape)->radius = 0.5;
  collider->shape->object = projectile;
  collider->collision_group = GETCOMP(caster->game_object, ColliderComponent)->collision_group;
  ProjectileComponent *porj_comp = ADDCOMP(projectile, ProjectileComponent);
  porj_comp->velocity = GETCOMP(target->game_object, TransformComponent)->position - GETCOMP(caster->game_object, TransformComponent)->position;
  if (porj_comp->velocity == Eigen::Vector2d(0, 0))
  {
    porj_comp->velocity = Eigen::Vector2d(1, 0);
  }
  porj_comp->velocity = porj_comp->velocity / porj_comp->velocity.norm() * 10;
  
  //listen for when the projectile collides with something
  collision_connection = collider->collision_started_signal.Connect([this, projectile](GameObject *other){ProjectileCollided(other, projectile);});
}

void FireBall::ProjectileCollided(GameObject *other, GameObject *projectile)
{
  //projectile wont hit itself
  if (other == caster->game_object)
  {
    return;
  }

  ApplyEffect(other);

  //destroy the projectile
  collision_connection.Disconnect();
  zone->dispatcher.Dispatch([this, projectile](){zone->RemoveGameObject(projectile);});
}

void FireBall::ApplyEffect(GameObject *hit)
{
  //send the hit message to all players
  char buffer[500];
  Stream stream(buffer, 500);
  stream << zone->protocol.LookUp("HitSpell") << caster->game_object->id << hit->id << type << 0;
  zone->SendToAllPlayers(stream);

  //deal aoe to entities in the explosion radius
  std::unordered_set<GameObject*> targets = zone->getNearby(hit, 2, GETCOMP(caster->game_object, ColliderComponent)->collision_group);
  for (auto it = targets.begin(); it != targets.end(); ++it)
  {
    EntityComponent * entity_comp = GETCOMP(*it, EntityComponent);
    if (entity_comp)
    {
      entity_comp->entity.damage(damage_amount->get());
      //apply the overtime effect
      burn->cast(entity_comp);
    }
  }
}