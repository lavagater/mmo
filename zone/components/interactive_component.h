#include "component.h"
#include "signals.h"
#include "wyatt_sock.h"
#include <Eigen/Dense>

/**
 * @brief This component goes on players and makes them walk to an interactable object then interact with it.
 */
class InteractiveComponent : public Component
{
public:
  InteractiveComponent();
  void Init();
  void OnMove();
  /**
   * @brief Moves the player withen interaction range of the other object then calls the function.
   */
  void Interact(GameObject *interactable, std::function<void()> interaction, double range = 0);
  void InteractAux(GameObject *interactable, std::function<void()> interaction, double range = 0);
  void OnTimer();

  //minimum range that this object can interact
  double interaction_range;
  double other_range;

  std::function<void()> interaction_callback;
  GameObject *interactable_object;
  Connection move_connection;
  Connection interactable_destroyed_connection;
  Connection arrive_connection;
};