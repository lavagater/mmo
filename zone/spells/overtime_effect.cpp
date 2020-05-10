#include "overtime_effect.h"
#include "game_object.h"
#include "entity_component.h"

OverTimeEffect::~OverTimeEffect()
{
  //onlye remove the TimedMessage if its valid
  if (effect_end.start_time != 0)
  {
    //in case we are dying for a different reason, remove the timed event
    dispatcher->RemoveTimedMessage(effect_end);
  }
}
void OverTimeEffect::cast(EntityComponent *target)
{
  OverTimeEffect *temp = static_cast<OverTimeEffect*>(copy());
  //after the duration delete the effect
  temp->effect_end = dispatcher->Dispatch([temp](double){delete temp;}, duration);
  temp->target_died = target->game_object->destroy_signal.Connect([this, temp](GameObject*){delete temp;});

  temp->startEffect(target);
}