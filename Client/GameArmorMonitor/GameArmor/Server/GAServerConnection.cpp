#include "GAServerConnection.h"

#include "../../Utils/LMFAOCrypt.h"
#include "../GameArmor.h"
#include <string>

#include <WinSock2.h>
#include <Windows.h>

GAServerConnection::GAServerConnection(string ip, unsigned short port)
{
	this->ip = ip;
	this->port = port;
	this->conn = socket(AF_INET, SOCK_STREAM, 0);
}

GAServerConnection::~GAServerConnection()
{
	this->Close();
}

bool GAServerConnection::Connect()
{
	struct sockaddr_in ep;

	memset(&ep, 0, sizeof(ep));
	ep.sin_family = AF_INET;
	ep.sin_addr.S_un.S_addr = inet_addr(this->ip.c_str());
	ep.sin_port = htons(this->port);

	return GameArmor::getSingleton()->getNetworkHooks()->getConnect()(this->conn, (const sockaddr *)&ep, sizeof(ep)) != SOCKET_ERROR;
}

bool GAServerConnection::Setup(string license, string clientmd5)
{
	if (license.size() != 32)
		return false;

	if (clientmd5.size() != 32)
		return false;

	this->license = license;
	this->clientmd5 = clientmd5;

	return true;
}

GAServerDescriptor GAServerConnection::QueryServerInfo(int serverID)
{
	GAServerDescriptor desc;
	memset(&desc, 0, sizeof(desc));

	desc.IP = -1;
	desc.Port = -1;

	GA_PACKET_REQUEST_KEY request_packet;
	GA_PACKET_REQUEST_KEY_REPLY response_packet;

	request_packet.Packet = 0xC00;
	memcpy(&request_packet.License, license.c_str(), 32);
	memcpy(&request_packet.ClientMD5, clientmd5.c_str(), 32);
	request_packet.ServerID = serverID;

	LMFAOCrypt::Encrypt((unsigned char *)&request_packet.ClientMD5, sizeof(GA_PACKET_REQUEST_KEY) - 2);
	
	if (GameArmor::getSingleton()->getNetworkHooks()->getSend()(this->conn, (char *)&request_packet, sizeof(GA_PACKET_REQUEST_KEY), 0) == SOCKET_ERROR)
		return desc;

	if (GameArmor::getSingleton()->getNetworkHooks()->getRecv()(this->conn, (char *)&response_packet, sizeof(GA_PACKET_REQUEST_KEY_REPLY), 0) <= 0)
		return desc;

	if (response_packet.Packet != 0xC02)
		return desc;

	LMFAOCrypt::Decrypt((unsigned char *)&response_packet.Server, sizeof(GA_PACKET_REQUEST_KEY_REPLY) - 2);
	desc = response_packet.Server;

	return desc;
}

void GAServerConnection::Close()
{
	if (this->conn)
	{
		closesocket(this->conn);
	}
}
