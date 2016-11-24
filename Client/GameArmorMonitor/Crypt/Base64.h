#pragma once

#include <string>

using namespace std;

string base64_encode(unsigned char const* , unsigned int len);
string base64_decode(string const& s);
