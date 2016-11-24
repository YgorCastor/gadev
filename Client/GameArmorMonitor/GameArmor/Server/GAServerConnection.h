#pragma once

#include <string>

using namespace std;

#define GATriggerIP "66.66.66.66"
#define GAServerIP "177.71.180.18"
#define GAServerPort 4589

#pragma pack(push,1)

typedef struct
{
	char Key[32];
	unsigned int IP;
	unsigned short Port;
} GAServerDescriptor;

typedef struct
{
	unsigned short Packet;
	char ClientMD5[32];
	char License[32];
	unsigned int ServerID;
} GA_PACKET_REQUEST_KEY;

typedef struct
{
	unsigned short Packet;

	GAServerDescriptor Server;
} GA_PACKET_REQUEST_KEY_REPLY;

#pragma pack(pop)

class GAServerConnection
{
public:
	GAServerConnection(string ip, unsigned short port);
	~GAServerConnection();

	bool Connect();
	bool Setup(string license, string clientmd5);
	GAServerDescriptor QueryServerInfo(int serverID);
	void Close();

private:
	int conn;
	string license;
	string clientmd5;
	
	string ip;
	unsigned short port;
};
