#include "stdafx.h"

#include "Base64.h"
#include "LMFAOCrypt.h"
#include "../Common/md5.h"

#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
	string grfFile;

	if (argc < 2)
		return 0;

	char *client = argv[1];
	char *license = argv[2];
	char *ini = strdup(client);
	char *grf = strdup(client);
	int size = strlen(ini);

	ini[size - 1] = 'i';
	ini[size - 2] = 'n';
	ini[size - 3] = 'i';
	
	grf[size - 1] = 'f';
	grf[size - 2] = 'r';
	grf[size - 3] = 'g';
	
	LMFAOCrypt::Encrypt((unsigned char *)grf, size);

	string clientMD5 = md5file(client);
	 
	char temp[32];
	memcpy(temp, clientMD5.c_str(), 32);
	LMFAOCrypt::Encrypt((unsigned char *)temp, 32);
	LMFAOCrypt::Encrypt((unsigned char *)temp, 32);
	LMFAOCrypt::Encrypt((unsigned char *)&temp[16], 16);
	
	MD5 checkMd5;
	checkMd5.update(temp, 32);
	checkMd5.finalize();

	string finalMD5 = checkMd5.hexdigest();

	ofstream ofs;
	ofs.open(ini);
	ofs << "[GameArmor]" << endl;
	ofs << "HASH=" << string(license) << endl;
	ofs << "HASH2=" << finalMD5 << endl;
	ofs << endl;
	ofs << "[Content]" << endl;
	ofs << "OficialGRF=data.grf" << endl;
	ofs << "CustomGRF0=" << base64_encode((const unsigned char *)grf, size) << endl;
	ofs << endl;
	ofs.close();

	return 0;
}

