#pragma once

#include <string>
#include <map>

using namespace std;

class PatternDB
{
public:
	PatternDB();
	~PatternDB();

	void AddPattern(string pat, int gravity)
	{
		patterns[pat] = gravity;
	}

	int ScanForPattern(char *start, char *end);

private:
	map<string, int> patterns;
};

