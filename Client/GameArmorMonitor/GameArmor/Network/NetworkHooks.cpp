#include "NetworkHooks.h"

#include "../GameArmor.h"
#include "../Server/GAServerConnection.h"
#include "../../Utils/MemoryTools.h"

#include <WinSock2.h>
#include <Windows.h>

#include "..\..\Hook\NCodeHookInstantiation.h"
static NetworkHooks *PointerToMe;

static int __stdcall connectHook(int s, const struct sockaddr *name, int namelen)
{
	return PointerToMe->Connect(s, name, namelen);
}

static int __stdcall selectHook(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval* timeout)
{
	return PointerToMe->Select(nfds, readfds, writefds, exceptfds, timeout);
}

static int __stdcall sendHook(int sock, char *buf, int len, int flags)
{
	return PointerToMe->Send(sock, buf, len, flags);
}

static int __stdcall recvHook(int sock, char *buf, int len, int flags)
{
	return PointerToMe->Receive(sock, buf, len, flags);
}

NetworkHooks::NetworkHooks()
{
	PointerToMe = this;
	hooked = false;
	disconnected = false;

	origConnect = (connectPrototype)connect;
	origSend = (sendPrototype)send;
	origRecv = (recvPrototype)recv;
}

NetworkHooks::~NetworkHooks()
{
}

void NetworkHooks::Boot()
{
	NCodeHookIA32 *hooks = GameArmor::getSingleton()->getHooks();

	origConnect = hooks->createHook<connectPrototype>((connectPrototype)connect, (connectPrototype)connectHook);
	origSend = hooks->createHook<sendPrototype>((sendPrototype)send, (sendPrototype)sendHook);
	origRecv = hooks->createHook<recvPrototype>((recvPrototype)recv, (recvPrototype)recvHook);
	origSelect = hooks->createHook<selectPrototype>((selectPrototype)select, (selectPrototype)selectHook);

	// Disable Packet Header Obfuscation, if present
	unsigned char *pho = (unsigned char *)GaFindPattern("66 31 01 83 7E 04 FF");

	if (pho)
	{
		DWORD old = 0, dummy = 0;
		VirtualProtect((LPVOID)pho, 3, PAGE_EXECUTE_READWRITE, &old);
		*pho++ = 0x90;
		*pho++ = 0x90;	
		*pho++ = 0x90;
		VirtualProtect((LPVOID)pho, 3, old, &dummy);
	}
}

int NetworkHooks::Connect(int sock, const struct sockaddr *name, int namelen)
{
	this->conn = sock;
	this->disconnected = false;

	packetHandler.ChangeServer();

	if (name->sa_family == AF_INET)
	{
		struct sockaddr_in *namein = (struct sockaddr_in *)name;

		if (namein->sin_addr.S_un.S_addr == inet_addr(GATriggerIP))
		{
			GAServerConnection gasc(GAServerIP, GAServerPort);

			if (!gasc.Setup(GameArmor::getSingleton()->getLicenseKey(), GameArmor::getSingleton()->getClientMD5()))
			{
				ShowErrorMsg("CNCGAS", 0);

				return SOCKET_ERROR;
			}
			
			if (!gasc.Connect())
			{
				ShowErrorMsg("CNCGAS", 1);

				return SOCKET_ERROR;
			}

			GAServerDescriptor gasd = gasc.QueryServerInfo(ntohs(namein->sin_port));
			if (gasd.IP == -1 || gasd.Port == -1)
			{
				ShowErrorMsg("CNCGAS", 2);

				return SOCKET_ERROR;
			}
			
			if (!hooked)
			{
				GameArmor::getSingleton()->getPacketLen()->Load();

				hooked = true;
			}

			namein->sin_addr.S_un.S_addr = gasd.IP;
			namein->sin_port = gasd.Port;

			packetHandler.Boot(gasd.Key);
			packetHandler.setState(CONNECTION_OK);
		}
	}

	return origConnect(sock, name, namelen);
}

int NetworkHooks::Select(int nfds, void *readfds, void *writefds, void *exceptfds, const struct timeval* timeout)
{
	fd_set *fds = 0;
	SOCKET sock = 0;

	if (readfds)
		fds = (fd_set *)readfds;
	
	if (fds)
		sock = fds->fd_array[0];

	int sel = origSelect(nfds, readfds, writefds, exceptfds, timeout);

	if (readfds)
	{
		packetHandler.FlushSendQueue(sock);

		if (sel)
		{
			char buf[16 * 1024];
			int received = origRecv(sock, buf, sizeof(buf), 0);

			if (received <= 0)
			{
				disconnected = true;

				return 0;
			}
			
			packetHandler.EnqueueBytes(buf, received);
		}
		
		FD_ZERO(fds);

		if (packetHandler.hasPackets() || disconnected)
		{
			FD_SET(sock, fds);

			return 1;
		}

		if (sel >= 0)
			return 0;
		else
			return sel;
	}
	
	return sel;
}

int NetworkHooks::Send(int sock, char *buf, int len, int flags)
{
	packetHandler.EnqueueSend(buf, len);

	packetHandler.FlushSendQueue(sock);

	return len;
}

int NetworkHooks::Receive(int sock, char *buf, int len, int flags)
{
	if (disconnected == true)
		return 0;

	return packetHandler.ReceiveSomePackets(buf, len);
}
