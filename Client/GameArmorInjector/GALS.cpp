#include "GALS.h"

#include <Windows.h>
#include "PEAnalyser.h"

#include <malloc.h>

GALS::GALS()
{
}

GALS::~GALS()
{
	if (buffer)
		free(buffer);
}

void GALS::LoadFromDll(char *dllFile)
{
	CPELibrary dll;

	dll.OpenFile(dllFile);

	bufferSize = sizeof(GALSHeader);
	for (int i = 0; i < dll.image_nt_headers->FileHeader.NumberOfSections; i++)
		bufferSize += dll.image_section_header[i]->SizeOfRawData + sizeof(GALSSection);

	buffer = (char *)malloc(bufferSize);

	PGALSHeader header = (PGALSHeader)buffer;
	header->Signature = GALSSignature;
	header->GalsSize = bufferSize;
	header->ImageBase = dll.image_nt_headers->OptionalHeader.ImageBase;
	header->EntryPoint = dll.image_nt_headers->OptionalHeader.AddressOfEntryPoint;
	header->SizeOfImage = dll.image_nt_headers->OptionalHeader.SizeOfImage;
	header->NumberOfSections = dll.image_nt_headers->FileHeader.NumberOfSections;
	header->ImportsVA = dll.image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	header->ExportsVA = dll.image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	header->RelocationsVA = dll.image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	header->ResourcesVA = dll.image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	
	PGALSSection curHeader = (PGALSSection)&buffer[sizeof(GALSHeader)];
	char *initOffset = &buffer[sizeof(GALSHeader) + sizeof(GALSSection) * header->NumberOfSections];
	char *curSectionOffset = initOffset;
	DWORD delta = sizeof(GALSHeader) + sizeof(GALSSection) * header->NumberOfSections;
	for (int i = 0; i < header->NumberOfSections; i++, curHeader++)
	{
		curHeader->RawAddress = delta + (unsigned int)curSectionOffset - (unsigned int)initOffset;
		curHeader->RawSize = dll.image_section_header[i]->SizeOfRawData;
		curHeader->VirtualAddress = dll.image_section_header[i]->VirtualAddress;
		curHeader->VirtualSize = dll.image_section_header[i]->Misc.VirtualSize;

		memcpy(curSectionOffset, dll.image_section[i], dll.image_section_header[i]->SizeOfRawData);

		curSectionOffset += curHeader->RawSize;
	}
}
