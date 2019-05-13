/*!
  \author
    Wyatt Lavigueure
  \date
    4/19/2019
  \brief
    Thread safe queue to be used by dispatcher.
*/
#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>

template<typename T>
class ThreadSafeQueue
{
public:
  T & front()
  {
    std::lock_guard<std::mutex> lock(mutex);
	return internal_queue.front();
  }
  T pop()
  {
    std::lock_guard<std::mutex> lock(mutex);
	T ret = internal_queue.front();
	internal_queue.pop();
    return ret;
  }
  bool empty()
  {
    std::lock_guard<std::mutex> lock(mutex);
    return internal_queue.empty();
  }
  size_t size()
  {
    std::lock_guard<std::mutex> lock(mutex);
    return internal_queue.size();
  }
  void push(T &element)
  {
	std::lock_guard<std::mutex> lock(mutex);
	return internal_queue.push(element);
  }
private:
  std::queue<T> internal_queue;
  std::mutex mutex;
};
#endif