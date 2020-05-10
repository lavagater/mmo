/*!
  \author
    Wyatt Lavigueure
  \date
    3/04/2019
  \brief
    Dispatcher used to send all the messages from one place.
*/
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "frame_rate.h"
#include "thread_safe_queue.h"
#include <functional>
#include <list>

struct TimedMessage
{
  double start_time = 0;
  double wait_time;
  std::function<void(double)> message;
};

class Dispatcher
{
public:
  /**
   * @brief add message to dispatcher.
   * @param message the message to be added.
   * @remark This function is thread safe.
   */
  void Dispatch(std::function<void()> message);
  /**
   * @brief add message to dispatcher with an identifier.
   * @param message the message to be added.
   * @param The id of the message, the id does not have to be unique. The id is a void pointer but it is not derefrenced in the dispatcher
   *        and it is completely valid to pass an inergal value as the id.
   * @remark This function is not thread safe.
   */
  void Dispatch(std::function<void()> message, void *id);
  /**
   * @brief add a message that will after a certain amount of time.
   * @param message the message to be added.
   * @param time the time until the message is executed.
   * @return a times message that can be used to remove the message later if need be.
   * @remark This function is not thread safe. But it is safe to use on the same thread that the dispatcher is on.
   */
  TimedMessage Dispatch(std::function<void(double)> message, double time);
  /**
   * @brief removes a timed message from dispatcher.
   * @remark Does not look up the timed message by the message but by the time, 
   *         if two messages were dispatched at the exact same time(which is not possible) with same timer the first one would be removed.
   * @remark This function is not thread safe. But it is safe to use on the same thread that the dispatcher is on.
   */
  void RemoveTimedMessage(TimedMessage &tm);
  /**
   * @brief removes all messages from dispatcher with a given id.
   * @param id the id of the messages to remove.
   * @param return true if on or more messages where removed. otherwise returns false
   * @remark This function is not thread safe. But it is safe to use on the same thread that the dispatcher is on.
   */
  bool RemoveMessageById(void *id);
  //start the dispatcher loop
  void Run();
  //instead of using the dispatcher loop, this function can be polled
  void Update();
  FrameRate timer;
private:
  ThreadSafeQueue<std::function<void()> > messages;
  std::list<std::pair<std::function<void()>, void *> > other_messages;
  std::vector<TimedMessage> timed_messages;
};
#endif