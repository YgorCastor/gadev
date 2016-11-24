#pragma once

#include <map>

using namespace std;

class CustomNickColor
{
public:
	CustomNickColor();
	~CustomNickColor();

	void Boot();
	void AddNickColor(int aid, char r, char g, char b);
	int getColor(int aid);

private:
	map<int, int> colors;
};

