#include "game_object.h"
#include "zone.h"
#include <math.h>
#include "query.h"
#include "utils.h"
#include "movement_component.h"
#include "transform_component.h"
#include "interactive_component.h"

InteractiveComponent::InteractiveComponent()
{
  interaction_range = 0;
}

void InteractiveComponent::Init()
{
}

void InteractiveComponent::OnMove()
{
  interaction_callback = std::function<void()>();
  interactable_object = 0;
  //disconnect the connections through dispatcher because this function could be 
  game_object->zone->dispatcher.Dispatch(std::bind(&Connection::Disconnect, &arrive_connection));
  game_object->zone->dispatcher.Dispatch(std::bind(&Connection::Disconnect, &move_connection));
}

void InteractiveComponent::OnTimer()
{
  if (interactable_object != 0)
  {
    //check if in range
    if ((GETCOMP(interactable_object, TransformComponent)->position - GETCOMP(game_object, TransformComponent)->position).norm() < other_range + interaction_range)
    {
      if (interaction_callback)
      {
        //do the interaction
        interaction_callback();
        //reset everything
        OnMove();
      }
    }
    else
    {
      //if not in range yet (the destination is moving), try interacting with it again
      Interact(interactable_object, interaction_callback, other_range);
    }
  }
}

void InteractiveComponent::Interact(GameObject *interactable, std::function<void()> interaction, double range)
{
  other_range = range;
  interaction_callback = interaction;
  interactable_object = interactable;
  //if the object we are trying to interact with gets deleted thenstop trying to interact with it
  interactable_destroyed_connection = interactable_object->destroy_signal.Connect(std::bind(&InteractiveComponent::OnMove, this));
  arrive_connection = GETCOMP(game_object, MovementComponent)->arrived_signal.Connect(std::bind(&InteractiveComponent::OnTimer, this));
  //disconnect this so moving wont reset everything
  move_connection.Disconnect();
  GETCOMP(game_object, MovementComponent)->MoveTo(GETCOMP(interactable, TransformComponent)->position);
  move_connection = GETCOMP(game_object, MovementComponent)->moved_signal.Connect(std::bind(&InteractiveComponent::OnMove, this));
}