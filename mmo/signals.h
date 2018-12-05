#ifndef SIGNALS_H
#define SIGNALS_H

#include <unordered_map>
#include <functional>

class InternalConnection;

class SignalBase
{
  public:
  virtual ~SignalBase() {}
  virtual void Disconnect(InternalConnection *connection) = 0;
};

class InternalConnection
{
public:
  void Disconnect();
  SignalBase *signal;
  int ref_count;
};

class Connection
{
public:
  Connection(InternalConnection *conn);
  Connection(const Connection &rhs);
  Connection &operator=(const Connection &rhs);
  ~Connection();
  void Disconnect();
private:
  void DeleteInternal();
  InternalConnection *internal_connection;
};

template<typename... Args>
class Signals : public SignalBase
{
public:
  ~Signals();
  Connection Connect(std::function<void(Args...)> slot);
  void Clear();
  void operator()(Args... p);

private:
  std::unordered_map<InternalConnection*, std::function<void(Args...)> > slots;
  void Disconnect(InternalConnection *connection);
};

template<typename... Args>
Signals<Args...>::~Signals()
{
  Clear();
}

template<typename... Args>
void Signals<Args...>::Clear()
{
 for (auto it = slots.begin(); it != slots.end(); ++it)
 {
  it->first->Disconnect();
 }
 slots.clear();
}

template<typename... Args>
Connection Signals<Args...>::Connect(std::function<void(Args...)> slot)
{
  InternalConnection *connection = new InternalConnection;
  connection->signal = this;
  slots[connection] = slot;
  return Connection(connection);
}

template<typename... Args>
void Signals<Args...>::operator()(Args... p)
{
  for (auto it = slots.begin(); it != slots.end(); ++it)
  {
    it->second(p...);
  }
}

template<typename... Args>
void Signals<Args...>::Disconnect(InternalConnection *connection)
{
  auto it = slots.find(connection);
  if (it != slots.end())
  {
    it->first->Disconnect();
    slots.erase(it);
  }
}

#endif