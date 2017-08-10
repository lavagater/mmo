#include "remote_database.h"

void RemoteDatabase::ProcessGet(char *buffer, int length, sockaddr_in *from)
{
  NetworkEvent data(buffer, length, from);
  db_events[*from].SendEvent(DatabaseGetEvent, &data);
}
void RemoteDatabase::ProcessCreate(char *buffer, int length, sockaddr_in *from)
{
  NetworkEvent data(buffer, length, from);
  db_events[*from].SendEvent(DatabaseCreateEvent, &data);
}
void RemoteDatabase::ProcessFind(char *buffer, int length, sockaddr_in *from)
{
  NetworkEvent data(buffer, length, from);
  db_events[*from].SendEvent(DatabaseFindEvent, &data);
}