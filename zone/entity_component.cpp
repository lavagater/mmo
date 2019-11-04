#include "game_object.h"
#include "zone.h"
#include "entity_component.h"
#include "debug_component.h"
#include "spell_generator.h"
#include "player_controller_component.h"
#include "database_protocol.h"
#include "transform_component.h"
#include "collider_component.h"
#include "projectile.h"
#include "logger.h"
#include "query.h"
#include <iostream>

void EntityComponent::Init()
{
  //casting must start at -1
  casting = -1;

  //setup update loop
  update_connection = game_object->zone->update_signal.Connect(std::bind(&EntityComponent::OnUpdate, this, std::placeholders::_1));

  //listen for entity signals
  entity_damaged_connection = entity.damage.Connect(std::bind(&EntityComponent::OnDamaged, this, std::placeholders::_1));
  entity_healed_connection = entity.heal.Connect(std::bind(&EntityComponent::OnHealed, this, std::placeholders::_1));
  entity_recover_mana_connection = entity.recover_mana.Connect(std::bind(&EntityComponent::OnManaRecovered, this, std::placeholders::_1));
  entity_lost_mana_connection = entity.spend_mana.Connect(std::bind(&EntityComponent::OnManaDrained, this, std::placeholders::_1));

  //make some default stats for the entity
	entity.stats[Entity::Entity::max_hp] = 100;
	entity.stats[Entity::current_hp] = 100;
	entity.stats[Entity::max_mana] = 100;
	entity.stats[Entity::current_mana] = 100;

  //todo make a basic attack spell to fill all posible slots by default
  Spell test_spell;
  test_spell.cast_time = 2;
  test_spell.cool_down = 0;
  test_spell.mana_cost = 0;
  test_spell.range = 14;
  test_spell.visual = 0;
  test_spell.type = 1;
  test_spell.effect1.action = Actions::damage;
  test_spell.effect1.target_type = TargetType::buffed;
  test_spell.effect1.value = 25;
  test_spell.effect2.action = Actions::recover_mana;
  test_spell.effect2.target_type = TargetType::buffer;
  test_spell.effect2.value = 10;

  basic_attack = test_spell;

  //add default spells for unused slots
  spells.push_back(test_spell);
  cool_downs.push_back(0);
  spells.push_back(test_spell);
  cool_downs.push_back(0);
  spells.push_back(test_spell);
  cool_downs.push_back(0);
  spells.push_back(test_spell);
  cool_downs.push_back(0);
  spells.push_back(test_spell);
  cool_downs.push_back(0);
  spells.push_back(test_spell);
  cool_downs.push_back(0);
  spells.push_back(test_spell);
  cool_downs.push_back(0);
  spells.push_back(test_spell);
  cool_downs.push_back(0);
  spells.push_back(test_spell);
  cool_downs.push_back(0);
  spells.push_back(test_spell);
  cool_downs.push_back(0);

  //temporarily set a spell in database for the poor guy
  CreateSpell(test_spell, 1000);
  SendSpellQuery(test_spell, 0);
  //ask database for all the spells
  SendSpellQuery(0);
  SendSpellQuery(1);
  SendSpellQuery(2);
  SendSpellQuery(3);
  SendSpellQuery(4);
  SendSpellQuery(5);
  SendSpellQuery(6);
  SendSpellQuery(7);
  SendSpellQuery(8);
  SendSpellQuery(9);
}

void EntityComponent::SendSpellQuery(Spell &spell, unsigned slot_num)
{
  PlayerControllerComponent *controller = GETCOMP(game_object, PlayerControllerComponent);
  //if the entity is not a player then this entities spell wont be in the database
  if (!controller)
  {
    std::cout << "not a player!" << std::endl;
    return;
  }
  char spell_data[79];
  SpellPacker(spell, spell_data, 79);
  BlobStruct spell_blob;
  spell_blob.data = &spell_data[0];
  spell_blob.size = 79;
  char query[MAXPACKETSIZE];
  int len = CreateQueryMessage(game_object->zone->protocol, -1, query, STRINGIZE(
  main(uint id, float slot_num, blob spell)
  {
    vector value = vector(id, slot_num);
    vector res = findRange(vector(22, 24), value, value);
    if (Size(res) == 0)
    {
      print("player id ", id, " has no spells creating spell\n");
      uint new_spell = create();
      print("new spell id = ", new_spell, "\n");
      set(new_spell, 22, id);
      set(new_spell, 23, char(0));
      set(new_spell, 24, slot_num);
      vector_push(res, new_spell);
    }
    setRange(res[0], 0, 21, spell);
    return;
  }
  ), controller->id, (float)slot_num, spell_blob);
  game_object->zone->stack.Send(query, len, &game_object->zone->spells_database, game_object->zone->flags[game_object->zone->spells_database]);
}

