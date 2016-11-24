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
*             Authentication Server Structures and Classes 	        *
* ==================================================================*/

#pragma once

#include "config_file.hpp"
#include "tcp_server.hpp"
#include <string>

#include <soci/soci.h>

#include <map>

using namespace std;

/*==============================================================*
* Class: Authorization Server								*
* Author: GreenBox                                              *
* Date: 11/01/12 												*
* Description: Class responsible for general Auth-Server		* 
* informations.                                                 *
**==============================================================*/
class GAServer
{
public:
	struct gamord_config
	{
		// Network
		string			network_bindip;
		unsigned short	network_bindport;
	};

	static void run();
	static int parse_from_client(tcp_connection::pointer cl);

	// Config
	static config_file *gamord_config;

	static struct gamord_config config;

	// Network
	static tcp_server *server;

	// Database
	static soci::session *database;
};
