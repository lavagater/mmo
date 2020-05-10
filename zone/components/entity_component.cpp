#include "game_object.h"
#include "zone.h"
#include "entity_component.h"
#include "debug_component.h"
#include "player_controller_component.h"
#include "database_protocol.h"
#include "transform_component.h"
#include "collider_component.h"
#include "projectile.h"
#include "logger.h"
#include "query.h"
#include <iostream>
#include "test_spell.h"
#include "fireball_spell.h"

void EntityComponent::Init()
{
  //casting must start at -1
  casting = -1;

  //setup update loop
  update_connection = game_object->zone->update_signal.Connect(std::bind(&EntityComponent::OnUpdate, this, std::placeholders::_1));

  //make some default stats for the entity
	entity.base_stat[Entity::Health] = 100;
	entity.base_stat[Entity::CurrentHealth] = 100;
	entity.base_stat[Entity::Mana] = 100;
	entity.base_stat[Entity::CurrentMana] = 100;
  entity.base_stat[Entity::Intelegence] = 10;
  entity.base_stat[Entity::Strength] = 10;

  //todo make a basic attack spell to fill all posible slots by default
  TestSpell *test_spell = new TestSpell();
  test_spell->type = 0;
  test_spell->zone = game_object->zone;
  test_spell->caster = this;
  test_spell->cast_time = 0.5;
  test_spell->cool_down = 10;
  test_spell->range = 10;
  test_spell->mana_cost = 0;

  //make a fire ball spell to test with
  game_object->zone->spell_data.owner = game_object;
  FireBall *fire_ball = (FireBall*)game_object->zone->fireball_factory.CreateSpell(&game_object->zone->spell_data);

  //add default spells for unused slots
  spells.push_back(fire_ball);
  cool_downs.push_back(0);

  //todo move this when the spells come from the database
  PlayerControllerComponent *player = GETCOMP(game_object, PlayerControllerComponent);
  if (player)
  {
    //send to the client the spell info for the test spell
    char buffer[1000];
    char msg[1000];
    Stream stream(buffer, 1000);
    stream << game_object->zone->protocol.LookUp("SpellInfo") << (int)0 << fire_ball->cool_down << fire_ball->cast_time << fire_ball->mana_cost << fire_ball->range;
    std::string description = fire_ball->getDescription();
    memcpy(stream.data+stream.pos, description.data(), description.size());
    stream.pos += description.size();
    unsigned message_size = stream.pos;
    CreateForwardMessage(game_object->zone->protocol, buffer, message_size, player->id, msg);
    BitArray<HEADERSIZE> flags = game_object->zone->flags[player->lb_addr];
    game_object->zone->stack.Send(msg, message_size, &player->lb_addr, flags);

    //send the spell location(i.e its equiped in slot one)
    //reset stream object
    stream.pos = 0;
    stream << game_object->zone->protocol.LookUp("SpellLocation") << (int)0 << char(0) << char(0);
    message_size = stream.pos;
    CreateForwardMessage(game_object->zone->protocol, buffer, message_size, player->id, msg);
    flags = game_object->zone->flags[player->lb_addr];
    game_object->zone->stack.Send(msg, message_size, &player->lb_addr, flags);
  }
}

void EntityComponent::SendSpellQuery(Spell &spell, unsigned slot_num)
{
  (void)spell;
  (void)slot_num;
  /*
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
  game_object->zone->stack.Send(query, len, &game_object->zone->spells_database, game_object->zone->flags[game_object->zone->spells_database]);*/
}

