#include "stdafx.h"
#include "Loader.h"

#include "../Common/IniReader.h"
#include "../Common/md5.h"
#include "../Common/split.h"
#include "../Common/utils.h"
#include "../Common/download.h"

#include <Winhttp.h>

#include <tlhelp32.h>

#define MAXWAIT 10000

using namespace std;

string lecmdline;

// Globals
char ModulePath[MAX_PATH];
char TargetPath[MAX_PATH];
char SplashPath[MAX_PATH];
char ConfigPath[MAX_PATH];
CIniReader *ConfigFile;
//

HANDLE startProcess(LPCSTR process, LPSTR args)
{
	char  hehe[1024];

	sprintf(hehe, "\"%s\" %s", process, args);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if(!CreateProcess(NULL,
		hehe,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi)) 
	{
		return (HANDLE)NULL;
	}

	return pi.hProcess;
}

void enableDebugPriv()
{
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tkp;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);

	CloseHandle(hToken); 
}

bool insertDll(HANDLE hProc, string dll)
{
	//Find the address of the LoadLibrary api, luckily for us, it is loaded in the same address for every process
	HMODULE hLocKernel32 = GetModuleHandle("Kernel32");
	FARPROC hLocLoadLibrary = GetProcAddress(hLocKernel32, "LoadLibraryA");

	//Allocate memory to hold the path to the Dll File in the process's memory
	dll += '\0';
	LPVOID hRemoteMem = VirtualAllocEx(hProc, NULL, dll.size(), MEM_COMMIT, PAGE_READWRITE);

	//Write the path to the Dll File in the location just created
	DWORD numBytesWritten;
	WriteProcessMemory(hProc, hRemoteMem, dll.c_str(), dll.size(), &numBytesWritten);

	//Create a remote thread that starts begins at the LoadLibrary function and is passed are memory pointer
	HANDLE hRemoteThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)hLocLoadLibrary, hRemoteMem, 0, NULL);

	//Wait for the thread to finish
	bool res = false;
	if (hRemoteThread)
		res = (bool)WaitForSingleObject(hRemoteThread, MAXWAIT) != WAIT_TIMEOUT;

	//Free the memory created on the other process
	VirtualFreeEx(hProc, hRemoteMem, dll.size(), MEM_RELEASE);

	//Release the handle to the other process
	CloseHandle(hProc);

	return res;
}

bool insertDllDebug(LPCSTR hProc, string dll)
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	while(true)
	{
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if (Process32First(snapshot, &entry) == TRUE)
		{
			while (Process32Next(snapshot, &entry) == TRUE)
			{
				if (strcmp(entry.szExeFile, hProc) == 0)
				{  
					HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
					Sleep(3000);
					return insertDll(hProcess, dll);

					CloseHandle(hProcess);
				}
			}
		}

		CloseHandle(snapshot);
	}

	return false;
}

void OpenTarget()
{
//#ifdef _DEBUG
	//insertDllDebug(L"AliveROMult.exe", "C:\\Work\\AliveAH\\BitGuard\\Debug\\bgmon.dll");
//#else

	startProcess(TargetPath, (LPSTR)lecmdline.c_str());

	//if (insertDll(proc, "agmon.dll") == FALSE)
	//{
	//	TerminateProcess(proc, -1);
	//	return;
	//}
//#endif
}

HINSTANCE hInst;

// Global
HFONT FontArial;
//

// Updater
HWND UpdaterHwnd;
HWND UpdaterTextHwnd;
HWND UpdaterStatusHwnd;
HWND UpdaterFileHwnd;
HWND UpdaterPbHwnd;
HWND UpdaterCancelHwnd;
LRESULT CALLBACK	UpdaterWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL				InitUpdaterInstance(HINSTANCE, int);
//

// Splash
HBITMAP SplashBitmap;
HWND SplashHwnd;
LRESULT CALLBACK	SplashWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL				InitSplashInstance(HINSTANCE, int);
//

