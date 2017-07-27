/*!
  \author
    Wyatt Lavigueure
  \date
    7/24/2017
  \brief
    Event system similar to c# delagates. Create an event object, then fucntions can subscribe to an event type
    then with one dispatch call it will call all the functions subscribed to that event type.
*/
#ifndef EVNT_H
#define EVNT_H
#include "event_types.h"

#include <list>
#include <vector>

template <typename T>
using CallbackType = void(T::*)(void*);

/*!
  /brief
    Base class so that the inherited templated classes can be stored into an array
*/
class CallbackBase
{
public:
  /*!
    \brief
      The function that calls the function
    \param data
      The param to pass to the functins
  */
   virtual void Callback(__attribute__((unused))void *data) {}
   /*!
     \brief
       Make the destructor virtual so that inherited classes have there destructors called
   */
   virtual ~CallbackBase(){}
};

/*!
  \brief
    A wrapper around a member function pointer
*/
template <typename T>
class CallbackClass : public CallbackBase
{
public:
  /*!
    \brief
      This function calls the member function
    \param data
      The data to send to the function
  */
   void Callback(void* data)
   {
      (mObject->*mCallback)(data);
   }
   /*!
     \brief
       Pointer to the object to call the function on
   */
   T* mObject;
   /*!
     \brief
       The function to call on the object
   */
   CallbackType<T> mCallback;
};

/*!
  \brief
    Wrapper for a normal function pointer
*/
class CallbackClassNormal : public CallbackBase
{
public:
  /*!
    \brief
      This function calls a normal function
    \param data
      The data to send to the function
  */
   void Callback(void* data)
   {
      mCallback(data);
   }
   /*!
     \brief
       The function to call
   */
   void (*mCallback)(void*);
};

typedef std::vector< std::list< CallbackBase* > > EventTable;

/*!
  \brief
    The event class. each event class has its own subscriptions, so if a ConnectEvent is called on one event object then
    SendEvent is called from a seperate event object it will not call the any events from other objects.
*/
class Event
{
  EventTable mEvents;
public:
  /*!
    \brief
      Construct the event object, just allocates the eventtable
  */
  Event();
  /*!
    \brief
      Frees memory from the EventTable
  */
  ~Event();
  /*!
    \brief
      Calls all the functions that are subscribed(connected) to the given event type and sends each function the data
    \param type
      Which event type from event_type.h
    \param data
      Pointer to the data to send to each function
  */
  void SendEvent(EventType type, void* data);

  /*!
    \brief
      Subscribes a member function to this event oject
    \param type
      The type of event to subscribe to
    \param object
      The object that the member funtion is on
    \param callback
      The function to call
  */
  template <typename T>
  void ConnectEvent(EventType type, T* object, CallbackType<T> callback)
  {
    CallbackClass<T>* cb = new CallbackClass<T>();
    cb->mCallback = callback;
    cb->mObject = object;
    mEvents[type].push_back(cb);
  }

  /*!
    \brief
      Unsubscribes a function from an event
    \param type
      The event type
    \param object
      The object that the callback funtion is on
    \param callback
      The function that was connected
  */
  template <typename T>
  void DisconnectEvent(EventType type, T* object, CallbackType<T> callback)
  {
    for (auto it = mEvents[type].begin(); it != mEvents[type].end(); ++it)
    {
      if (reinterpret_cast<CallbackClass<T>*>(*it)->mObject == object && reinterpret_cast<CallbackClass<T>*>(*it)->mCallback == callback)
      {
        delete (*it);
        mEvents[type].erase(it);
        break;
      }
    }
  }
  /*!
    \brief
      Subscribes a normal function to this event oject
    \param type
      The type of event to subscribe to
    \param callback
      The function to call
  */
  void ConnectEvent(EventType type, void (*callback)(void*))
  {
    CallbackClassNormal* cb = new CallbackClassNormal();
    cb->mCallback = callback;
    mEvents[type].push_back(cb);
  }

  /*!
    \brief
      Unsubscribes a function from an event
    \param type
      The event type
    \param callback
      The function that was connected
  */
  void DisconnectEvent(EventType type, void (*callback)(void*))
  {
    for (auto it = mEvents[type].begin(); it != mEvents[type].end(); ++it)
    {
      if (reinterpret_cast<CallbackClassNormal*>(*it)->mCallback == callback)
      {
        delete (*it);
        mEvents[type].erase(it);
        break;
      }
    }
  }
};
#endif