void EntityComponent::SendSpellQuery(unsigned slot_num)
{
  (void)slot_num;/*
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
  game_object->zone->stack.Send(query, len, &game_object->zone->spells_database, game_object->zone->flags[game_object->zone->spells_database]);*/
}
void EntityComponent::OnSpellDatabaseResponse(char *data, unsigned size, unsigned slot)
{
  (void)data;
  (void)size;
  (void)slot;
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
  //todo should add a one of these for each stat
  float temp = entity.getStat(Entity::Stats::CurrentHealth);
  if (temp != previous_health)
  {
    LOG("hp changed " << previous_health << " -> " << temp);
    SendHpChange();
  }
  previous_health = temp;
  temp = entity.getStat(Entity::Stats::CurrentMana);
  if (temp != previous_mana)
  {
    LOG("mana changed " << previous_mana << " -> " << temp);
    SendManaChange();
  }
  previous_mana = temp;
}

bool EntityComponent::CheckCoolDown(int spell)
{
  return cool_downs[spell] <= game_object->zone->dispatcher.timer.GetTotalTime();
}

void EntityComponent::UseSpell(int spell, GameObject *obj)
{
  (void)spell;
  (void)obj;
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
    //tell player they cant cast atm
    SendCantCast();
    return;
  }
  //check mana
  float mana = entity.getStat(Entity::CurrentMana);
  if (mana < spells[spell]->mana_cost)
  {
    LOG("object " << game_object->id << " does not have enough mana has " << mana << " needs " << spells[spell]->mana_cost);
    SendNotEnoughMana();
    return;
  }
  //check for cool downs
  if (CheckCoolDown(spell) == false)
  {
    LOG("object " << game_object->id << " spell " << spell << " is on cooldown");
    SendOnCooldown();
    return;
  }
  //check range
  if ((GETCOMP(game_object, TransformComponent)->position - GETCOMP(obj, TransformComponent)->position).squaredNorm() > spells[spell]->range * spells[spell]->range)
  {
    LOG("object " << game_object->id << " is too far away to cast spell");
    SendOutOfRange();
    return;
  }
  //todo need to add check for being cc'ed
  casting = spell;
  //once the cast time is finished actually cast the spell
  std::cout << "casting spell cast time = " << spells[spell]->cast_time << std::endl;
  cast_connection = game_object->zone->dispatcher.Dispatch(std::bind(&EntityComponent::OnCastSpell, this, target), spells[spell]->cast_time);
  target_died_connection = obj->destroy_signal.Connect(std::bind(&EntityComponent::Interupt, this));

  //send packet to all the players that the entity is casting a spell
  char buffer[500];
  unsigned size = 0;
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("CastSpell");
  size += sizeof(MessageType);
  *reinterpret_cast<int*>(buffer+size) = game_object->id;
  size += sizeof(int);
  *reinterpret_cast<char*>(buffer+size) = spells[spell]->type;
  size += sizeof(char);
  *reinterpret_cast<char*>(buffer+size) = 0;
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
  (void)target;
  if (casting < 0)
  {
    //no longer casting
    return;
  }

  //subtract the mana cost for the spell
  entity.burn_mana(spells[casting]->mana_cost);

  //cast the spell
  spells[casting]->cast(target);

  //send a packet to the player that cast the spell, so the client can show the cooldown animation and what not
  PlayerControllerComponent *player = GETCOMP(game_object, PlayerControllerComponent);
  if (player)
  {
    //reset the stream
    char buffer[500];
    Stream stream(buffer, 500);
    stream << game_object->zone->protocol.LookUp("ClientSpell") << casting;
    char msg[500];
    unsigned message_size = stream.pos;
    CreateForwardMessage(game_object->zone->protocol, buffer, message_size, player->id, msg);
    BitArray<HEADERSIZE> flags = game_object->zone->flags[player->lb_addr];
    game_object->zone->stack.Send(msg, message_size, &player->lb_addr, flags);
  }

  casting = -1;
  target_died_connection.Disconnect();
}

void EntityComponent::Interupt()
{
  //stop the spell that is being casted, wont put the spell on cooldown
  casting = -1;
  target_died_connection.Disconnect();
  game_object->zone->dispatcher.RemoveTimedMessage(cast_connection);
}

