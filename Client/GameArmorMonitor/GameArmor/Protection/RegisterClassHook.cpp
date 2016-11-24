#include "..\GameArmor.h"

#include "RegisterClassHook.h"
#include "..\..\Hook\NCodeHookInstantiation.h"

#include <Windows.h>
#include <malloc.h>

static RegisterClassHook *Me;

void *__stdcall RegisterClassAHook(const WNDCLASSA *lpWndClass)
{
	if (!Me->CheckName((char *)lpWndClass->lpszClassName))
		return (void *)0;

	return Me->getRcA()(lpWndClass);
}

RegisterClassHook::RegisterClassHook()
{
	Me = this;

	this->RegisterForbiddenClass("WinsockSpy.SendClass", 1);
}

RegisterClassHook::~RegisterClassHook()
{
}

void RegisterClassHook::Boot()
{
	NCodeHookIA32 *hooks = GameArmor::getSingleton()->getHooks();

	rca = hooks->createHook<RegisterClassAPrototype>((RegisterClassAPrototype)RegisterClassA, (RegisterClassAPrototype)RegisterClassAHook);
}

bool RegisterClassHook::CheckName(char *name)
{
#ifdef _DEBUG
	return true;
#else
	string sname(name);

	if (forbiddenClasses.count(sname))
	{
		int gravity = forbiddenClasses[sname];

		switch (gravity)
		{
		case 1:
			ShowErrorMsg("HTD");
			GameArmor::getSingleton()->NotifyHackTool();
			return false;
		case 0:
		default:
			return false;
		}
	}

	return true;
#endif
}
