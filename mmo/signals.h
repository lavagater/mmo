#ifndef SIGNALS_H
#define SIGNALS_H

#include <unordered_map>
#include <functional>
#include <vector>

class InternalConnection;

class SignalBase
{
  public:
  virtual ~SignalBase() {}
  virtual void Disconnect(InternalConnection *connection) = 0;
};

class ConnectionProxy
{
public:
  ConnectionProxy(InternalConnection *);
  InternalConnection *internal_connection;
};

class InternalConnection
{
public:
  void Disconnect();
  SignalBase *signal;
  int ref_count;
  bool clean_up_self;
};

class Connection
{
public:
  Connection();
  Connection(ConnectionProxy);
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
  Signals(){}
  Signals<Args...> &operator=(const Signals<Args...>&);
  Signals(const Signals<Args...>&);
  void Copy(const Signals<Args...>&);
  ConnectionProxy Connect(std::function<void(Args...)> slot);
  void Clear();
  void operator()(Args... p);
  operator bool();

private:
  std::unordered_map<InternalConnection*, std::function<void(Args...)> > slots;
  void Disconnect(InternalConnection *connection);
};

template<typename... Args>
void Signals<Args...>::Copy(const Signals<Args...>&rhs)
{
  //add all the old signals connections
  for (auto it = rhs.slots.begin(); it != rhs.slots.end(); ++it)
  {
    Connect(it->second);
  }
}

template<typename... Args>
Signals<Args...>& Signals<Args...>::operator=(const Signals<Args...>&rhs)
{
  Copy(rhs);
  return *this;
}

template<typename... Args>
Signals<Args...>::Signals(const Signals<Args...>&rhs)
{
  Copy(rhs);
}

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
Signals<Args...>::operator bool()
{
 return slots.size() > 0;
}

template<typename... Args>
ConnectionProxy Signals<Args...>::Connect(std::function<void(Args...)> slot)
{
  InternalConnection *connection = new InternalConnection;
  connection->signal = this;
  connection->clean_up_self = true;
  slots[connection] = slot;
  return ConnectionProxy(connection);
}

template<typename... Args>
void Signals<Args...>::operator()(Args... p)
{
  //get all the connections in one loop
  std::vector<InternalConnection*> connections;
  for (auto it = slots.begin(); it != slots.end(); ++it)
  {
    connections.push_back(it->first);
  }
  //now loop through the connections since the connections vector wont change during the loop
  //this solves the problem of the signal or slot being modified during this function
  for (unsigned i = 0; i < connections.size(); ++i)
  {
    if (slots.find(connections[i]) != slots.end())
    {
      slots[connections[i]](p...);
    }
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