#pragma once

typedef void *HINTERNET;
#include <fstream>

const int MAX_ERRMSG_SIZE = 80;
const int MAX_FILENAME_SIZE = 512;
const int BUF_SIZE = 4096;             // 10 KB

// Exception class for donwload errors;
class DLExc
{
private:
	char err[MAX_ERRMSG_SIZE];
public:
	DLExc(char *exc)
	{
		if(strlen(exc) < MAX_ERRMSG_SIZE)
			strcpy(err, exc);
	}

	// Return a pointer to the error message
	const char *geterr()
	{
		return err;
	}
};

// A class for downloading files from the internet
class Download
{
private:
	static bool ishttp(char *url);
	static bool httpverOK(HINTERNET hIurl);
	static unsigned long openfile(char *sto, char *url, bool reload, std::ofstream &fout);
public:
	static bool getfname(char *url, char *fname);
	static bool getfnamewin(char *url, char *fname);
	static bool download(char *url, char *sto, bool reload=false, void (*update)(unsigned long, unsigned long)=NULL);
};