void EntityComponent::SendSpellQuery(unsigned slot_num)
{
  PlayerControllerComponent *controller = GETCOMP(game_object, PlayerControllerComponent);
  //if the entity is not a player then this entities spell wont be in the database
  if (!controller)
  {
    std::cout << "not a player!" << std::endl;
    return;
  }
  char query[MAXPACKETSIZE];
  game_object->zone->query_id += 1;
  game_object->zone->query_callbacks[game_object->zone->query_id] = std::bind(&EntityComponent::OnSpellDatabaseResponse, this, std::placeholders::_1, std::placeholders::_2, slot_num);
  int len = CreateQueryMessage(game_object->zone->protocol, game_object->zone->query_id, query, STRINGIZE(
  main(uint id, float slot_num)
  {
    vector value = vector(id, slot_num);
    vector res = findRange(vector(22, 24), value, value);
    if (Size(res) == 0)
    {
      print("player id ", id, " has no spell in slot", slot_num, "\n");
      return int(-1);
    }
    print("res size = ", Size(res), "\n");
    return getRange(res[0], 0, 21);
  }
  ), controller->id, (float)slot_num);
  game_object->zone->stack.Send(query, len, &game_object->zone->spells_database, game_object->zone->flags[game_object->zone->spells_database]);
}
void EntityComponent::OnSpellDatabaseResponse(char *data, unsigned size, unsigned slot)
{
  std::cout << "Spell response " << size << std::endl;
  if (size == sizeof(int))
  {
    //error occured, -1 no spells, -2 means no spell for this slot
    std::cout << "error = " << *reinterpret_cast<int*>(data) << std::endl;
  }
  else
  {
    spells[slot] =  SpellUnpacker(data, size);
    std::cout << "spell action = " << spells[slot].effect1.action << std::endl;
    std::cout << "spell value = " << spells[slot].effect1.value << std::endl;
  }
  
}

void EntityComponent::OnUpdate(double dt)
{
  (void)(dt);
  //update directions
  if (GETCOMP(game_object, TransformComponent)->position != previous_position)
  {
    facing = GETCOMP(game_object, TransformComponent)->position - previous_position;
    previous_position = GETCOMP(game_object, TransformComponent)->position;
  }
}

void EntityComponent::UseSpell(int spell, GameObject *obj)
{
  EntityComponent *target = GETCOMP(obj, EntityComponent);
  if (target == 0)
  {
    LOG("Target is not an entity target = " << obj->id);
    return;
  }
  if (spell >= (int)spells.size())
  {
    LOG("object " << game_object->id << " does not have " << spell << "spells");
    return;
  }
  if (casting != -1)
  {
    //already casting a spell
    LOG("object " << game_object->id << " is already casting spell " << casting);
    return;
  }
  //todo check mana
  //todo check for cool downs
  //todo need to add check for being cc'ed
  casting = spell;
  //once the cast time is finished actually cast the spell
  std::cout << "casting spell cast time = " << spells[spell].cast_time << std::endl;
  cast_connection = game_object->zone->dispatcher.Dispatch(std::bind(&EntityComponent::OnCastSpell, this, target), spells[spell].cast_time);
  target_died_connection = obj->destroy_signal.Connect(std::bind(&EntityComponent::Interupt, this));

  //send packet to all the players that the entity is casting a spell
  char buffer[500];
  unsigned size = 0;
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("CastSpell");
  size += sizeof(MessageType);
  *reinterpret_cast<int*>(buffer+size) = game_object->id;
  size += sizeof(int);
  *reinterpret_cast<char*>(buffer+size) = spells[spell].type;
  size += sizeof(char);
  *reinterpret_cast<char*>(buffer+size) = spells[spell].visual;
  size += sizeof(char);
  for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
  {
    LOG("Sending spell cast to player " << it->first);
    char msg[500];
    unsigned message_size = size;
    CreateForwardMessage(game_object->zone->protocol, buffer, message_size, it->first, msg);
    BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr];
    game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, flags);
  }
}

