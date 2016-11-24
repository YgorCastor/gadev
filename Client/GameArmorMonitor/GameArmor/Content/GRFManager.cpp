#include "GRFManager.h"

#include "..\GameArmor.h"
#include "..\..\Utils\LMFAOCrypt.h"
#include "..\..\Utils\MemoryTools.h"
#include "..\..\Crypt\Base64.h"
#include "..\..\..\Common\IniReader.h"

static GRFManager *Me;

static void _stdcall LoadGrfHook(void *ptr)
{
	Me->Load(ptr);
}

static void __declspec(naked) LoadGrfStub()
{
	_asm
	{
		push ecx;
		call LoadGrfHook;
		retn 4;
	}
}

static void CallAddGrf(char *fn, void *me, char *name)
{
	_asm
	{
		push [name];
		mov ecx, [me];
		call [fn];
	}
}

GRFManager::GRFManager()
{
	Me = this;
	loaded = false;
}

GRFManager::~GRFManager()
{
}

void GRFManager::Boot()
{
	NCodeHookIA32 *hooks = GameArmor::getSingleton()->getHooks();
	CIniReader ini(GameArmor::getSingleton()->getIniPath());
	char *loadGrfPointer;
	char *tmp;
	int i = 0;

	loadGrfPointer = GaFindPattern("6A FF 68 ?d 64 A1 ?d 50 83 EC 08 53 56 57 A1 ?d 33 C4 50 8D 44 24 18 64 A3 ?d 8B D9");

	loadGrf = hooks->createHook<LoadGrfPrototype>((LoadGrfPrototype)loadGrfPointer, (LoadGrfPrototype)LoadGrfStub);
	
	grfList.push_back(string(ini.ReadString("Content", "OriginalGRF", "data.grf")));

	while (true)
	{
		char name[32];
		sprintf(name, "CustomGRF%d", i);

		tmp = ini.ReadString("Content", name, "<none>");

		if (strcmp(tmp, "<none>") == 0)
			break;

		string nameb64_1(tmp);
		string namelmfao = base64_decode(nameb64_1);
		LMFAOCrypt::Decrypt((unsigned char *)namelmfao.c_str(), namelmfao.size());

		grfList.push_back(namelmfao);

		i++;
	}
}

void GRFManager::Load(void *ptr)
{
	if (loaded)
		return;

	for (int i = 0; i < grfList.size(); i++)
	{
		CallAddGrf((char *)loadGrf, ptr, (char *)grfList[i].c_str());
	}

	loaded = true;
}
