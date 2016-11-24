#pragma once

#include <string>
#include <vector>

using namespace std;

typedef void (_fastcall * LoadGrfPrototype)(void *ptr, char *name);

class GRFManager
{
public:
	GRFManager();
	~GRFManager();

	void Boot();
	void Load(void *ptr);

	LoadGrfPrototype getLoadGrf()
	{
		return loadGrf;
	}

private:
	vector<string> grfList;
	LoadGrfPrototype loadGrf;
	bool loaded;
};