void EntityComponent::OnCastSpell(EntityComponent *target)
{
  if (casting < 0)
  {
    //no longer casting
    return;
  }
  //only apply the spell to the target if its an instant hit spell
  if (spells[casting].type == 0)
  {
    ::UseSpell(spells[casting], entity, target->entity);
  }
  //projectiles make a projectile
  else if (spells[casting].type == 1)
  {
    //for projectile based spells we need to make the projectile and shoot it
    GameObject *projectile = game_object->zone->CreateGameObject();
    static int counter = 0;
    counter += 1;
    projectile->name = "fireball" + std::to_string(counter);
    LOG("Creating " << projectile->name);
    //add transform and set the position to my position
    ADDCOMP(projectile, TransformComponent)->position = GETCOMP(game_object, TransformComponent)->position;
    //add projectile comp and set velocity to be towards the target
    ADDCOMP(projectile, ProjectileComponent)->velocity = (GETCOMP(target->game_object, TransformComponent)->position - GETCOMP(game_object, TransformComponent)->position).normalized() * 3;
    //for slelf cast shoo in the direction the entity is facing
    if (target == this)
    {
      GETCOMP(projectile, ProjectileComponent)->velocity = facing.normalized() * 3;
    }
    ADDCOMP(projectile, ColliderComponent);
    GETCOMP(projectile, ColliderComponent)->shape = new Circle();
    ((Circle*)GETCOMP(projectile, ColliderComponent)->shape)->radius = 0.5;
    GETCOMP(projectile, ColliderComponent)->shape->object = projectile;
    GETCOMP(projectile, ColliderComponent)->collision_group = 1; //projectile collision group todo need an enum
    //listen for when the projectile hits something
    Connection temp = GETCOMP(projectile, ColliderComponent)->collision_started_signal.Connect(std::bind(&EntityComponent::onProjectileCollision, this, projectile, std::placeholders::_1, spells[casting]));
    projectiles[projectile] = temp;
  }
  cool_downs[casting] = game_object->zone->dispatcher.timer.GetTotalTime();

  //send packet to all the players that the entity has fired the spell
  char buffer[500];
  unsigned size = 0;
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("FireSpell");
  size += sizeof(MessageType);
  *reinterpret_cast<int*>(buffer+size) = game_object->id;
  size += sizeof(int);
  *reinterpret_cast<int*>(buffer+size) = target->game_object->id;
  size += sizeof(int);
  *reinterpret_cast<char*>(buffer+size) = spells[casting].type;
  size += sizeof(char);
  *reinterpret_cast<char*>(buffer+size) = spells[casting].visual;
  size += sizeof(char);
  for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
  {
    LOG("Sending spell fire to player " << it->first);
    char msg[500];
    unsigned message_size = size;
    CreateForwardMessage(game_object->zone->protocol, buffer, message_size, it->first, msg);
    BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr];
    game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, flags);
  }

  casting = -1;
  target_died_connection.Disconnect();
}

void EntityComponent::onProjectileCollision(GameObject * projectile, GameObject *other, Spell spell)
{
  //if projectile is colliding with caster ignore it
  if (other == game_object)
  {
    return;
  }
  LOG("Projectile Collision!!");

  //remove the projectile from the list
  auto it = projectiles.find(projectile);
  if (it != projectiles.end())
  {
    projectiles.erase(it);
  }
  else
  {
    LOGW("Projectile hit something but was not in the projectiles map");
  }

  //if the thing hit was an entity then cast the spell on it
  if (GETCOMP(other, EntityComponent))
  {
    ::UseSpell(spell, entity, GETCOMP(other, EntityComponent)->entity);
  }

  //send packet to all the players that the entity has fired the spell
  char buffer[500];
  unsigned size = 0;
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("HitSpell");
  size += sizeof(MessageType);
  *reinterpret_cast<int*>(buffer+size) = game_object->id;
  size += sizeof(int);
  *reinterpret_cast<int*>(buffer+size) = other->id;
  size += sizeof(int);
  *reinterpret_cast<char*>(buffer+size) = spell.type;
  size += sizeof(char);
  *reinterpret_cast<char*>(buffer+size) = spell.visual;
  size += sizeof(char);
  for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
  {
    LOG("Sending spell hit to player " << it->first);
    char msg[500];
    unsigned message_size = size;
    CreateForwardMessage(game_object->zone->protocol, buffer, message_size, it->first, msg);
    BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr];
    game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, flags);
  }
  //kill the projectile, use dispatcher becasue this function was called from the projectiles collision signal
  game_object->zone->dispatcher.Dispatch(std::bind(&Zone::RemoveGameObject, game_object->zone, projectile));
}

