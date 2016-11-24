#include "PatternDB.h"

#include "..\..\Utils\MemoryTools.h"

PatternDB::PatternDB()
{
	AddPattern("56 E8 ?d 83 C4 04 8D 4C 24 04 68 ?d 51 E8 ?d 8B F0 56", 1); // WPE PRO
	AddPattern("68 ?d e8 ?d 8B F0 68 ?d E8 ?d 8B D8 68 ?d 56 e8 ?d 8B F8 A1 ?d", 1); // rEdox Packet Editor
}

PatternDB::~PatternDB()
{
}

int PatternDB::ScanForPattern(char *start, char *end)
{
#ifdef _DEBUG
	return -1;
#else
	map<string, int>::iterator it;
	for (it = patterns.begin(); it != patterns.end(); it++)
	{
		if (GaFindPatternEx(start, end, (char *)it->first.c_str()))
			return it->second;
	}

	return -1;
#endif
}