char *DownloadBin(char *domain, char *file, int &sz)
{
	DWORD size = 0;
	BOOL  bResults = FALSE;
	HINTERNET hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	hSession = WinHttpOpen(L"GAMEARMOR UPDATE 1.0", 
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, 
		WINHTTP_NO_PROXY_BYPASS, 0);

	if (hSession)
		hConnect = WinHttpConnect( hSession, StringToWString(domain).c_str(),
		INTERNET_DEFAULT_HTTP_PORT, 0);

	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", 
		StringToWString(file).c_str(), 
		NULL, WINHTTP_NO_REFERER, 
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0);

	if (hRequest) 
		bResults = WinHttpSendRequest(hRequest, 
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0, WINHTTP_NO_REQUEST_DATA, 0, 
		0, 0);

	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	if (bResults)
		bResults =WinHttpQueryDataAvailable(hRequest, &size);

	char *data = (char*)malloc(size+1);

	if (!data)
		return 0;

	if (bResults)
		WinHttpReadData(hRequest, data, size, &size);

	sz = size;
	data[size] = 0;

	if (hRequest) 
		WinHttpCloseHandle(hRequest);

	if (hConnect) 
		WinHttpCloseHandle(hConnect);

	if (hSession) 
		WinHttpCloseHandle(hSession);

	if (bResults)
		return data;
	else
		return 0;
}

char *curfname;
char updbuff[1024];
void showprogress(unsigned long total, unsigned long part)
{
	int val = (int) ((double)part / total * 100);
	SendMessage(UpdaterPbHwnd, PBM_SETPOS, val, 0);
	sprintf(updbuff, "%s %d/%d", curfname, part, total);
	SetWindowText(UpdaterFileHwnd, updbuff);
}

