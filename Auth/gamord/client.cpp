/*==================================================================*
*     ___ _           _           _          _       _				*
*    / __(_)_ __ ___ | |__  _   _| |_      _(_)_ __ | |_ ___ _ __	*
*   / _\ | | '_ ` _ \| '_ \| | | | \ \ /\ / / | '_ \| __/ _ \ '__|	*
*  / /   | | | | | | | |_) | |_| | |\ V  V /| | | | | ||  __/ |		*
*  \/    |_|_| |_| |_|_.__/ \__,_|_| \_/\_/ |_|_| |_|\__\___|_|		*
*																	*
* ------------------------------------------------------------------*
*							 Emulator   			                *
* ------------------------------------------------------------------*
*                     Licenced under GNU GPL v3                     *
* ----------------------------------------------------------------- *
*                       Client to Auth Modules 	               	    *
* ==================================================================*/

#include "gamord.hpp"

#include "show_message.hpp"
#include "database_helper.h"
#include "timers.hpp"
#include <iostream>

#include "md5.hpp"
#include "strfuncs.hpp"
#include <string>

using namespace soci;
using namespace std;

void do_the_wigle_man(unsigned char *buff, int size)
{
	for (int i = 0; i < size; i++)
	{
		buff[i] ^= size;
		buff[i] = ((buff[i] >> 4) & 0x0f) | ((buff[i] << 4) & 0xf0);
		buff[i] ^= i;
		buff[i] = ((buff[i] >> 4) & 0x0f) | ((buff[i] << 4) & 0xf0);
	}
}

void i_do_the_wigle_man(unsigned char *buff, int size)
{
	for (int i = 0; i < size; i++)
	{
		buff[i] = ((buff[i] >> 4) & 0x0f) | ((buff[i] << 4) & 0xf0);
		buff[i] ^= i;
		buff[i] = ((buff[i] >> 4) & 0x0f) | ((buff[i] << 4) & 0xf0);
		buff[i] ^= size;
	}
}

/*! 
 *  \brief     Parse Informations From Client ( Auth )
 *  
 *  \author    GameArmor Development Team
 *  \author    GreenBox
 *  \date      08/12/11
 *
 **/
int GAServer::parse_from_client(tcp_connection::pointer cl)
{
	try
	{
		if (cl->flags.eof)
		{
			cl->do_close();
			return 0;
		}

		while(RFIFOREST(cl) >= 2)
		{
			unsigned short cmd = RFIFOW(cl, 0);

			switch(cmd)
			{
				// Request Encryption Key
			case 0xA00:
				if (RFIFOREST(cl) < 34)
					return 0;
				{
					int exp = 1;
					char key[33];

					i_do_the_wigle_man(RFIFOP(cl, 2), 32);
					memcpy(key, RFIFOP(cl, 2), 32);
					key[32] = 0;

					string skey = key;

					statement s = (database->prepare << "SELECT `expiration` FROM \
						`servers` WHERE `licensekey`=:lic",
						use(skey), 
						into(exp));

					s.execute(true);

					if (s.get_affected_rows() <= 0 || (exp != 0 && exp < time(NULL)))
					{
						WFIFOHEAD(cl, 3);
						WFIFOW(cl, 0) = 0xA01;
						WFIFOB(cl, 2) = 0x1;
						do_the_wigle_man(WFIFOP(cl, 2), 1);
						cl->send_buffer(3);
						cl->set_eof();
						return 0;
					}

					WFIFOHEAD(cl, 34);
					WFIFOW(cl, 0) = 0xA02;

					do_the_wigle_man((unsigned char *)key + 00, 32);
					do_the_wigle_man((unsigned char *)key + 00, 16);
					do_the_wigle_man((unsigned char *)key + 16, 16);
					i_do_the_wigle_man((unsigned char *)key, 32);
					do_the_wigle_man((unsigned char *)key + 00, 8);
					do_the_wigle_man((unsigned char *)key + 8, 8);
					do_the_wigle_man((unsigned char *)key + 16, 8);
					do_the_wigle_man((unsigned char *)key + 24, 8);
					do_the_wigle_man((unsigned char *)key + 00, 32);

					memcpy(WFIFOP(cl, 2), key, 32);
					do_the_wigle_man(WFIFOP(cl, 2), 32);

					cl->send_buffer(34);
					cl->set_eof();
					return 0;
				}
				break;
				// Request Server Info
			case 0xC00:
				if (RFIFOREST(cl) < 70)
					return 0;
				{
					int exp = 0;
					char md5[33];
					char key[33];
					int id;
					string ip;
					int port;

					i_do_the_wigle_man(RFIFOP(cl, 2), 68);

					memcpy(md5, RFIFOP(cl, 2), 32);
					memcpy(key, RFIFOP(cl, 34), 32);
					md5[32] = 0;
					key[32] = 0;

					id = RFIFOL(cl,66);

					string skey = key;
					string smd5 = md5;

					statement s = (database->prepare << "SELECT `expiration`, `ip`, \
						`port` FROM `servers` WHERE `hexedmd5`=:m AND \
						`licensekey`=:l AND `sid`=:i",
						use(smd5), use(skey), 
						use(id), into(exp), into(ip), into(port));

					s.execute(true);
					
					if (s.get_affected_rows() <= 0 || (exp != 0 && exp < time(NULL)))
					{
						WFIFOHEAD(cl, 3);
						WFIFOW(cl, 0) = 0xC01;
						WFIFOB(cl, 2) = 0x1;
						do_the_wigle_man(WFIFOP(cl, 2), 1);
						cl->send_buffer(3);
						cl->set_eof();
						return 0;
					}

					WFIFOHEAD(cl, 34);
					WFIFOW(cl, 0) = 0xC02;

					do_the_wigle_man((unsigned char *)key + 00, 32);
					do_the_wigle_man((unsigned char *)key + 00, 16);
					do_the_wigle_man((unsigned char *)key + 16, 16);
					i_do_the_wigle_man((unsigned char *)key, 32);
					do_the_wigle_man((unsigned char *)key + 00, 8);
					do_the_wigle_man((unsigned char *)key + 8, 8);
					do_the_wigle_man((unsigned char *)key + 16, 8);
					do_the_wigle_man((unsigned char *)key + 24, 8);
					do_the_wigle_man((unsigned char *)key + 00, 32);

					memcpy(WFIFOP(cl, 2), key, 32);

					WFIFOL(cl, 34) = inet_addr(ip.c_str());
					WFIFOW(cl, 38) = htons(port);

					do_the_wigle_man(WFIFOP(cl, 2), 38);
					cl->send_buffer(40);
					cl->set_eof();
					return 0;
				}
				break;
			default:
				ShowWarning("Unknown packet 0x%04x sent from %s, closing connection.\n", cmd, cl->socket().remote_endpoint().address().to_string().c_str());
				cl->set_eof();
				return 0;
			}
		}
	}
	catch (...)
	{
		ShowError("Exception caught on parse_from_client\n");
	}
	return 0;
}
