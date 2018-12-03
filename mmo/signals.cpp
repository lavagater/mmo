#include "signals.h"

void InternalConnection::Disconnect()
{
  signal = 0;
}

Connection::Connection(InternalConnection *conn)
{
  internal_connection = conn;
  internal_connection->ref_count += 1;
}

Connection::~Connection()
{
  DeleteInternal();
}

void Connection::Disconnect()
{
  DeleteInternal();
}

void Connection::DeleteInternal()
{
  if (internal_connection)
  {
    if (internal_connection->signal)
    {
      internal_connection->signal->Disconnect(*this);
    }
    internal_connection->ref_count -= 1;
    if (internal_connection->ref_count == 0)
    {
      delete internal_connection;
    }
    internal_connection = 0;
  }
}