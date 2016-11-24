#include "PacketHandler.h"

#include <math.h>
#include <malloc.h>
#include "../GameArmor.h"
#include "../../Crypt/Rijndael.h"
#include "../../Crypt/crypton1.h"

PacketHandler::PacketHandler(void)
{
	memset(&tempPacket, 0, sizeof(tempPacket));
}

PacketHandler::~PacketHandler(void)
{
}

void PacketHandler::ChangeServer()
{
	gameguardChallenged = false;
	gamearmorChallenged = false;

	state = CONNECTION_NEW;

	if (tempPacketFullSize)
	{
		free(tempPacket.Buffer);

		tempPacket.Buffer = 0;
		tempPacket.Size = 0;

		tempPacketFullSize = 0;
	}
}

void PacketHandler::Boot(char *key)
{
	static u4byte storageEncryptKey[] = { 0x050B6F79, 0x0202C179, 0x0E20120, 0x04FA43E3, 0x0179B6C8, 0x05973DF2, 0x07D8D6B, 0x08CB9ED9 };

	PacketLen *pl = GameArmor::getSingleton()->getPacketLen();

	blowfish = new CBlowFish((unsigned char *)key, 32);

	pl->SetPacketLen(0xA258, 2);
	pl->SetPacketLen(0xA259, 2);

	crypton_set_key(storageEncryptKey, 32);
}

void PacketHandler::EnqueueBytes(char *buff, int size)
{
	PacketLen *pl = GameArmor::getSingleton()->getPacketLen();

	char recvBuff[20 * 1024];
	int recvLen = 0;
	int buffPos = 0;

	if (tempPacketFullSize > 0 && tempPacket.Size <= 3)
	{
		memcpy(recvBuff, tempPacket.Buffer, tempPacket.Size);
		
		recvLen += tempPacket.Size;

		tempPacketFullSize = 0;
		free(tempPacket.Buffer);
	}

	memcpy(&recvBuff[recvLen], buff, size);
	recvLen += size;

	if (tempPacketFullSize > 0)
	{
		if (tempPacket.Size + recvLen < tempPacketFullSize)
		{
			memcpy(&tempPacket.Buffer[tempPacket.Size], recvBuff, recvLen);

			tempPacket.Size += recvLen;
			buffPos += recvLen;
		}
	}

	if (state == CONNECTION_NEW && buffPos != recvLen)
	{
		int tempLen = pl->GetPacketLen(BUFW(recvBuff, buffPos));

		if (tempLen == 1 && recvLen - buffPos >= 4)
			tempLen = BUFW(recvBuff, buffPos + 2);

		if (tempLen == 0)
		{
			state = CONNECTION_OK;
			aid = BUFL(recvBuff, buffPos);

			Packet p;
			p.Buffer = (char *)malloc(4);
			p.Size = 4;

			memcpy(p.Buffer, &aid, 4);

			packetQueue.push_back(p);
			buffPos += 4;
		}
	}
	else if (state == CONNECTION_OK && buffPos != recvLen)
	{
		if (BUFL(recvBuff, buffPos) == aid)
		{
			Packet p;
			p.Buffer = (char *)malloc(4);
			p.Size = 4;

			memcpy(p.Buffer, &aid, 4);

			packetQueue.push_back(p);
			buffPos += 4;
		}
	}

	while (buffPos < recvLen)
	{
		unsigned short cmd = BUFW(recvBuff, buffPos);
		int thisLen = pl->GetPacketLen(cmd);

		if ((thisLen == 1) && ((recvLen - buffPos) >= 4))
			thisLen = BUFW(recvBuff, buffPos + 2);

		if (thisLen <= 1)
		{
			if ((recvLen - buffPos) <= 3)
			{
				tempPacket.Buffer = (char *)malloc(recvLen - buffPos);
				tempPacket.Size = recvLen - buffPos;

				memcpy(tempPacket.Buffer, &recvBuff[buffPos], tempPacket.Size);

				tempPacketFullSize = 1;

				buffPos = recvLen;
				break;
			}
			else
			{
				Packet p;

				p.Buffer = (char *)malloc(recvLen - buffPos);
				p.Size = recvLen - buffPos;

				memcpy(p.Buffer, &recvBuff[buffPos], p.Size);

				buffPos = recvLen;
				break;
			}
		}
			
		if ((buffPos + thisLen) > recvLen)
		{
			tempPacket.Size = recvLen - buffPos;
			tempPacket.Buffer = (char *)malloc(tempPacket.Size);

			memcpy(tempPacket.Buffer, &recvBuff[buffPos], tempPacket.Size);
			
			tempPacketFullSize = thisLen;
			break;
		}
		else
		{
			if (ProcessPackets(&recvBuff[buffPos], thisLen))
			{
				Packet p;

				p.Size = thisLen;
				p.Buffer = (char *)malloc(p.Size);

				memcpy(p.Buffer, &recvBuff[buffPos], thisLen);

				packetQueue.push_back(p);
			}

			buffPos+=thisLen;
		}
	}
}

int PacketHandler::ReceiveSomePackets(char *buff, int size)
{
	int buffPos = 0;

	while (packetQueue.size())
	{
		if (buffPos >= size)
			break;

		Packet next = packetQueue.front();
		packetQueue.pop_front();

		if ((buffPos + next.Size) > size)
		{
			int partialLen = size - buffPos;

			memcpy(&buff[buffPos], next.Buffer, partialLen);
			buffPos += partialLen;

			next.Size -= partialLen;
			memcpy(next.Buffer, &next.Buffer[partialLen], next.Size);
			packetQueue.push_front(next);

			break;
		}

		memcpy(&buff[buffPos], next.Buffer, next.Size);

		buffPos += next.Size;

		free(next.Buffer);
	}

	return buffPos;
}

