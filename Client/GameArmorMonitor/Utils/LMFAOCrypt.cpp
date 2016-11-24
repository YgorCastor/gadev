#include "LMFAOCrypt.h"

void LMFAOCrypt::Encrypt(unsigned char *buff, int size)
{
	for (int i = 0; i < size; i++)
	{
		buff[i] ^= size;
		buff[i] = ((buff[i] >> 4) & 0x0f) | ((buff[i] << 4) & 0xf0);
		buff[i] ^= i;
		buff[i] = ((buff[i] >> 4) & 0x0f) | ((buff[i] << 4) & 0xf0);
	}
}

void LMFAOCrypt::Decrypt(unsigned char *buff, int size)
{
	for (int i = 0; i < size; i++)
	{
		buff[i] = ((buff[i] >> 4) & 0x0f) | ((buff[i] << 4) & 0xf0);
		buff[i] ^= i;
		buff[i] = ((buff[i] >> 4) & 0x0f) | ((buff[i] << 4) & 0xf0);
		buff[i] ^= size;
	}
}
