#include <Windows.h>   
#include <imagehlp.h>//#include <Dbghelp.h>   
#include "PEAnalyser.h"

#include <math.h>
   
#ifdef _DEBUG   
#define DEBUG_NEW   
#endif   

const char *szErrorMsg[]=   
{   "",                                                         // Non-Error   
    "File access error :(",                                     // MemErr   
    "Invalid PE file!",                                         // PEErr   
    "Not enough memory :(",                                     // FileErr   
    "Files with a filesize of 0 aren't allowed!",               // NoRoom4SectionErr   
    "There's no room for a new section :(",                     // FsizeErr   
    "Too many sections!",                                       // SecNumErr   
    "Too much ImageImportDescriptors!",                         // IIDErr      
    "File already was protected!",                              // FileISProtect   
    "Invalid PE file! It might be protected by another tool.",  // PEnotValid      
    "This Version does not support COM Runtime structure.",     // PEisCOMRuntime   
    "This Version does not support dynamic link library.",      // DLLnotSupport   
    "This Version does not support windows driver model.",      // WDMnotSupport   
    "This Version does not support terminal server aware.",     // TServernotSupport   
    "This Version does not support system file.",               // SYSnotSupport   
    "No SE handler resides in this PE.",                        // NOSEHnotSupport     
    "Can not support PE file with no bind.",                    // NOBINDnotSupport   
    "Section's Name is not recognized :("                       // PackSectionName     
};

void ShowErr(unsigned char numErr)   
{   
    char *szErr=new TCHAR[128];   
    strcpy(szErr,szErrorMsg[numErr]);   
    MessageBox(GetActiveWindow(),szErr,   
               "Error",    
               MB_OK | MB_ICONERROR );   
    delete [] szErr;   
}

CPELibrary::CPELibrary()   
{   
    image_dos_header=new (IMAGE_DOS_HEADER);   
    dwDosStubSize=0;   
    image_nt_headers=new (IMAGE_NT_HEADERS);   
    for(int i=0;i<MAX_SECTION_NUM;i++) image_section_header[i]=new (IMAGE_SECTION_HEADER);   
    image_tls_directory=NULL;   
}

CPELibrary::~CPELibrary()   
{   
    delete []image_dos_header;   
    dwDosStubSize=0;   
    delete []image_nt_headers;   
    for(int i=0;i<MAX_SECTION_NUM;i++) delete []image_section_header[i];   
    if(image_tls_directory!=NULL) delete image_tls_directory;   
	
	if (pMem)
		GlobalFree(pMem);
}

DWORD CPELibrary::PEAlign(DWORD dwTarNum,DWORD dwAlignTo)   
{      
    return(((dwTarNum+dwAlignTo-1)/dwAlignTo)*dwAlignTo);   
}

void CPELibrary::AlignmentSections()   
{   
	int i;
    for(i=0;i<image_nt_headers->FileHeader.NumberOfSections;i++)   
    {   
        image_section_header[i]->VirtualAddress=   
            PEAlign(image_section_header[i]->VirtualAddress,   
            image_nt_headers->OptionalHeader.SectionAlignment);   
   
        image_section_header[i]->Misc.VirtualSize=   
            PEAlign(image_section_header[i]->Misc.VirtualSize,   
            image_nt_headers->OptionalHeader.SectionAlignment);   
   
        image_section_header[i]->PointerToRawData=   
            PEAlign(image_section_header[i]->PointerToRawData,   
            image_nt_headers->OptionalHeader.FileAlignment);   
   
        image_section_header[i]->SizeOfRawData=   
            PEAlign(image_section_header[i]->SizeOfRawData,   
            image_nt_headers->OptionalHeader.FileAlignment);   
    }   
    image_nt_headers->OptionalHeader.SizeOfImage=image_section_header[i-1]->VirtualAddress+   
        image_section_header[i-1]->Misc.VirtualSize;   
    image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress=0;   
    image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size=0;   
    image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress=0;   
    image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size=0;   
    image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress=0;   
    image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size=0;   
}

