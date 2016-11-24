#pragma once

#include "PacketHandler.h"

typedef int (__stdcall * connectPrototype)(int s, const struct sockaddr *name, int namelen);
typedef int (__stdcall * sendPrototype)(int sock, char *buf, int len, int flags);
typedef int (__stdcall * recvPrototype)(int sock, char *buf, int len, int flags);
typedef int (__stdcall * selectPrototype)(int nfds, void *readfds, void *writefds, void *exceptfds, const struct timeval* timeout);

class NetworkHooks
{
public:
	NetworkHooks();
	~NetworkHooks();

	void Boot();

	int Connect(int sock, const struct sockaddr *name, int namelen);
	int Send(int sock, char *buf, int len, int flags);
	int Receive(int sock, char *buf, int len, int flags);
	int Select(int nfds, void *readfds, void *writefds, void *exceptfds, const struct timeval* timeout);
	
	PacketHandler *getPacketHandler()
	{
		return &packetHandler;
	}

	connectPrototype getConnect()
	{
		return origConnect;
	}

	sendPrototype getSend()
	{
		return origSend;
	}

	recvPrototype getRecv()
	{
		return origRecv;
	}

	int getConn()
	{
		return conn;
	}

	void setDisconnected(bool close)
	{
		disconnected = close;
	}

private:
	bool hooked;
	int conn;
	bool disconnected;

	connectPrototype origConnect;
	sendPrototype origSend;
	recvPrototype origRecv;
	selectPrototype origSelect;

	PacketHandler packetHandler;
};