DWORD Worker(DWORD arg)
{
	std::vector<std::string> toUpdate;
	char urlget[256];
	char buff2[256];
	int size = 0;
	
// Verify Files
	sprintf(urlget, "/upd_ga/get.php?code=%s", ConfigFile->ReadString("GameArmor", "HASH", "INVALID"));
	SetWindowText(UpdaterStatusHwnd, "Verificando");

	char *data = DownloadBin("177.71.180.18", urlget, size);

	std::vector<std::string> lines = split(std::string(data), '\n');

	SendMessage(UpdaterPbHwnd, PBM_SETRANGE, 0, MAKELPARAM(0, lines.size()));
	SendMessage(UpdaterPbHwnd, PBM_SETPOS, 0, 0);

	for (int i = 0; i < lines.size(); i++)
	{
		std::vector<std::string> parts = split(std::string(lines[i]), ':');

		if (parts.size() != 3)
			continue;

		std::string fname = parts[0];
		std::string url = parts[1];
		std::string md5 = parts[2];

		char name[255];
		memset(name, 0, 255);
		memset(urlget, 0, sizeof(urlget));
		Download::getfname((char*)fname.c_str(), name);
		sprintf(urlget, "%s", StringToWString(name).c_str());
		SetWindowText(UpdaterFileHwnd, urlget);
		
		if (!fexists(fname.c_str()))
		{
			toUpdate.push_back(lines[i]);
			continue;
		}
		
		if (md5file(fname) != md5)
		{
			toUpdate.push_back(lines[i]);
			continue;
		}

		SendMessage(UpdaterPbHwnd, PBM_SETPOS, i, 0);
	}
	SendMessage(UpdaterPbHwnd, PBM_SETPOS, lines.size(), 0);
	SetWindowText(UpdaterFileHwnd, "");
//

	free(data);

// Update Files
	SetWindowText(UpdaterStatusHwnd, "Atualizando");

	for (int i = 0; i < toUpdate.size(); i++)
	{
		std::vector<std::string> parts = split(std::string(toUpdate[i]), ':');

		std::string fname = parts[0];
		std::string url = parts[1];
		std::string md5 = parts[2];

		memset(buff2, 0, 255);
		sprintf(buff2, "http://177.71.180.18/upd_ga/files/%s", url.c_str());

		char name[255];
		memset(name, 0, 255);
		memset(urlget, 0, sizeof(urlget));
		Download::getfnamewin((char*)fname.c_str(), name);
		curfname = name;
		wsprintf(urlget, "%s", StringToWString(name).c_str());
		SetWindowText(UpdaterFileHwnd, urlget);

		SendMessage(UpdaterPbHwnd, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessage(UpdaterPbHwnd, PBM_SETPOS, 0, 0);

		try
		{
			Download::download(buff2, (char*)fname.c_str(), true, showprogress);
		}
		catch (DLExc e)
		{
			MessageBox(NULL, e.geterr(), "GameArmor - Error", NULL);
			SendMessage(UpdaterHwnd, WM_DESTROY, 0, 0);
			return 0;
		}
		SendMessage(UpdaterPbHwnd, PBM_SETPOS, 100, 0);
	}
	SetWindowText(UpdaterFileHwnd, "");
//

	SetWindowText(UpdaterStatusHwnd, "Pronto");

	OpenTarget();

	SendMessage(UpdaterHwnd, WM_DESTROY, 0, 0);
	return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	DWORD dummy = 0;

	lecmdline = string(lpCmdLine);

	INITCOMMONCONTROLSEX args;
	args.dwICC = ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES;
	args.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&args);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (!GetModuleFileName(NULL, ModulePath, MAX_PATH) || 
		!GetModuleFileName(NULL, TargetPath, MAX_PATH) ||
		!GetModuleFileName(NULL, SplashPath, MAX_PATH) ||
		!GetModuleFileName(NULL, ConfigPath, MAX_PATH))
	{
		return FALSE;
	}

	{
		int len;

		len = strlen(TargetPath);
		TargetPath[len-3] = 'b';
		TargetPath[len-2] = 'i';
		TargetPath[len-1] = 'n';

		len = strlen(SplashPath);
		SplashPath[len-3] = 'b';
		SplashPath[len-2] = 'm';
		SplashPath[len-1] = 'p';

		len = strlen(ConfigPath);
		ConfigPath[len-3] = 'i';
		ConfigPath[len-2] = 'n';
		ConfigPath[len-1] = 'i';
	}

	ConfigFile = new CIniReader(ConfigPath);

	FontArial = CreateFont(12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, 
		FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");

	SplashBitmap = (HBITMAP)LoadImage(hInstance, SplashPath, IMAGE_BITMAP, 
		0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	if (!InitSplashInstance(hInstance, SW_SHOW))
		return FALSE;

	if (!InitUpdaterInstance(hInstance, SW_SHOW))
		return FALSE;

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Worker, NULL, NULL, &dummy);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

#pragma region Updater

BOOL InitUpdaterInstance(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
	wcex.lpfnWndProc	= UpdaterWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= 0;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= "gaUpdater";
	wcex.hIconSm		= 0;

	RegisterClassEx(&wcex);

	HWND tmp;

	hInst = hInstance;

	tmp = CreateWindow("gaUpdater", "", WS_POPUP,
		0, 0, 210, 113, NULL, NULL, hInstance, NULL);

	if (!tmp)
	{
		return FALSE;
	}

	ShowWindow(tmp, SW_HIDE);
	UpdateWindow(tmp);

	UpdaterHwnd = CreateWindow("gaUpdater", "", WS_POPUP,
      0, 0, 210, 113, tmp, NULL, hInstance, NULL);

	if (!UpdaterHwnd)
	{
		return FALSE;
	}

	ShowWindow(UpdaterHwnd, nCmdShow);
	UpdateWindow(UpdaterHwnd);

	UpdaterPbHwnd = CreateWindow(PROGRESS_CLASS, "", WS_CHILD | WS_VISIBLE,
		16, 58, 179, 8, UpdaterHwnd, NULL, hInst, NULL);

	if (!UpdaterPbHwnd)
	{
		return FALSE;
	}

	ShowWindow(UpdaterPbHwnd, nCmdShow);
	UpdateWindow(UpdaterPbHwnd);

	UpdaterCancelHwnd = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE,
		151, 90, 50, 17, UpdaterHwnd, NULL, hInst, NULL);

	if (!UpdaterCancelHwnd)
	{
		return FALSE;
	}

	SetWindowText(UpdaterCancelHwnd, "Cancelar");
	SendMessage(UpdaterCancelHwnd, WM_SETFONT, WPARAM(FontArial), LPARAM(NULL));
	ShowWindow(UpdaterCancelHwnd, nCmdShow);
	UpdateWindow(UpdaterCancelHwnd);

	UpdaterStatusHwnd = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_GROUPBOX | BS_CENTER,
		9, 34, 193, 51, UpdaterHwnd, NULL, hInst, NULL);

	if (!UpdaterStatusHwnd)
	{
		return FALSE;
	}

	SetWindowText(UpdaterStatusHwnd, "Atualizando");
	SendMessage(UpdaterStatusHwnd, WM_SETFONT, WPARAM(FontArial), LPARAM(NULL));
	ShowWindow(UpdaterStatusHwnd, nCmdShow);
	UpdateWindow(UpdaterStatusHwnd);

	UpdaterFileHwnd = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_CENTER,
		16, 66, 179, 17, UpdaterHwnd, NULL, hInst, NULL);

	if (!UpdaterFileHwnd)
	{
		return FALSE;
	}

	SetWindowText(UpdaterFileHwnd, "");
	SendMessage(UpdaterFileHwnd, WM_SETFONT, WPARAM(FontArial), LPARAM(NULL));
	ShowWindow(UpdaterFileHwnd, nCmdShow);
	UpdateWindow(UpdaterFileHwnd);

	UpdaterTextHwnd = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE,
		41, 14, 120, 15, UpdaterHwnd, NULL, hInst, NULL);

	if (!UpdaterTextHwnd)
	{
		return FALSE;
	}

	SetWindowText(UpdaterTextHwnd, "GameArmor Update");
	SendMessage(UpdaterTextHwnd, WM_SETFONT, WPARAM(FontArial), LPARAM(NULL));
	ShowWindow(UpdaterTextHwnd, nCmdShow);
	UpdateWindow(UpdaterTextHwnd);

	tmp = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_ICON,
		17, 11, 16, 16, UpdaterHwnd, NULL, hInst, NULL);

	if (!tmp)
	{
		return FALSE;
	}

	ShowWindow(tmp, nCmdShow);
	SendMessage(tmp, STM_SETIMAGE, WPARAM(IMAGE_ICON), 
		LPARAM(LoadImage(hInst, 
		MAKEINTRESOURCE(IDI_SMALL), IMAGE_ICON,
		16, 16, NULL)));
	UpdateWindow(tmp);

	tmp = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_GRAYFRAME,
		0, 0, 210, 113, UpdaterHwnd, NULL, hInst, NULL);

	if (!tmp)
	{
		return FALSE;
	}

	ShowWindow(tmp, nCmdShow);
	UpdateWindow(tmp);

	ShowWindow(tmp, nCmdShow);
	UpdateWindow(tmp);

	return TRUE;
}

LRESULT CALLBACK UpdaterWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		if (HWND(lParam) == UpdaterCancelHwnd)
		{
			PostQuitMessage(0);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#pragma endregion Updater

#pragma region Splash

BOOL InitSplashInstance(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
	wcex.lpfnWndProc	= SplashWndProc;
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

	hInst = hInstance;

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

	UpdaterHwnd = CreateWindow("gaSplash", "", WS_POPUP,
		x, y, bmp.bmWidth, bmp.bmHeight, tmp, NULL, hInstance, NULL);

	if (!UpdaterHwnd)
	{
		return FALSE;
	}

	ShowWindow(UpdaterHwnd, nCmdShow);
	UpdateWindow(UpdaterHwnd);

	tmp = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_BITMAP,
		0, 0, bmp.bmWidth, bmp.bmHeight, UpdaterHwnd, NULL, hInst, NULL);

	if (!tmp)
	{
		return FALSE;
	}

	ShowWindow(tmp, nCmdShow);
	SendMessage(tmp, STM_SETIMAGE, WPARAM(IMAGE_BITMAP), LPARAM(SplashBitmap));
	UpdateWindow(tmp);

	return TRUE;
}

LRESULT CALLBACK SplashWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#pragma endregion Splash
