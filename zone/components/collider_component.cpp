#include "collider_component.h"
#include "transform_component.h"
#include "player_controller_component.h"
#include "game_object.h"
#include "zone.h"


ColliderComponent::~ColliderComponent()
{
  for (unsigned i = 0; i < prev_grid.size(); ++i)
  {
    //remove prev grid from colliders
    auto it = game_object->zone->colliders[collision_group][prev_grid[i]].find(game_object);
    if (it != game_object->zone->colliders[collision_group][prev_grid[i]].end())
    {
      game_object->zone->colliders[collision_group][prev_grid[i]].erase(it);
    }
  }
  delete shape;
}

void ColliderComponent::Load(std::istream &stream)
{
  //need to determine which shape the collider is and make the correct shape
  int shape_type;
  stream >> shape_type;
  switch(shape_type)
  {
    case 0:
    {
      Circle *circle = new Circle();
      //TODO Make the shape have a function to read the data from the file
      stream >> circle->radius;
      shape = circle;
      break;
    }
    case 1:
    {
      Line *line = new Line();
      stream >> line->start.x();
      stream >> line->start.y();
      stream >> line->end.x();
      stream >> line->end.y();
      shape = line;
      break;
    }
    default:
      LOGW("Collider with shape " << shape_type);
  }
  stream >> collision_group;
  shape->object = game_object;
}
void ColliderComponent::Write(std::ostream &stream)
{
  stream << shape->GetType();
  stream << "\n";
  switch(shape->GetType())
  {
    case 0:
    {
      stream << ((Circle*)shape)->radius;
      stream << "\n";
    }
  }
  stream << collision_group;
  stream << "\n";
}

void ColliderComponent::Init()
{
  update_connection = game_object->zone->update_signal.Connect(std::bind(&ColliderComponent::OnBaseUpdate, this));
}

void ColliderComponent::OnBaseUpdate()
{
  if (prev_grid.size() == 0)
  {
    //this should be done on init somehow, but because of shape being dynamic I cant do it
    //maybe do stack allocations
    prev_grid = shape->GetGridOccupation();
    for (unsigned i = 0; i < prev_grid.size(); ++i)
    {
      game_object->zone->colliders[collision_group][prev_grid[i]].insert(game_object);
    }
  }
  //update the colliders grid
  std::vector<std::pair<int, int>> current_grid = shape->GetGridOccupation();
  if (current_grid != prev_grid)
  {
    for (unsigned i = 0; i < prev_grid.size(); ++i)
    {
      //remove prev grid from colliders
      auto it = game_object->zone->colliders[collision_group][prev_grid[i]].find(game_object);
      if (it != game_object->zone->colliders[collision_group][prev_grid[i]].end())
      {
        game_object->zone->colliders[collision_group][prev_grid[i]].erase(it);
      }
    }
    for (unsigned i = 0; i < current_grid.size(); ++i)
    {
      //add the new pos
      game_object->zone->colliders[collision_group][current_grid[i]].insert(game_object);
    }
    prev_grid = current_grid;
  }
  //check for collisions
  //save the objects you already checked collision with to not collide twice
  std::unordered_set<GameObject*> checked;
  checked.insert(game_object);
  //objects that are colliding this frame, will replace the previous collisions
  std::unordered_set<GameObject*> collided;
  for (auto group = game_object->zone->collision_groups[collision_group].begin(); group != game_object->zone->collision_groups[collision_group].end(); ++group)
  {
    for (unsigned i = 0; i < current_grid.size(); ++i)
    {
      if (game_object->zone->colliders[*group].find(current_grid[i]) != game_object->zone->colliders[*group].end())
      {
        for (auto it = game_object->zone->colliders[*group][current_grid[i]].begin(); it != game_object->zone->colliders[*group][current_grid[i]].end(); ++it)
        {
          //if already checked this frame then skip checking again
          if (checked.find(*it) != checked.end())
          {
            continue;
          }
          checked.insert(*it);
          //check if the object is actually colliding
          if (shape->CheckCollision(GETCOMP(*it, ColliderComponent)->shape))
          {
            collided.insert(*it);
            //now check if this is a collision started or a collision persisted
            if (previous_collisions.find(*it) == previous_collisions.end())
            {
              collision_started_signal(*it);
            }
            else
            {
              collision_persisted_signal(*it);
            }
            
          }
        }
      }
    }
  }
  //check for collision ended
  for (auto it = previous_collisions.begin(); it != previous_collisions.end(); ++it)
  {
    //if we collided with it last frame but not this frame then signal the collision has ended
    if (collided.find(*it) == collided.end())
    {
      collision_ended_signal(*it);
    }
  }
  //save the collisions for next frame
  previous_collisions = collided;
}