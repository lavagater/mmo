#ifndef SIGNALS_H
#define SIGNALS_H

#include <unordered_map>
#include <functional>

class Connection;

class SignalBase
{
  public:
  virtual ~SignalBase() {}
  virtual void Disconnect(Connection &connection) = 0;
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
  ~Connection();
  void Disconnect();
  //dont touch
  InternalConnection *internal_connection;
private:
  void DeleteInternal();
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
  void Disconnect(Connection &connection);
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
void Signals<Args...>::Disconnect(Connection &connection)
{
  auto it = slots.find(connection.internal_connection);
  if (it != slots.end())
  {
    it->first->Disconnect();
    slots.erase(it);
  }
}

#endif