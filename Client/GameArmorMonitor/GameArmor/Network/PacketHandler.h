#pragma once

#include "..\..\Crypt\Blowfish.h"
#include <deque>

using namespace std;

#define BUFP(p,pos) (((unsigned char*)(p)) + (pos))
#define BUFB(p,pos) (*(unsigned char*)BUFP((p),(pos)))
#define BUFW(p,pos) (*(unsigned short*)BUFP((p),(pos)))
#define BUFL(p,pos) (*(unsigned long*)BUFP((p),(pos)))
#define BUFQ(p,pos) (*(unsigned long long*)BUFP((p),(pos)))

enum
{
	CONNECTION_NEW,
	CONNECTION_OK,
	CONNECTION_CLOSED,
};

typedef struct
{
	char *Buffer;
	int Size;
} Packet;

class PacketHandler
{
public:
	PacketHandler(void);
	~PacketHandler(void);

	void setState(int state)
	{
		this->state = state;
	}

	bool hasPackets()
	{
		return packetQueue.size() > 0;
	}

	void ChangeServer();
	void Boot(char *key);

	void EnqueueBytes(char *buff, int size);
	int ReceiveSomePackets(char *buff, int size);
	bool ProcessPackets(char *buff, int size);

	void EnqueueSend(char *buff, int size);
	int FlushSendQueue(int sock);

	void SendPacketToClient(char *buff, int size);

private:
	deque<Packet> packetQueue;
	deque<Packet> sendQueue;
	CBlowFish *blowfish;

	Packet tempPacket;
	Packet loginPacket;
	int tempPacketFullSize;

	int aid;
	int state;

	bool gameguardChallenged;
	bool gamearmorChallenged;
};
