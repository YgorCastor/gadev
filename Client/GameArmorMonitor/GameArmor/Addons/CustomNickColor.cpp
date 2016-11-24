#include "..\GameArmor.h"

#include "CustomNickColor.h"
#include "..\..\Hook\NCodeHookInstantiation.h"
#include "..\..\Utils\MemoryTools.h"

#include <Windows.h>

static CustomNickColor *Me;

typedef COLORREF (*IsNameSpecificPrototype)(unsigned int aid);
IsNameSpecificPrototype IsNameSpecific;

COLORREF IsNameSpecificHook(unsigned int aid)
{
	return Me->getColor(aid);
}

CustomNickColor::CustomNickColor()
{
	Me = this;
}

CustomNickColor::~CustomNickColor()
{
}

void CustomNickColor::Boot()
{
	NCodeHookIA32 *hooks = GameArmor::getSingleton()->getHooks();
	char *ins = GaFindPattern("55 8B EC A1 ?d 85 C0");
	
	if (!ins)
		ins = GaFindPattern("83 3D ?d 06 75 3E");

	if (ins)
	{
		IsNameSpecific = hooks->createHook<IsNameSpecificPrototype>((IsNameSpecificPrototype)ins, IsNameSpecificHook);
	}
	else
	{
		ShowErrorMsg("CNS");
		ExitProcess(0);
	}

	GameArmor::getSingleton()->getPacketLen()->SetPacketLen(0xA00, 9);
}

void CustomNickColor::AddNickColor(int aid, char r, char g, char b)
{
	colors[aid] = RGB(r,g,b);
}

int CustomNickColor::getColor(int aid)
{
	if (colors.count(aid))
		return colors[aid];

	return 0;
}
