#include "PEInject.h"

#include <time.h>

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

#define PEIROTATE(x, n) ((x << n) | (x >> (32-n)))
#define PEI1(x,y,z) (x&y|~x&z)
#define PEI2(x,y,z) (x&y|x&~z)
#define PEI3(x,y,z) (x^y^z)
#define PEI4(x,y,z) (y^(x | ~z))
#define PEIROUND1(ret, n, i, s, ac) ret = PEIROTATE(i ^ PEI1(n,ac,ret) + ret ^ ac, s) + PEIROTATE((ret + ac), s) & ~3;
#define PEIROUND2(ret, n, i, s, ac) ret = PEIROTATE(i ^ PEI2(n,ac,ret) + ret ^ ac, s) + (~(PEIROTATE((ret + ac), s)));
#define PEIROUND3(ret, n, i, s, ac) ret = PEIROTATE(i ^ PEI3(n,ac,ret) + ret ^ ac, s) + PEIROTATE((ret + ac), s) & ~3;
#define PEIROUND4(ret, n, i, s, ac) ret = PEIROTATE(i ^ PEI3(n,ac,ret) + ret ^ ac, s) + (~(PEIROTATE((ret + ac), s)));

__forceinline unsigned int _PEI_CHECKSUM_(char *data, int size)
{
	unsigned int ret = 0x67452301 ^ 0xefcdab89 ^ 0x98badcfe ^ 0x10325476;

	for (int i = 0; i < size; i += 4)
	{
		unsigned int n = *((DWORD*)&data[i]);

		/* Round 1 */
		PEIROUND1(ret, n, i, S11, 0xd76aa478); /* 1 */
		PEIROUND1(ret, n, i, S12, 0xe8c7b756); /* 2 */
		PEIROUND1(ret, n, i, S13, 0x242070db); /* 3 */
		PEIROUND1(ret, n, i, S14, 0xc1bdceee); /* 4 */
		PEIROUND1(ret, n, i, S11, 0xf57c0faf); /* 5 */
		PEIROUND1(ret, n, i, S12, 0x4787c62a); /* 6 */
		PEIROUND1(ret, n, i, S13, 0xa8304613); /* 7 */
		PEIROUND1(ret, n, i, S14, 0xfd469501); /* 8 */
		PEIROUND1(ret, n, i, S11, 0x698098d8); /* 9 */
		PEIROUND1(ret, n, i, S12, 0x8b44f7af); /* 10 */
		PEIROUND1(ret, n, i, S13, 0xffff5bb1); /* 11 */
		PEIROUND1(ret, n, i, S14, 0x895cd7be); /* 12 */
		PEIROUND1(ret, n, i, S11, 0x6b901122); /* 13 */
		PEIROUND1(ret, n, i, S12, 0xfd987193); /* 14 */
		PEIROUND1(ret, n, i, S13, 0xa679438e); /* 15 */
		PEIROUND1(ret, n, i, S14, 0x49b40821); /* 16 */

		/* Round 2 */
		PEIROUND2(ret, n, i, S21, 0xf61e2562); /* 17 */
		PEIROUND2(ret, n, i, S22, 0xc040b340); /* 18 */
		PEIROUND2(ret, n, i, S23, 0x265e5a51); /* 19 */
		PEIROUND2(ret, n, i, S24, 0xe9b6c7aa); /* 20 */
		PEIROUND2(ret, n, i, S21, 0xd62f105d); /* 21 */
		PEIROUND2(ret, n, i, S22,  0x2441453); /* 22 */
		PEIROUND2(ret, n, i, S23, 0xd8a1e681); /* 23 */
		PEIROUND2(ret, n, i, S24, 0xe7d3fbc8); /* 24 */
		PEIROUND2(ret, n, i, S21, 0x21e1cde6); /* 25 */
		PEIROUND2(ret, n, i, S22, 0xc33707d6); /* 26 */
		PEIROUND2(ret, n, i, S23, 0xf4d50d87); /* 27 */
		PEIROUND2(ret, n, i, S24, 0x455a14ed); /* 28 */
		PEIROUND2(ret, n, i, S21, 0xa9e3e905); /* 29 */
		PEIROUND2(ret, n, i, S22, 0xfcefa3f8); /* 30 */
		PEIROUND2(ret, n, i, S23, 0x676f02d9); /* 31 */
		PEIROUND2(ret, n, i, S24, 0x8d2a4c8a); /* 32 */

		/* Round 3 */
		PEIROUND3(ret, n, i, S31, 0xfffa3942); /* 33 */
		PEIROUND3(ret, n, i, S32, 0x8771f681); /* 34 */
		PEIROUND3(ret, n, i, S33, 0x6d9d6122); /* 35 */
		PEIROUND3(ret, n, i, S34, 0xfde5380c); /* 36 */
		PEIROUND3(ret, n, i, S31, 0xa4beea44); /* 37 */
		PEIROUND3(ret, n, i, S32, 0x4bdecfa9); /* 38 */
		PEIROUND3(ret, n, i, S33, 0xf6bb4b60); /* 39 */
		PEIROUND3(ret, n, i, S34, 0xbebfbc70); /* 40 */
		PEIROUND3(ret, n, i, S31, 0x289b7ec6); /* 41 */
		PEIROUND3(ret, n, i, S32, 0xeaa127fa); /* 42 */
		PEIROUND3(ret, n, i, S33, 0xd4ef3085); /* 43 */
		PEIROUND3(ret, n, i, S34,  0x4881d05); /* 44 */
		PEIROUND3(ret, n, i, S31, 0xd9d4d039); /* 45 */
		PEIROUND3(ret, n, i, S32, 0xe6db99e5); /* 46 */
		PEIROUND3(ret, n, i, S33, 0x1fa27cf8); /* 47 */
		PEIROUND3(ret, n, i, S34, 0xc4ac5665); /* 48 */

		/* Round 4 */
		PEIROUND4(ret, n, i, S41, 0xf4292244); /* 49 */
		PEIROUND4(ret, n, i, S42, 0x432aff97); /* 50 */
		PEIROUND4(ret, n, i, S43, 0xab9423a7); /* 51 */
		PEIROUND4(ret, n, i, S44, 0xfc93a039); /* 52 */
		PEIROUND4(ret, n, i, S41, 0x655b59c3); /* 53 */
		PEIROUND4(ret, n, i, S42, 0x8f0ccc92); /* 54 */
		PEIROUND4(ret, n, i, S43, 0xffeff47d); /* 55 */
		PEIROUND4(ret, n, i, S44, 0x85845dd1); /* 56 */
		PEIROUND4(ret, n, i, S41, 0x6fa87e4f); /* 57 */
		PEIROUND4(ret, n, i, S42, 0xfe2ce6e0); /* 58 */
		PEIROUND4(ret, n, i, S43, 0xa3014314); /* 59 */
		PEIROUND4(ret, n, i, S44, 0x4e0811a1); /* 60 */
		PEIROUND4(ret, n, i, S41, 0xf7537e82); /* 61 */
		PEIROUND4(ret, n, i, S42, 0xbd3af235); /* 62 */
		PEIROUND4(ret, n, i, S43, 0x2ad7d2bb); /* 63 */
		PEIROUND4(ret, n, i, S44, 0xeb86d391); /* 64 */

		ret ^= n - i + ret & 3;
	}

	return ret ^ size;
}

