#pragma once

#define MAX_SECTION_NUM         20

class CPELibrary
{
private:
	PCHAR					pMem;
	DWORD					dwFileSize;
	
public: 
	PIMAGE_DOS_HEADER		image_dos_header;
	PCHAR					pDosStub;
	DWORD					dwDosStubSize, dwDosStubOffset;
	PIMAGE_NT_HEADERS		image_nt_headers;
	PIMAGE_SECTION_HEADER	image_section_header[MAX_SECTION_NUM];
	PCHAR					image_section[MAX_SECTION_NUM];
	PIMAGE_TLS_DIRECTORY32	image_tls_directory;
	DWORD					LoadLibrary_addr;
	DWORD					GetProcAddress_addr;
	
	DWORD PEAlign(DWORD dwTarNum,DWORD dwAlignTo);
	void AlignmentSections();

	DWORD Offset2RVA(DWORD dwRO);
	DWORD RVA2Offset(DWORD dwRVA);

	PIMAGE_SECTION_HEADER ImageRVA2Section(DWORD dwRVA);
	PIMAGE_SECTION_HEADER ImageOffset2Section(DWORD dwRO);

	DWORD ImageOffset2SectionNum(DWORD dwRVA);
	PIMAGE_SECTION_HEADER AddNewSection(char* szName,DWORD dwSize,PCHAR *pointerToData);

	PIMAGE_SECTION_HEADER GetSectionByName(char* szName, PCHAR *ptr)
	{
		for (int i = 0; i < image_nt_headers->FileHeader.NumberOfSections; i++)
		{
			if (strncmp((char *)image_section_header[i]->Name, szName, 8) == 0)
			{
				if (ptr)
					*ptr = image_section[i];

				return image_section_header[i];
			}
		}
		
		return NULL;
	}

	void FindApiCalls();

	PCHAR GetRawData()
	{
		return pMem;
	}

	DWORD GetRawDataSize()
	{
		return dwFileSize;
	}

	DWORD GetNTAddr()
	{
		return (DWORD)(pMem + image_dos_header->e_lfanew);
	}

	void Obfuscate();

	CPELibrary();
	~CPELibrary();

	void OpenFile(char* FileName);
	void SaveFile(char* FileName);
};

#define MemErr					1 
#define PEErr					2 
#define FileErr					3 
#define NoRoom4SectionErr		4 
#define FsizeErr				5 
#define SecNumErr				6 
#define IIDErr					7 
#define FileISProtect			8 
#define PEnotValid				9 
#define PEisCOMRuntime			10 
#define DLLnotSupport			11 
#define WDMnotSupport			12 
#define TServernotSupport		13 
#define SYSnotSupport			14 
#define NOSEHnotSupport			15 
#define NOBINDnotSupport		16 
#define PackSectionName			17 
 
extern void ShowErr(unsigned char numErr);
