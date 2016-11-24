#pragma once

#define _WINSOCKAPI_
#include <WinSock2.h>
#include <Windows.h>

#include "Kernel\TDriver.h"

#include "..\..\Common\IniReader.h"

#include "Network\NetworkHooks.h"
#include "Network\PacketLen.h"

#include "Protection\LoadLibraryHook.h"
#include "Protection\RegisterClassHook.h"
#include "Protection\PatternDB.h"

#include "Addons\CustomNickColor.h"

#include "Content\GRFManager.h"

#include "..\Hook\NCodeHookInstantiation.h"

#include <string>

using namespace std;

class GameArmor
{
public:
	GameArmor();
	~GameArmor();

	void ReadConfig();
	void Boot();
	void NotifyHackTool();
	void ErrorMessageBox(char *fmt, ...);

	NCodeHookIA32 *getHooks()
	{
		return hooks;
	}
	
	PacketLen *getPacketLen()
	{
		return packetLen;
	}

	NetworkHooks *getNetworkHooks()
	{
		return networkHooks;
	}

	CustomNickColor *getNickColor()
	{
		return nickColor;
	}

	RegisterClassHook *getRegisterClassHook()
	{
		return rch;
	}

	PatternDB *getPatternDB()
	{
		return pattern;
	}

	TDriver *getDriver()
	{
		return driver;
	}

	string &getLicenseKey()
	{
		return licenseKey;
	}

	string &getClientMD5()
	{
		return clientMD5;
	}

	string getString(char *id)
	{
		return string(lang->ReadString("GameArmor", id, ""));
	}

	char *getIniPath()
	{
		return IniPath;
	}

	static GameArmor *getSingleton()
	{
		return singleton;
	}

	void ShowSplash();

	HWND SplashHwnd;
	LRESULT CALLBACK	SplashWndProc(HWND, UINT, WPARAM, LPARAM);
	BOOL				InitSplashInstance(HINSTANCE, int);

private:
	NCodeHookIA32 *hooks;
	NetworkHooks *networkHooks;
	PacketLen *packetLen;
	CustomNickColor *nickColor;
	LoadLibraryHook *llh;
	RegisterClassHook *rch;
	PatternDB *pattern;
	TDriver *driver;
	GRFManager *grf;

	char BinPath[MAX_PATH];
	char IniPath[MAX_PATH];
	char LngPath[MAX_PATH];
	char BmpPath[MAX_PATH];
	char DrvPath[MAX_PATH];

	string licenseKey;
	string clientMD5;
	string configClientMD5;
	
	HBITMAP SplashBitmap;

	CIniReader *lang;

	HANDLE singleInstance;
	
	static GameArmor *singleton;
};

#define ShowErrorMsg(fmt, ...) GameArmor::getSingleton()->ErrorMessageBox(fmt, __VA_ARGS__)