void PacketHandler::EnqueueSend(char *buff, int size)
{
	PacketLen *pl = GameArmor::getSingleton()->getPacketLen();
	
	char *pkt = buff;
	while (size > 0)
	{
		unsigned short cmd = BUFW(pkt, 0);
		int cmdsize = pl->GetPacketLen(cmd);

		if (cmdsize == 1)
			cmdsize = BUFW(pkt, 2);
					
		if (ProcessPackets(pkt, cmdsize))
		{
			Packet p;
			p.Buffer = (char *)malloc(cmdsize);
			p.Size = cmdsize;
			memcpy(p.Buffer, pkt, cmdsize);
			sendQueue.push_back(p);
		}

		size -= cmdsize;
		pkt = (char *)BUFP(pkt, cmdsize);
	}
}

int PacketHandler::FlushSendQueue(int sock)
{
	NetworkHooks *nh = GameArmor::getSingleton()->getNetworkHooks();
	int sent = 0;

	while (sendQueue.size() > 0)
	{
		Packet &p = sendQueue.front();
		int sented;

		if ((sented = nh->getSend()(sock, p.Buffer, p.Size, 0)) < p.Size)
		{
			if (sented == -1)
				return -1;

			p.Size -= sented;
			memmove(p.Buffer, BUFP(p.Buffer, sented), p.Size);
			sent += sented;

			break;
		}
		
		free(p.Buffer);
		sendQueue.pop_front();

		sent += p.Size;
	}

	return sent;
}

void PacketHandler::SendPacketToClient(char *buff, int size)
{
	Packet p;
	
	p.Buffer = (char *)malloc(size);
	p.Size = size;

	memcpy(p.Buffer, buff, size);

	packetQueue.push_back(p);
}

bool PacketHandler::ProcessPackets(char *buff, int len)
{
	bool result = true;
	unsigned short cmd = BUFW(buff, 0);

	switch (cmd)
	{
		// LoginPacket
	case 0x02b0:
		if (gameguardChallenged) // bRO Client
		{
			static unsigned char key[24] = { 6, 169, 33, 64, 54, 184, 161, 91, 81, 46, 3, 213, 52, 18, 0, 6, 61, 175, 186, 66, 157, 158, 180, 48 };
			static unsigned char chain[24] = { 61, 175, 186, 66, 157, 158, 180, 48, 180, 34, 218, 128, 44, 159, 172, 65, 1, 2, 4, 8, 16, 32, 128 };

			CRijndael aes;
			aes.MakeKey((char *)key, (char *)chain, 24, 24);
			aes.Decrypt((const char *)BUFP(buff, 30), (char *)BUFP(buff, 30), 24, 0);
		}
	case 0x0064:
	case 0x0277:
	case 0x01dd:
	case 0x01fa:
	case 0x027c:
	case 0x0825:
		// WantToConnect
	case 0x0072:
	case 0x007e:
	case 0x00f5:
	case 0x009b:
	case 0x0363:
	case 0x0436:
	case 0x022d:
	case 0x083c:
	case 0x0835:
	case 0x0361:
	case 0x0930:
		{
			if (!gamearmorChallenged)
			{
				char tmp[2];
			
				BUFW(tmp, 0) = 0xA258;

				EnqueueSend(tmp, 2);
			
				loginPacket.Buffer = (char *)malloc(len);
				loginPacket.Size = len;

				memcpy(loginPacket.Buffer, buff, len);

				result = false;
			}
		}
		break;

		// bRO Client StorageEncryptKey
	case 0x023b:
	case 0x087f:
		{
			// This is not a storagepassword packet
			if (len != 36)
				break;
			
			char str[32] = { 0 };
			u4byte tmp[4] = { 0 };

			memset(str, 0, sizeof(str));
			memset(tmp, 0, sizeof(tmp));

			crypton_decrypt((const u4byte *)BUFP(buff, 4), (u4byte *)&tmp);

			sprintf(str, "%d", tmp[0]);
			memcpy(BUFP(buff, 4), &str[1], 8);
			BUFB(buff, 12) = 0;

			crypton_decrypt((const u4byte *)BUFP(buff, 20), (u4byte *)&tmp);
			
			sprintf(str, "%d", tmp[0]);
			memcpy(BUFP(buff, 20), &str[1], 8);
			BUFB(buff, 28) = 0;

			//blowfish->Encrypt((unsigned char *)BUFP(buff, 4), len - 4, CBlowFish::ECB);
		}
		break;

	case 0xA259:
		{
			blowfish->Encrypt((unsigned char *)loginPacket.Buffer + 4, loginPacket.Size - 4, CBlowFish::ECB);

			sendQueue.push_back(loginPacket);

			gamearmorChallenged = true;
			result = false;
		}
		break;

		// Emulates GameGuard challenges
	case 0x0258:
		{
			char buff[3];
			BUFW(buff, 0) = 0x259;
			BUFB(buff, 2) = gameguardChallenged ? 2 : 1;

			gameguardChallenged = true;

			SendPacketToClient(buff, 3);

			result = false;
		}
		break;

		// Colorize Nickname
	case 0xA00:
		{
			GameArmor::getSingleton()->getNickColor()->AddNickColor(BUFL(buff, 2), BUFB(buff, 6), BUFB(buff, 7), BUFB(buff, 8));
			result = false;
		}
		break;
	}

	return result;
}
