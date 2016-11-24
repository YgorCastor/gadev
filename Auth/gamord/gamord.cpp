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
*                       Authentication Server              	        *
* ==================================================================*/

#include "gamord.hpp"

#include "show_message.hpp"
#include "database_helper.h"
#include "timers.hpp"
#include <iostream>

#include "md5.hpp"
#include "strfuncs.hpp"
#include <boost/foreach.hpp>

// Config
config_file *GAServer::gamord_config;

struct GAServer::gamord_config GAServer::config;

// Network
tcp_server *GAServer::server;

// Database
soci::session *GAServer::database;

/*==============================================================*
* Function:	Auth::Run											*                                                     
* Author: GreenBox                                              *
* Date: 08/12/11 												*
* Description: Start Auth-Server and load configurations        *
**==============================================================*/
void GAServer::run()
{
	boost::asio::io_service io_service;

	// Read Config Files
	try
	{
		gamord_config = new config_file("./gamord.conf");
		{
			config.network_bindip = gamord_config->read<string>("network.bindip", "0.0.0.0");
			config.network_bindport = gamord_config->read<unsigned short>("network.bindport", 6900);
		}
		ShowStatus("Finished reading gamord.conf.\n");
	}
	catch (config_file::file_not_found *fnf)
	{
		ShowFatalError("Config file not found: %s.\n", fnf->filename.c_str());
		return;
	}

	TimerManager::Initialize(&io_service);

	// Initialize Database System
	{
		ShowInfo("Opening connection to database...\n");

		try
		{
			database = database_helper::get_session(gamord_config);
		}
		catch (soci::soci_error err)
		{
			ShowFatalError("Error opening database connection: %s\n", err.what());
			return;
		}

		ShowSQL("Successfully opened database connection.\n");
	}

	// Initialize Network System
	{
		boost::system::error_code err;
		address_v4 bindip = address_v4::from_string(config.network_bindip, err);

		if (err)
		{
			ShowFatalError("%s\n", err.message().c_str());
			return;
		}

		server = new tcp_server((boost::asio::io_service&)io_service, (boost::asio::ip::address&)bindip, (unsigned short)config.network_bindport);
		server->set_default_parser(GAServer::parse_from_client);

		ShowStatus("GAServer is ready and listening on %s:%d.\n", config.network_bindip.c_str(), config.network_bindport);
	}

	// Run IO service service and start pooling events
	io_service.run();
}

int main(int argc, char *argv[])
{
	srand((unsigned int)time(NULL));

	ShowMessage(CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n");
	ShowMessage(CL_XXBL"          ("CL_BT_RED"                    GameArmor Server                     "CL_XXBL")"CL_CLL""CL_NORMAL"\n");
	ShowMessage(CL_WTBL"          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)"CL_CLL""CL_NORMAL"\n\n");

	GAServer::run();

	getchar();
	return 0;
}
