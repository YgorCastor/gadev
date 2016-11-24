#include "stdafx.h"
#include "download.h"


//#include <iostream>
#include <string>
#include <windows.h>
#include <wininet.h>
#include <fstream>

using namespace std;

bool Download::download(char *url, char *sto, bool reload, void (*update)(unsigned long, unsigned long))
{
    ofstream fout;              
    unsigned char buf[BUF_SIZE];
    unsigned long numrcved = 0;     
    unsigned long filelen = 0;      
    HINTERNET hIurl = 0, hInet = 0;     
    unsigned long contentlen = 0;   
    unsigned long len = 0;          
    unsigned long total = 0;    
    char header[80];            

    try
    {
        if(!ishttp(url))
            throw DLExc("Must be HTTP url");

        filelen = openfile(sto, url, reload, fout);

        if(InternetAttemptConnect(0) != ERROR_SUCCESS)
            throw DLExc("Can't connect");

        hInet = InternetOpenA("downloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if(hInet == NULL)
            throw DLExc("Can't open connection");

        sprintf(header, "Range:bytes=%d-", filelen);

        hIurl = InternetOpenUrlA(hInet, url, header, strlen(header), INTERNET_FLAG_NO_CACHE_WRITE, 0);
        if(hIurl == NULL)
            throw DLExc("Can't open url");

        if(!httpverOK(hIurl))
            throw DLExc("HTTP/1.1 not supported");

        len = sizeof contentlen;
        if(!HttpQueryInfoA(hIurl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentlen, &len, NULL))
            throw DLExc("File or content length not found");

		int code;
		len = sizeof code;
		if(!HttpQueryInfoA(hIurl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &code, &len, NULL))
			throw DLExc("Status code not found");

		if (!(code >= 20 && code < 300) && code != 100 && code != 302)
		{
			switch (code)
			{
			case 404:
				throw DLExc("HTTP error: file not found");
			default:
				char tmp[128];
				sprintf(tmp, "HTTP error: code %d", code);
				throw DLExc(tmp);
			}
		}

        if(filelen != contentlen && contentlen)
        {
            do
            {
                if(!InternetReadFile(hIurl, &buf, BUF_SIZE, &numrcved))
                    throw DLExc("Error occurred during download");

                fout.write((const char *) buf, numrcved);
                if(!fout.good())
                    throw DLExc("Error writing file");

                total += numrcved;

                if(update && numrcved > 0)
                    update(contentlen + filelen, total + filelen);
            } while (numrcved > 0);
        }
        else
        {
            if(update)
                update(filelen, filelen);
        }
    }
    catch (DLExc)
    {
        fout.close();

        InternetCloseHandle(hIurl);
        InternetCloseHandle(hInet);

        throw;
    }

    fout.close();
    InternetCloseHandle(hIurl);
    InternetCloseHandle(hInet);

    return true;
}

bool Download::httpverOK(HINTERNET hIurl)
{
    char str[80];
    unsigned long len = 79;

    if(!HttpQueryInfoA(hIurl, HTTP_QUERY_VERSION, &str, &len, NULL))
        return false;

    char *p = strchr(str, '/');
    p++;
    if(*p == '0')
        return false;

    p = strchr(str, '.');
    p++;

    int minorVerNum = atoi(p);

    if(minorVerNum > 0)
        return true;

    return false;
}

bool Download::getfname(char *url, char *fname)
{
    char *p = strrchr(url, '/');

    if(p && (strlen(p) < MAX_FILENAME_SIZE))
    {
        p++;
        strcpy(fname, p);
        return true;
    }
    else
    {
        return false;
    }
}

bool Download::getfnamewin(char *url, char *fname)
{
	char *p = strrchr(url, '\\');

	if(p && (strlen(p) < MAX_FILENAME_SIZE))
	{
		p++;
		strcpy(fname, p);
		return true;
	}
	else
	{
		return false;
	}
}

unsigned long Download::openfile(char *sto, char *url, bool reload, ofstream &fout)
{
    char fname[MAX_FILENAME_SIZE];

    if(!getfname(url, fname))
        throw DLExc("File name error");

    if(!reload)
        fout.open((sto ? sto : fname), ios::binary | ios::out | ios::app | ios::ate);
    else
        fout.open((sto ? sto : fname), ios::binary | ios::out | ios::trunc);

    if(!fout)
        throw DLExc("Can't open output file");

    return fout.tellp();
}

bool Download::ishttp(char *url)
{
    char str[5] = "";

    strncpy(str, url, 4);

    for(char *p = str; *p; p++)
        *p = tolower(*p);

    return !strcmp("http", str);
}
