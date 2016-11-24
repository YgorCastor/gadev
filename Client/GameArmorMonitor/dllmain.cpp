#include "GameArmor\GameArmor.h"

HINSTANCE OurInstance;
GameArmor gameArmor;
DWORD OrigEP;

#define EMBED_GA

extern "C" _declspec(dllexport) void _init_agmon()
{

}

VOID CALLBACK StartGameArmor()
{
	gameArmor.Boot();
	
	_asm
	{
		call [OrigEP]
	}

	Sleep(INFINITE);
}
 
BOOL WINAPI DllMain(
  __in  HINSTANCE hinstDLL,
  __in  DWORD fdwReason,
  __in  LPVOID lpvReserved
)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
#ifndef EMBED_GA
				PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
				PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((DWORD)dos + dos->e_lfanew);
				DWORD epa = (DWORD)((DWORD)dos + nt->OptionalHeader.AddressOfEntryPoint) + 6;
				DWORD ep = (*(DWORD*)epa) + epa + 4;
				OrigEP = gameArmor.getHooks()->createHook(ep, (DWORD)StartGameArmor);
#else
				DWORD *tmp = (DWORD*)hinstDLL;
				DWORD epa = *tmp;
				DWORD ep = (*(DWORD*)epa) + epa + 4;
				OrigEP = *tmp;
				*tmp = (DWORD)StartGameArmor;
#endif
				
			OurInstance = GetModuleHandle(NULL);
		}
		break;
	}

	return TRUE;
}