#define XEPEIROUND(n, size, d, k) \
	n ^= k; \
	n ^= (k << d); \
	n += k ^ size;

__forceinline void _PEI_ENC_(unsigned int *keys, char *data, int size)
{
	for (int i = 0; i + 4 < size; i += 4)
	{
		unsigned int n = *((DWORD*)&data[i]);

		XEPEIROUND(n, size, i % 7, keys[i % 7]);

		*((DWORD*)&data[i]) = n;
	}
}

PEInject::PEInject()
{
}

PEInject::~PEInject()
{
}

int PEInject::InjectFile(char *input, char *output, char *dll, void *extraData, DWORD extraDataSize)
{
	inputPE.OpenFile(input);
	dllGALS.LoadFromDll(dll);

	inputPE.FindApiCalls();

	DoInjection(extraData, extraDataSize);

	//inputPE.Obfuscate();
	inputPE.SaveFile(output);

	return 0;
}

extern "C" int __stdcall NewEntryPoint_Start(DWORD dwDataSection);
extern "C" int __stdcall NewEntryPoint_End();

void PEInject::DoInjection(void *extraData, DWORD extraDataSize)
{
	DWORD totalSize = dllGALS.getBufferSize() + sizeof(PEInjectHeader);

	PCHAR dataSec = 0;
	PIMAGE_SECTION_HEADER dataSecHdr = inputPE.AddNewSection(".ga00", totalSize, &dataSec);
	dataSecHdr->Characteristics = 0xE0000040;
	
	PCHAR ptr = dataSec;
	PEInjectHeader *peih = (PEInjectHeader *)ptr;

	peih->Magic = 0x48AFB89D;
	peih->ImageBase = inputPE.image_nt_headers->OptionalHeader.ImageBase;
	peih->OEP = inputPE.image_nt_headers->OptionalHeader.AddressOfEntryPoint;
	peih->LoadLibraryPtr = (LOADLIBRARY)inputPE.LoadLibrary_addr;
	peih->GetProcAddressPtr = (GETPROCADDRESS)inputPE.GetProcAddress_addr;
	peih->GALSChecksum = _PEI_CHECKSUM_(dllGALS.getBuffer(), dllGALS.getBufferSize());

	srand(time(NULL));
	peih->GALSKey[0] = rand();
	peih->GALSKey[1] = rand();
	peih->GALSKey[2] = rand();
	peih->GALSKey[3] = rand();
	peih->GALSKey[4] = rand();
	peih->GALSKey[5] = rand();
	peih->GALSKey[6] = rand();
	peih->GALSKey[7] = rand();

	strcpy(peih->Strings[PEI_STR_KERNEL32], "kernel32.dll");
	strcpy(peih->Strings[PEI_STR_VIRTUAL_ALLOC], "VirtualAlloc");
	strcpy(peih->Strings[PEI_STR_VIRTUAL_FREE], "VirtualFree");
	strcpy(peih->Strings[PEI_STR_VIRTUAL_PROTECT], "VirtualProtect");
	
	char *galsCode = dllGALS.getBuffer() + sizeof(PEInjectHeader);
	_PEI_ENC_(peih->GALSKey, galsCode, dllGALS.getBufferSize() - sizeof(PEInjectHeader));
	memcpy(ptr + sizeof(PEInjectHeader), dllGALS.getBuffer(), dllGALS.getBufferSize());

	PCHAR codeSec = 0;
	PIMAGE_SECTION_HEADER codeSecHdr = inputPE.AddNewSection(".ga01", (DWORD)NewEntryPoint_End - (DWORD)NewEntryPoint_Start + 7, &codeSec);
	codeSecHdr->Characteristics = 0xE0000040;
	
	*((CHAR*)(codeSec+0)) = 0x90;
	*((CHAR*)(codeSec+1)) = 0x68;
	*((DWORD*)(codeSec+2)) = dataSecHdr->VirtualAddress + inputPE.image_nt_headers->OptionalHeader.ImageBase;
	*((CHAR*)(codeSec+6)) = 0x52;

	memcpy(codeSec + 7, (void *)NewEntryPoint_Start, (DWORD)NewEntryPoint_End - (DWORD)NewEntryPoint_Start);
	
	inputPE.image_nt_headers->OptionalHeader.AddressOfEntryPoint = 0;
	char *ep = (char *)(&inputPE.image_dos_header->e_magic) + 2;
	*ep = 0xe9;
	*((DWORD*)(ep+1)) = codeSecHdr->VirtualAddress - 7;
}
