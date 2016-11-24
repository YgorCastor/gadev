#pragma once
#include <fstream>

static bool fexists(const char *filename)
{
	std::ifstream ifile(filename);
	return ifile;
}

static std::wstring StringToWString(const std::string& s)
{
	std::wstring temp(s.length(),L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp; 
}


static std::string WStringToString(const std::wstring& s)
{
	std::string temp(s.length(), ' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp; 
}

static std::string ToLower(std::string in)
{
	int len = in.length();

	for (int i = 0; i < len; i++)
		in[i] = tolower((int)in[i]);

	return in;
}