DWORD CPELibrary::RVA2Offset(DWORD dwRVA)   
{   
    DWORD _offset;   
    PIMAGE_SECTION_HEADER section;   
    section=ImageRVA2Section(dwRVA);//ImageRvaToSection(pimage_nt_headers,Base,dwRVA);   
    if(section==NULL)   
    {   
        return(0);   
    }   
    _offset=dwRVA+section->PointerToRawData-section->VirtualAddress;   
    return(_offset);   
}

DWORD CPELibrary::Offset2RVA(DWORD dwRO)   
{   
    PIMAGE_SECTION_HEADER section;   
    section=ImageOffset2Section(dwRO);   
    if(section==NULL)   
    {   
        return(0);   
    }   
    return(dwRO+section->VirtualAddress-section->PointerToRawData);   
}

PIMAGE_SECTION_HEADER CPELibrary::ImageRVA2Section(DWORD dwRVA)   
{   
    int i;   
    for(i=0;i<image_nt_headers->FileHeader.NumberOfSections;i++)   
    {   
		if((dwRVA>=image_section_header[i]->VirtualAddress) && (dwRVA<(image_section_header[i]->VirtualAddress+image_section_header[i]->Misc.VirtualSize)))   
        {   
            return ((PIMAGE_SECTION_HEADER)image_section_header[i]);   
        }   
    }   
    return(NULL);   
}

PIMAGE_SECTION_HEADER CPELibrary::ImageOffset2Section(DWORD dwRO)   
{   
    for(int i=0;i<image_nt_headers->FileHeader.NumberOfSections;i++)   
    {   
        if((dwRO>=image_section_header[i]->PointerToRawData) && (dwRO<(image_section_header[i]->PointerToRawData+image_section_header[i]->SizeOfRawData)))   
        {   
            return ((PIMAGE_SECTION_HEADER)image_section_header[i]);   
        }   
    }   
    return(NULL);   
}   
 
DWORD CPELibrary::ImageOffset2SectionNum(DWORD dwRO)   
{   
    for(int i=0;i<image_nt_headers->FileHeader.NumberOfSections;i++)   
    {   
        if((dwRO>=image_section_header[i]->PointerToRawData) && (dwRO<(image_section_header[i]->PointerToRawData+image_section_header[i]->SizeOfRawData)))   
        {   
            return (i);   
        }   
    }   
    return(-1);   
}   

PIMAGE_SECTION_HEADER CPELibrary::AddNewSection(char* szName,DWORD dwSize,PCHAR *pointerToData)   
{   
    DWORD roffset,rsize,voffset,vsize;   
    int i=image_nt_headers->FileHeader.NumberOfSections;   
    rsize=PEAlign(dwSize,   
                image_nt_headers->OptionalHeader.FileAlignment);   
    vsize=PEAlign(rsize,   
                image_nt_headers->OptionalHeader.SectionAlignment);   
    roffset=PEAlign(image_section_header[i-1]->PointerToRawData+image_section_header[i-1]->SizeOfRawData,   
                image_nt_headers->OptionalHeader.FileAlignment);   
    voffset=PEAlign(image_section_header[i-1]->VirtualAddress+image_section_header[i-1]->Misc.VirtualSize,   
                image_nt_headers->OptionalHeader.SectionAlignment);   
    memset(image_section_header[i],0,(size_t)sizeof(IMAGE_SECTION_HEADER));   
    image_section_header[i]->PointerToRawData=roffset;   
    image_section_header[i]->VirtualAddress=voffset;   
    image_section_header[i]->SizeOfRawData=rsize;   
    image_section_header[i]->Misc.VirtualSize=vsize;   
    image_section_header[i]->Characteristics=0xC0000040;   
    memcpy(image_section_header[i]->Name,szName,(size_t)strlen(szName));   
    image_section[i]=(char*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,rsize);   
    image_nt_headers->FileHeader.NumberOfSections++;   
	if (pointerToData)
		*pointerToData = image_section[i];
    return (PIMAGE_SECTION_HEADER)image_section_header[i];   
}   