void EntityComponent::Interupt()
{
  //stop the spell that is being casted, wont put the spell on cooldown
  casting = -1;
  target_died_connection.Disconnect();
  game_object->zone->dispatcher.RemoveTimedMessage(cast_connection);
}

void EntityComponent::OnDamaged(double damage)
{
  std::cout << "object " << game_object->id << " took " << damage << " damage" << std::endl;
  std::cout << "hp = " << entity.stats[Entity::current_hp] << " / " << entity.stats[Entity::max_hp] << std::endl;
  //negative because we lost that much hp
  SendHpChange(-damage);
}
void EntityComponent::OnHealed(double heal)
{
  std::cout << "object " << game_object->id << " healed " << heal << " hp" << std::endl;
  std::cout << "hp = " << entity.stats[Entity::current_hp] << " / " << entity.stats[Entity::max_hp] << std::endl;
  SendHpChange(heal);
}
void EntityComponent::OnManaRecovered(double mana)
{
  std::cout << "object " << game_object->id << " recovered " << mana << " mana" << std::endl;
  std::cout << "mana = " << entity.stats[Entity::current_mana] << " / " << entity.stats[Entity::max_mana] << std::endl;
  SendManaChange(mana);
}
void EntityComponent::OnManaDrained(double mana)
{
  std::cout << "object " << game_object->id << " lost " << mana << " mana" << std::endl;
  std::cout << "mana = " << entity.stats[Entity::current_mana] << " / " << entity.stats[Entity::max_mana] << std::endl;
  SendManaChange(-mana);
}

void EntityComponent::SendHpChange(double hp)
{
  //send packet to all the players that this entity has changed hp
  char buffer[500];
  unsigned size = 0;
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("EntityStatChange");
  size += sizeof(MessageType);
  *reinterpret_cast<int*>(buffer+size) = game_object->id;
  size += sizeof(int);
  *reinterpret_cast<char*>(buffer+size) = Entity::current_hp;//the stat being changed
  size += sizeof(char);
  *reinterpret_cast<float*>(buffer+size) = hp;//the amoun it changed
  size += sizeof(float);
  for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
  {
    LOG("Sending hp change to player " << it->first);
    char msg[500];
    unsigned message_size = size;
    CreateForwardMessage(game_object->zone->protocol, buffer, message_size, it->first, msg);
    BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr];
    game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, flags);
  }
}
void EntityComponent::SendManaChange(double mana)
{
  //send packet to all the players that this entity has changed mana, this could be same function ad hp with a parameter.....TODO
  char buffer[500];
  unsigned size = 0;
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("EntityStatChange");
  size += sizeof(MessageType);
  *reinterpret_cast<int*>(buffer+size) = game_object->id;
  size += sizeof(int);
  *reinterpret_cast<char*>(buffer+size) = Entity::current_mana;//the stat being changed
  size += sizeof(char);
  *reinterpret_cast<float*>(buffer+size) = mana;//the amoun it changed
  size += sizeof(float);
  for (auto it = game_object->zone->players.begin(); it != game_object->zone->players.end(); ++it)
  {
    LOG("Sending hp change to player " << it->first);
    char msg[500];
    unsigned message_size = size;
    CreateForwardMessage(game_object->zone->protocol, buffer, message_size, it->first, msg);
    BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(it->second, PlayerControllerComponent)->lb_addr];
    game_object->zone->stack.Send(msg, message_size, &GETCOMP(it->second, PlayerControllerComponent)->lb_addr, flags);
  }
}