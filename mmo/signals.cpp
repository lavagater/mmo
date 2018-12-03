#include "signals.h"
#include <iostream>

void InternalConnection::Disconnect()
{
  std::cout << "internal signal diconnected" << std::endl;
  signal = 0;
}

Connection::Connection(InternalConnection *conn)
{
  std::cout << "connection created" << std::endl;
  internal_connection = conn;
  internal_connection->ref_count += 1;
}

Connection::Connection(const Connection &rhs)
{
  std::cout << "connection copy constructor" << std::endl;
  internal_connection = rhs.internal_connection;
  internal_connection->ref_count += 1;
}

Connection &Connection::operator=(const Connection &rhs)
{
  std::cout << "connection copied" << std::endl;
  internal_connection = rhs.internal_connection;
  internal_connection->ref_count += 1;
  return *this;
}

Connection::~Connection()
{
  std::cout << "connection Deleted" << std::endl;
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