#pragma once

class LMFAOCrypt
{
public:
	static void Encrypt(unsigned char *buff, int size);
	static void Decrypt(unsigned char *buff, int size);
};
