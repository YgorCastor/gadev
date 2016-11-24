#pragma once

#include <Windows.h>
#include "PEAnalyser.h"
#include "GALS.h"

typedef FARPROC (WINAPI *GETPROCADDRESS)(HMODULE, LPCSTR);
typedef HMODULE (WINAPI *LOADLIBRARY)(LPCSTR);

enum
{
	PEI_STR_KERNEL32,
	PEI_STR_VIRTUAL_ALLOC,
	PEI_STR_VIRTUAL_FREE,
	PEI_STR_VIRTUAL_PROTECT,
};

typedef struct 
{
	unsigned int Magic;
	unsigned int ImageBase;
	unsigned int OEP;

	LOADLIBRARY LoadLibraryPtr;
	GETPROCADDRESS GetProcAddressPtr;

	unsigned int GALSChecksum;
	unsigned int GALSKey[8];
	char Strings[4][16];
} PEInjectHeader;

class PEInject
{
public:
	PEInject();
	~PEInject();

	int InjectFile(char *input, char *output, char *dll, void *extraData, DWORD extraDataSize);

private:
	CPELibrary inputPE;
	GALS dllGALS;

	void DoInjection(void *extraData, DWORD extraDataSize);
};
