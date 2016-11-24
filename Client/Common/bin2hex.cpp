#include "stdafx.h"

static char HEXSTR[] = "0123456789ABCDEF";

unsigned int unhexnib(unsigned char hexchar) 
{
	if (hexchar>='0' && hexchar <='9') 
	{
		return hexchar-'0';
	}

	if (hexchar>='A' && hexchar <='F') 
	{
		return hexchar-'A'+10;
	}

	if (hexchar>='a' && hexchar <='f') 
	{
		return hexchar-'a'+10;
	}

	return -1;
}

unsigned char *hexify_block(unsigned char *resultbuf, int length) 
{
	unsigned char *resultstr;
	int i;
	int leftnib;
	int rightnib;
	unsigned int ch;

	resultstr = (unsigned char *)malloc(length * 2 + 1);

	for (i = 0; i < length; i++) 
	{
		ch = (unsigned int)*resultbuf++;
		leftnib = (ch >> 4) & 0x0f ;
		rightnib = ch & 0x0f;
		resultstr[i * 2] = HEXSTR[leftnib];
		resultstr[i * 2 + 1] = HEXSTR[rightnib];
	}

	resultstr[length * 2] = 0;

	return resultstr;
}  
/*
unsigned char *unhexify_block(unsigned char *str, int length) 
{
	unsigned char *source;
	unsigned char *result;

	int hexlen = length * 2;
	int len = strlen((const char *)str);

	int i;

	unsigned int ch;
	int leftnib;
	int rightnib;

	result = (char*)malloc(length);

	if (len < hexlen) 
	{
		source = d_lpad(str, '0', hexlen);
	}
	else 
	{
		source = d_dup(str);
		source[hexlen] = 0;
	}

	for (i=0;i<length;i++) 
	{
		leftnib = unhexnib(source[i * 2]);
		rightnib = unhexnib(source[i * 2 + 1]);
		ch = (leftnib << 4) | rightnib;
		result[i] = (unsigned char)ch;
	}

	free(source);

	return result;
}*/
