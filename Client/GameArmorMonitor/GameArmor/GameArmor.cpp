#include "GameArmor.h"

#include "..\..\Common\md5.h"
#include "..\Utils\MemoryTools.h"
#include "..\resource.h"
#include "..\Utils\LMFAOCrypt.h"

#include <Windows.h>
#include <TlHelp32.h>
#include <Shellapi.h>

#include <stdio.h>

GameArmor *GameArmor::singleton;
extern HINSTANCE OurInstance;

DWORD CALLBACK GameArmorSplash(LPVOID ARG)
{
	GameArmor::getSingleton()->ShowSplash();

	return 0;
}

LRESULT CALLBACK ISplashWndProc(HWND h, UINT u, WPARAM w, LPARAM l)
{
	return GameArmor::getSingleton()->SplashWndProc(h, u, w, l);
}

DWORD GetCurrentModuleSize()
{
    HANDLE hSnap;
    MODULEENTRY32 xModule;
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    xModule.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnap, &xModule))
	{
        CloseHandle(hSnap);

        return (DWORD)xModule.modBaseSize;
    }

    CloseHandle(hSnap);

    return 0;
}

GameArmor::GameArmor()
{
	GameArmor::singleton = this;

	hooks = new NCodeHookIA32(true);
	llh = new LoadLibraryHook();
	rch = new RegisterClassHook();
	networkHooks = new NetworkHooks();
	packetLen = new PacketLen();
	nickColor = new CustomNickColor();
	pattern = new PatternDB();
	grf = new GRFManager();
	//driver = new TDriver();
}

GameArmor::~GameArmor()
{
	GameArmor::singleton = NULL;

	CloseHandle(singleInstance);

	delete hooks;
	delete networkHooks;
	delete packetLen;
	delete nickColor;
	delete llh;
	delete rch;
	delete pattern;
	delete grf;
	//delete driver;

	ExitProcess(0);
	TerminateProcess(GetCurrentProcess(), 0);
}

void GameArmor::ReadConfig()
{
	CIniReader ini(IniPath);
	licenseKey = ini.ReadString("GameArmor", "HASH", "NULL");
	configClientMD5 = ini.ReadString("GameArmor", "HASH2", "NULL");

	lang = new CIniReader(LngPath);

	// Calculates client checksum
	clientMD5 = md5file(BinPath);
	 
	char temp[32];
	memcpy(temp, clientMD5.c_str(), 32);
	LMFAOCrypt::Encrypt((unsigned char *)temp, 32);
	LMFAOCrypt::Encrypt((unsigned char *)temp, 32);
	LMFAOCrypt::Encrypt((unsigned char *)&temp[16], 16);
	
	MD5 checkMd5;
	checkMd5.update(temp, 32);
	checkMd5.finalize();

	string finalMD5 = checkMd5.hexdigest();

	if (finalMD5 != configClientMD5)
	{
		ShowErrorMsg("MCCS");
		NotifyHackTool();
	}
}

void GameArmor::Boot()
{
	int len;

	GetModuleFileName(NULL, BinPath, MAX_PATH);
	GetModuleFileName(NULL, IniPath, MAX_PATH);
	GetModuleFileName(NULL, LngPath, MAX_PATH);
	GetModuleFileName(NULL, BmpPath, MAX_PATH);
	
	len = strlen(BinPath);

	IniPath[len - 1] = 'i';
	IniPath[len - 2] = 'n';
	IniPath[len - 3] = 'i';

	LngPath[len - 1] = 'g';
	LngPath[len - 2] = 'n';
	LngPath[len - 3] = 'l';

	BmpPath[len - 1] = 'p';
	BmpPath[len - 2] = 'm';
	BmpPath[len - 3] = 'b';

	CreateThread(NULL, NULL, GameArmorSplash, NULL, NULL, NULL);
	Sleep(0);

	while (SplashHwnd == NULL)
		Sleep(0);

	InitializeMemoryTools((char *)GetModuleHandle(NULL), (char *)((DWORD)GetModuleHandle(NULL) + GetCurrentModuleSize()));
	ReadConfig();

	/*GetCurrentDirectory(MAX_PATH, DrvPath);
	strcat(DrvPath, "\\crsoul64.sys");

	driver->LoadDriver("GameArmorService", DrvPath, "\\\\.\\GameArmor");
	driver->SetRemovable(TRUE);
	driver->StartDriver();
	driver->OpenDevice();*/
	
#ifndef _DEBUG
	singleInstance = CreateMutex(NULL, FALSE, "Global\\{C09C486B-1DD2-4CCB-860B-2ED56E1331B9}");
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		ExitProcess(0);
		TerminateProcess(GetCurrentProcess(), 0);
		return;
	}
#endif

	llh->Boot();
	rch->Boot();
	packetLen->Boot();
	networkHooks->Boot();
	nickColor->Boot();
	grf->Boot();

	SendMessage(SplashHwnd, WM_DESTROY, 0, 0);
}

void GameArmor::ErrorMessageBox(char *fmt, ...)
{
	char buffer[2048];

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, getString(fmt).c_str(), args);
	va_end(args);

	MessageBoxA(NULL, buffer, "GameArmor", 0);
}

void GameArmor::NotifyHackTool()
{
	ExitProcess(0);
	TerminateProcess(GetCurrentProcess(), 0);
}

BOOL GameArmor::InitSplashInstance(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
	wcex.lpfnWndProc	= ISplashWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= 0;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= "gaSplash";
	wcex.hIconSm		= 0;

	RegisterClassEx(&wcex);

	HWND tmp;

	BITMAP bmp = { 0 };
	GetObject(SplashBitmap, sizeof(BITMAP), (LPVOID)&bmp);

	int x = ((GetSystemMetrics(SM_CXSCREEN) / 2) - (bmp.bmWidth / 2));
	int y = ((GetSystemMetrics(SM_CYSCREEN) / 2) - (bmp.bmHeight / 2));

	tmp = CreateWindow("gaSplash", "", WS_POPUP,
		x, y, bmp.bmWidth, bmp.bmHeight, NULL, NULL, hInstance, NULL);

	if (!tmp)
	{
		return FALSE;
	}

	ShowWindow(tmp, SW_HIDE);
	UpdateWindow(tmp);

	SplashHwnd = CreateWindow("gaSplash", "", WS_POPUP,
		x, y, bmp.bmWidth, bmp.bmHeight, tmp, NULL, hInstance, NULL);

	if (!SplashHwnd)
	{
		return FALSE;
	}

	ShowWindow(SplashHwnd, nCmdShow);
	UpdateWindow(SplashHwnd);

	tmp = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_BITMAP,
		0, 0, bmp.bmWidth, bmp.bmHeight, SplashHwnd, NULL, hInstance, NULL);

	if (!tmp)
	{
		return FALSE;
	}

	ShowWindow(tmp, nCmdShow);
	SendMessage(tmp, STM_SETIMAGE, WPARAM(IMAGE_BITMAP), LPARAM(SplashBitmap));
	UpdateWindow(tmp);

	return TRUE;
}

LRESULT CALLBACK GameArmor::SplashWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		break;
	case WM_DESTROY:
		Sleep(1000);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void GameArmor::ShowSplash()
{
	MSG msg;
	HINSTANCE hinst = GetModuleHandle(NULL);

	SplashBitmap = (HBITMAP)LoadImage(hinst, BmpPath, IMAGE_BITMAP, 
		0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	int err = GetLastError();
	
	if (!InitSplashInstance(OurInstance, SW_SHOW))
		return;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
