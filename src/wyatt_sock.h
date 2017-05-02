#ifndef WYATTSOCK_H
#define WYATTSOCK_H
#include <stdlib.h>  /*exit, calloc, free*/
#ifdef _WIN32
#include <WinSock2.h> /*sockets*/
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
typedef int SOCKET;
#define SD_SEND SHUT_WR
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif
#define EBLOCK -6969

void SetNonBlocking(SOCKET sock);
int GetError();
void Close(SOCKET sock, bool now);
int Send(SOCKET sock, const char* buffer, int bytes, sockaddr_in* dest);
int Receive(SOCKET sock, char* buffer, int maxBytes);
int Bind(SOCKET sock, sockaddr_in* addr);
sockaddr_in* CreateAddress(const char* ip, int port);
SOCKET CreateSocket(int protocol);
int Connect(SOCKET sock, sockaddr_in* address);
int SendTCP(SOCKET sock, const char* buffer, int bytes);
int ReceiveTCP(SOCKET sock, char* buffer, int maxBytes);
int Init();
void Deinit();
int Listen(SOCKET socket, int backlog);
SOCKET Accept(SOCKET socket, sockaddr_in* addr);
#endif