void EntityComponent::SendHpChange()
{
  //send packet to all the players that this entity has changed hp
  char buffer[500];
  Stream stream(buffer, 500);
  stream << game_object->zone->protocol.LookUp("EntityStatChange") << game_object->id << (char)Entity::Stats::CurrentHealth << entity.getStat(Entity::Stats::CurrentHealth);
  game_object->zone->SendToAllPlayers(stream);
}
void EntityComponent::SendManaChange()
{
  char buffer[500];
  Stream stream(buffer, 500);
  stream << game_object->zone->protocol.LookUp("EntityStatChange") << game_object->id << (char)Entity::Stats::CurrentMana << entity.getStat(Entity::Stats::CurrentMana);
  game_object->zone->SendToAllPlayers(stream);
}
void EntityComponent::SendNotEnoughMana()
{
  if (GETCOMP(game_object, PlayerControllerComponent) == 0)
  {
    return;
  }
  //send packet to all the player that says he did not have enough mana to cast the spell
  char buffer[500];
  unsigned size = 0;
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("NotEnoughMana");
  size += sizeof(MessageType);

  //send it to the player
  char msg[500];
  unsigned message_size = size;
  CreateForwardMessage(game_object->zone->protocol, buffer, message_size, GETCOMP(game_object, PlayerControllerComponent)->id, msg);
  BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(game_object, PlayerControllerComponent)->lb_addr];
  game_object->zone->stack.Send(msg, message_size, &GETCOMP(game_object, PlayerControllerComponent)->lb_addr, flags);
}
void EntityComponent::SendOutOfRange()
{
  if (GETCOMP(game_object, PlayerControllerComponent) == 0)
  {
    return;
  }
  //send packet to all the player that says he did not have enough mana to cast the spell
  char buffer[500];
  unsigned size = 0;
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("OutOfRange");
  size += sizeof(MessageType);

  //send it to the player
  char msg[500];
  unsigned message_size = size;
  CreateForwardMessage(game_object->zone->protocol, buffer, message_size, GETCOMP(game_object, PlayerControllerComponent)->id, msg);
  BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(game_object, PlayerControllerComponent)->lb_addr];
  game_object->zone->stack.Send(msg, message_size, &GETCOMP(game_object, PlayerControllerComponent)->lb_addr, flags);
}
void EntityComponent::SendOnCooldown()
{
  if (GETCOMP(game_object, PlayerControllerComponent) == 0)
  {
    return;
  }
  //send packet to all the player that says he did not have enough mana to cast the spell
  char buffer[500];
  unsigned size = 0;
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("OnCooldDown");
  size += sizeof(MessageType);

  //send it to the player
  char msg[500];
  unsigned message_size = size;
  CreateForwardMessage(game_object->zone->protocol, buffer, message_size, GETCOMP(game_object, PlayerControllerComponent)->id, msg);
  BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(game_object, PlayerControllerComponent)->lb_addr];
  game_object->zone->stack.Send(msg, message_size, &GETCOMP(game_object, PlayerControllerComponent)->lb_addr, flags);
}

void EntityComponent::SendCantCast()
{
  if (GETCOMP(game_object, PlayerControllerComponent) == 0)
  {
    return;
  }
  //send packet to all the player that says he did not have enough mana to cast the spell
  char buffer[500];
  unsigned size = 0;
  *reinterpret_cast<MessageType*>(buffer) = game_object->zone->protocol.LookUp("CantCast");
  size += sizeof(MessageType);

  //send it to the player
  char msg[500];
  unsigned message_size = size;
  CreateForwardMessage(game_object->zone->protocol, buffer, message_size, GETCOMP(game_object, PlayerControllerComponent)->id, msg);
  BitArray<HEADERSIZE> flags = game_object->zone->flags[GETCOMP(game_object, PlayerControllerComponent)->lb_addr];
  game_object->zone->stack.Send(msg, message_size, &GETCOMP(game_object, PlayerControllerComponent)->lb_addr, flags);
}