void CPELibrary::OpenFile(char* FileName)   
{   
    DWORD   dwBytesRead     = 0;   
    HANDLE  hFile= NULL;   
    DWORD SectionNum;   
    DWORD i;   
    DWORD dwRO_first_section;   
    pMem=NULL;   
  
    hFile=CreateFile(FileName,   
                     GENERIC_READ,   
                     FILE_SHARE_WRITE | FILE_SHARE_READ,   
                     NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);   
    if(hFile==INVALID_HANDLE_VALUE)   
    {   
        ShowErr(FileErr);   
        return;   
    }   
    dwFileSize=GetFileSize(hFile,0);   
    if(dwFileSize == 0)   
    {   
        CloseHandle(hFile);   
        ShowErr(FsizeErr);   
        return;   
    }   
	if (pMem)
		GlobalFree(pMem);
    pMem=(char*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,dwFileSize);   
    if(pMem == NULL)   
    {   
        CloseHandle(hFile);   
        ShowErr(MemErr);   
        return;   
    }   
    ReadFile(hFile,pMem,dwFileSize,&dwBytesRead,NULL);   
    CloseHandle(hFile);   

    memcpy(image_dos_header,pMem,sizeof(IMAGE_DOS_HEADER));   
    dwDosStubSize=image_dos_header->e_lfanew-sizeof(IMAGE_DOS_HEADER);   
    dwDosStubOffset=sizeof(IMAGE_DOS_HEADER);   
    pDosStub=new CHAR[dwDosStubSize];   
    if((dwDosStubSize&0x80000000)==0x00000000)   
    {   
        CopyMemory(pDosStub,pMem+dwDosStubOffset,dwDosStubSize);   
    }   
    memcpy(image_nt_headers,   
               pMem+image_dos_header->e_lfanew,   
               sizeof(IMAGE_NT_HEADERS));   
    dwRO_first_section=image_dos_header->e_lfanew+sizeof(IMAGE_NT_HEADERS);   
    if(image_dos_header->e_magic!=IMAGE_DOS_SIGNATURE)// MZ   
    {   
        ShowErr(PEErr);   
        GlobalFree(pMem);   
        return;   
    }   
    if(image_nt_headers->Signature!=IMAGE_NT_SIGNATURE)// PE00   
    {   
        ShowErr(PEErr);   
        GlobalFree(pMem);   
        return;   
    }   

    SectionNum=image_nt_headers->FileHeader.NumberOfSections;   

    for( i=0;i<SectionNum;i++)    
    {   
        CopyMemory(image_section_header[i],pMem+dwRO_first_section+i*sizeof(IMAGE_SECTION_HEADER),   
            sizeof(IMAGE_SECTION_HEADER));   
    }   

    for(i=0;i<SectionNum;i++)   
    {   
            image_section[i]=(char*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,   
                PEAlign(image_section_header[i]->SizeOfRawData,   
                image_nt_headers->OptionalHeader.FileAlignment));   
   
            CopyMemory(image_section[i],   
                    pMem+image_section_header[i]->PointerToRawData,   
                    image_section_header[i]->SizeOfRawData);   
    }   

    if(image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress!=0)   
    {   
        image_tls_directory = new (IMAGE_TLS_DIRECTORY32);   
        DWORD dwOffset=RVA2Offset(image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);   
        memcpy(image_tls_directory,   
                pMem+dwOffset,   
                sizeof(IMAGE_TLS_DIRECTORY32));        
    }  
}   

