#include "signals.h"
#include <iostream>

void InternalConnection::Disconnect()
{
  signal = 0;
}

ConnectionProxy::ConnectionProxy(InternalConnection *conn)
{
  internal_connection = conn;
}

Connection::Connection()
{
  internal_connection = 0;
}
Connection::Connection(ConnectionProxy proxy)
{
  internal_connection = proxy.internal_connection;
  //since this is from a proxy this internal connection is garunteed to be a virgin
  internal_connection->ref_count = 1;
}

Connection::Connection(const Connection &rhs)
{
  internal_connection = rhs.internal_connection;
  if (internal_connection)
  {
    internal_connection->ref_count += 1;
  }
}

Connection &Connection::operator=(const Connection &rhs)
{
  internal_connection = rhs.internal_connection;
  if (internal_connection)
  {
    internal_connection->ref_count += 1;
  }
  return *this;
}

Connection::~Connection()
{
  DeleteInternal();
}

void Connection::Disconnect()
{
  if (internal_connection && internal_connection->signal)
  {
    internal_connection->signal->Disconnect(this->internal_connection);
  }
  DeleteInternal();
}

void Connection::DeleteInternal()
{
  if (internal_connection)
  {
    internal_connection->ref_count -= 1;
    if (internal_connection->ref_count == 0)
    {
      if (internal_connection->signal)
      {
        internal_connection->signal->Disconnect(this->internal_connection);
      }
      delete internal_connection;
    }
    internal_connection = 0;
  }
}