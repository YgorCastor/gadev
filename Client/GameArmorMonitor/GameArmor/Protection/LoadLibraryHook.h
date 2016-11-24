#pragma once

typedef void * (__stdcall * LoadLibraryAPrototype)(char *lpFileName);
typedef void * (__stdcall * LoadLibraryWPrototype)(wchar_t *lpFileName);

typedef void * (__stdcall * LoadLibraryExAPrototype)(char *lpFileName, void *hFile, unsigned int dwFlags);
typedef void * (__stdcall * LoadLibraryExWPrototype)(wchar_t *lpFileName, void *hFile, unsigned int dwFlags);

class LoadLibraryHook
{
public:
	LoadLibraryHook();
	~LoadLibraryHook();

	void Boot();
	bool CheckMemory(void *hFile);

	LoadLibraryAPrototype getLla()
	{
		return lla;
	}

	LoadLibraryWPrototype getLlw()
	{
		return llw;
	}

	LoadLibraryExAPrototype getLlexa()
	{
		return llexa;
	}

	LoadLibraryExWPrototype getLlexw()
	{
		return llexw;
	}

private:
	LoadLibraryAPrototype lla;
	LoadLibraryWPrototype llw;

	LoadLibraryExAPrototype llexa;
	LoadLibraryExWPrototype llexw;
};
