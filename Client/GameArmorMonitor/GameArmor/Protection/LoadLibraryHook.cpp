#include "..\GameArmor.h"

#include "LoadLibraryHook.h"
#include "..\..\Hook\NCodeHookInstantiation.h"

#include <Windows.h>
#include <malloc.h>

static LoadLibraryHook *Me;

void *__stdcall LoadLibraryAHook(char *lpFileName)
{
	HANDLE fp = CreateFileA(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (fp == INVALID_HANDLE_VALUE)
	{
		// Go to hell if this is larger than 16k
		char buffer[16 * 1024];

		if (!GetEnvironmentVariableA("PATH", buffer, sizeof(buffer)))
			return NULL;

		char *pointer = strtok(buffer, ";");
		while (pointer != NULL)
		{
			char filename[MAX_PATH];
			filename[0] = NULL;

			strcat(filename, pointer);
			strcat(filename, "\\");
			strcat(filename, lpFileName);

			fp = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
			if (fp != INVALID_HANDLE_VALUE)
				break;

			pointer = strtok(NULL, ";");
		}
	}

	if (fp == INVALID_HANDLE_VALUE)
		return NULL;

	bool canLoad = Me->CheckMemory(fp);
	CloseHandle(fp);

	if (canLoad)
		return Me->getLla()(lpFileName);
	else
		return NULL;
}

void *__stdcall LoadLibraryWHook(wchar_t *lpFileName)
{
	HANDLE fp = CreateFileW(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (fp == INVALID_HANDLE_VALUE)
	{
		// Go to hell if this is larger than 16k
		wchar_t buffer[16 * 1024];

		if (!GetEnvironmentVariableW(L"PATH", buffer, sizeof(buffer)))
			return NULL;

		wchar_t *pointer = wcstok(buffer, L";");
		while (pointer != NULL)
		{
			wchar_t filename[MAX_PATH];
			filename[0] = NULL;

			wcscat(filename, pointer);
			wcscat(filename, L"\\");
			wcscat(filename, lpFileName);

			fp = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
			if (fp != INVALID_HANDLE_VALUE)
				break;

			pointer = wcstok(NULL, L";");
		}
	}

	if (fp == INVALID_HANDLE_VALUE)
		return NULL;

	bool canLoad = Me->CheckMemory(fp);
	CloseHandle(fp);

	if (canLoad)
		return Me->getLlw()(lpFileName);
	else
		return NULL;
}

void *__stdcall LoadLibraryExAHook(char *lpFileName, void *hFile, unsigned int dwFlags)
{
	bool canLoad = true;

	if (lpFileName)
	{
		HANDLE fp = CreateFileA(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (fp == INVALID_HANDLE_VALUE)
		{
			// Go to hell if this is larger than 16k
			char buffer[16 * 1024];

			if (!GetEnvironmentVariableA("PATH", buffer, sizeof(buffer)))
				return NULL;
			
			char *pointer = strtok(buffer, ";");
			while (pointer != NULL)
			{
				char filename[MAX_PATH];
				filename[0] = NULL;

				strcat(filename, pointer);
				strcat(filename, "\\");
				strcat(filename, lpFileName);

				fp = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
				if (fp != INVALID_HANDLE_VALUE)
					break;

				pointer = strtok(NULL, ";");
			}
		}

		if (fp == INVALID_HANDLE_VALUE)
			return NULL;

		canLoad &= Me->CheckMemory(fp);
		
		CloseHandle(fp);
	}

	if (hFile)
	{
		canLoad &= Me->CheckMemory(hFile);
	}

	if (canLoad)
		return Me->getLlexa()(lpFileName, hFile, dwFlags);
	else
		return NULL;
}

void *__stdcall LoadLibraryExWHook(wchar_t *lpFileName, void *hFile, unsigned int dwFlags)
{
	bool canLoad = true;

	if (lpFileName)
	{
		HANDLE fp = CreateFileW(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		
		if (fp == INVALID_HANDLE_VALUE)
		{
			// Go to hell if this is larger than 16k
			wchar_t buffer[16 * 1024];

			if (!GetEnvironmentVariableW(L"PATH", buffer, sizeof(buffer)))
				return NULL;
			
			wchar_t *pointer = wcstok(buffer, L";");
			while (pointer != NULL)
			{
				wchar_t filename[MAX_PATH];
				filename[0] = NULL;

				wcscat(filename, pointer);
				wcscat(filename, L"\\");
				wcscat(filename, lpFileName);

				fp = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
				if (fp != INVALID_HANDLE_VALUE)
					break;
				
				pointer = wcstok(NULL, L";");
			}
		}

		if (fp == INVALID_HANDLE_VALUE)
			return NULL;

		canLoad &= Me->CheckMemory(fp);
		
		CloseHandle(fp);
	}

	if (hFile)
	{
		canLoad &= Me->CheckMemory(hFile);
	}

	if (canLoad)
		return Me->getLlexw()(lpFileName, hFile, dwFlags);
	else
		return NULL;
}

LoadLibraryHook::LoadLibraryHook()
{
	Me = this;
}

LoadLibraryHook::~LoadLibraryHook()
{
}

void LoadLibraryHook::Boot()
{
	NCodeHookIA32 *hooks = GameArmor::getSingleton()->getHooks();

	lla = hooks->createHook<LoadLibraryAPrototype>((LoadLibraryAPrototype)LoadLibraryA, (LoadLibraryAPrototype)LoadLibraryAHook);
	llw = hooks->createHook<LoadLibraryWPrototype>((LoadLibraryWPrototype)LoadLibraryW, (LoadLibraryWPrototype)LoadLibraryWHook);
	llexa = hooks->createHook<LoadLibraryExAPrototype>((LoadLibraryExAPrototype)LoadLibraryExA, (LoadLibraryExAPrototype)LoadLibraryExAHook);
	llexw = hooks->createHook<LoadLibraryExWPrototype>((LoadLibraryExWPrototype)LoadLibraryExW, (LoadLibraryExWPrototype)LoadLibraryExWHook);
}

bool LoadLibraryHook::CheckMemory(void *hFile)
{
#ifdef _DEBUG
	return true;
#else
	bool result = false;
	DWORD highSize = 0;
	DWORD lowSize = GetFileSize(hFile, &highSize);
	char *mem = (char *)malloc(lowSize);
	int read = 0, totalRead = 0;

	do
	{
		if (!ReadFile(hFile, &mem[totalRead], 4096, (LPDWORD)&read, NULL))
			break;

		totalRead += read;
	}
	while (read > 0 && totalRead < lowSize);

	int gravity = GameArmor::getSingleton()->getPatternDB()->ScanForPattern(mem, &mem[lowSize]);
	switch(gravity)
	{
		case 1:
			ShowErrorMsg("HTD");
			GameArmor::getSingleton()->NotifyHackTool();
			break;
		case -1:
			result = true;
			break;
	}

	if (mem)
		free(mem);

	return result;
#endif
}
