#pragma once

#include <string>
#include <map>

using namespace std;

typedef void * (__stdcall * RegisterClassAPrototype)(const void *lpWndClass);

class RegisterClassHook
{
public:
	RegisterClassHook();
	~RegisterClassHook();

	void Boot();
	bool CheckName(char *name);

	RegisterClassAPrototype getRcA()
	{
		return rca;
	}

	void RegisterForbiddenClass(string name, int gravity)
	{
		forbiddenClasses[name] = gravity;
	}

private:
	RegisterClassAPrototype rca;

	map<string, int> forbiddenClasses;
};