void CPELibrary::SaveFile(char* FileName)   
{   
    DWORD   dwBytesWritten  = 0;   
    DWORD i;   
    DWORD dwRO_first_section;   
    DWORD SectionNum;   
    HANDLE  hFile= NULL;   
    pMem=NULL;   

    hFile=CreateFile(FileName,   
                     GENERIC_WRITE,   
                     FILE_SHARE_WRITE | FILE_SHARE_READ,   
                     NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);   
    if(hFile==INVALID_HANDLE_VALUE)   
    {   
        hFile=CreateFile(FileName,   
                     GENERIC_WRITE,   
                     FILE_SHARE_WRITE | FILE_SHARE_READ,   
                     NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);   
        if(hFile==INVALID_HANDLE_VALUE)   
        {   
            ShowErr(FileErr);   
            return;   
        }   
    }   

    AlignmentSections();   

    i=image_nt_headers->FileHeader.NumberOfSections;   
    dwFileSize=image_section_header[i-1]->PointerToRawData+   
        image_section_header[i-1]->SizeOfRawData;   
   
	if (pMem)
		GlobalFree(pMem);

    pMem=(char*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,dwFileSize);   
    if(pMem == NULL)   
    {   
        CloseHandle(hFile);   
        ShowErr(MemErr);   
        return;   
    }   

    memcpy(pMem,image_dos_header,sizeof(IMAGE_DOS_HEADER));   
    if((dwDosStubSize&0x80000000)==0x00000000)   
    {   
        memcpy(pMem+dwDosStubOffset,pDosStub,dwDosStubSize);   
    }   
    memcpy(pMem+image_dos_header->e_lfanew,   
        image_nt_headers,   
        sizeof(IMAGE_NT_HEADERS));   
   
    dwRO_first_section=image_dos_header->e_lfanew+sizeof(IMAGE_NT_HEADERS);   
   
    SectionNum=image_nt_headers->FileHeader.NumberOfSections;   

    for( i=0;i<SectionNum;i++)    
    {   
        CopyMemory(pMem+dwRO_first_section+i*sizeof(IMAGE_SECTION_HEADER),   
            image_section_header[i],   
            sizeof(IMAGE_SECTION_HEADER));   
    }   

    for(i=0;i<SectionNum;i++)   
    {   
        CopyMemory(pMem+image_section_header[i]->PointerToRawData,   
                image_section[i],   
                image_section_header[i]->SizeOfRawData);   
    }   

    SetFilePointer(hFile,0,NULL,FILE_BEGIN);   
    WriteFile(hFile,pMem,dwFileSize,&dwBytesWritten,NULL);   
       
    SetFilePointer(hFile,dwFileSize,NULL,FILE_BEGIN);   
    SetEndOfFile(hFile);   
    CloseHandle(hFile);
}

void CPELibrary::FindApiCalls()
{
	char *dllName = NULL;
	DWORD dll_import = 0;
	DWORD dwThunk = 0;
	const IMAGE_IMPORT_DESCRIPTOR *lpImp = NULL;
   const IMAGE_THUNK_DATA *itd = NULL;
	const IMAGE_IMPORT_BY_NAME *name_import = NULL;

	dll_import = RVA2Offset(image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	if ( !dll_import )
	   return;

	lpImp = (IMAGE_IMPORT_DESCRIPTOR *)((DWORD)pMem + dll_import);      
	
	while ( lpImp->Name )
	{
		dllName = (char *)((DWORD)(DWORD)pMem + this->RVA2Offset(lpImp->Name)); 
		if ( stricmp( dllName, "kernel32.dll" ) )
		{
			lpImp++;
			continue;
		}

		dwThunk = (lpImp->OriginalFirstThunk ? lpImp->OriginalFirstThunk : lpImp->FirstThunk);
		itd = (IMAGE_THUNK_DATA *)((DWORD)pMem + this->RVA2Offset(dwThunk));    

		dwThunk = lpImp->FirstThunk;

		while ( itd->u1.AddressOfData )
		{
			name_import = (IMAGE_IMPORT_BY_NAME *)((DWORD)pMem + this->RVA2Offset((DWORD)itd->u1.AddressOfData));    

			if ( !stricmp( (char *)name_import->Name, "LoadLibraryA" ) )
				LoadLibrary_addr = image_nt_headers->OptionalHeader.ImageBase + dwThunk;
			else if ( !stricmp( (char *)name_import->Name, "GetProcAddress" ) )
				GetProcAddress_addr = image_nt_headers->OptionalHeader.ImageBase + dwThunk;
			
			itd++;
			dwThunk += sizeof(DWORD);
		}

		break;
	}
}

void CPELibrary::Obfuscate()
{
	image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = image_nt_headers->FileHeader.TimeDateStamp;
	image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size = rand() * 40 + 1;
}
