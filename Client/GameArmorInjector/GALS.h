#pragma once

#define GALSSignature 0x89D43AF8

typedef struct
{
	unsigned int Signature;
	unsigned int GalsSize;
	unsigned int ImageBase;
	unsigned int EntryPoint;
	unsigned int SizeOfImage;
	unsigned int NumberOfSections;
	unsigned int ImportsVA;
	unsigned int ExportsVA;
	unsigned int RelocationsVA;
	unsigned int ResourcesVA;
} GALSHeader, *PGALSHeader;

typedef struct
{
	unsigned int RawAddress;
	unsigned int RawSize;
	unsigned int VirtualAddress;
	unsigned int VirtualSize;
	unsigned int Flags;
} GALSSection, *PGALSSection;

class GALS
{
public:
	GALS();
	~GALS();
	
	void LoadFromDll(char *filename);

	char *getBuffer()
	{
		return buffer;
	}

	unsigned int getBufferSize()
	{
		return bufferSize;
	}

private:
	char *buffer;
	unsigned int bufferSize;
};
