#pragma once

#ifndef _cplusplus

extern void _stdcall VirtualizerStart(void);
extern void _stdcall VirtualizerEnd(void);
extern void _stdcall Virtualizer1Start(void);
extern void _stdcall Virtualizer1End(void);
extern void _stdcall Virtualizer2Start(void);
extern void _stdcall Virtualizer2End(void);
extern void _stdcall Virtualizer3Start(void);
extern void _stdcall Virtualizer3End(void);
extern void _stdcall Virtualizer4Start(void);
extern void _stdcall Virtualizer4End(void);
extern void _stdcall Virtualizer5Start(void);
extern void _stdcall Virtualizer5End(void);
extern void _stdcall VirtualizerMutate1Start(void);
extern void _stdcall VirtualizerMutate2Start(void);
extern void _stdcall VirtualizerMutate3Start(void);

#else

extern "C" void _stdcall VirtualizerStart(void);
extern "C" void _stdcall VirtualizerEnd(void);
extern "C" void _stdcall Virtualizer1Start(void);
extern "C" void _stdcall Virtualizer1End(void);
extern "C" void _stdcall Virtualizer2Start(void);
extern "C" void _stdcall Virtualizer2End(void);
extern "C" void _stdcall Virtualizer3Start(void);
extern "C" void _stdcall Virtualizer3End(void);
extern "C" void _stdcall Virtualizer4Start(void);
extern "C" void _stdcall Virtualizer4End(void);
extern "C" void _stdcall Virtualizer5Start(void);
extern "C" void _stdcall Virtualizer5End(void);
extern "C" void _stdcall VirtualizerMutate1Start(void);
extern "C" void _stdcall VirtualizerMutate2Start(void);
extern "C" void _stdcall VirtualizerMutate3Start(void);

#endif

#ifdef _DEBUG
#define START_PROTECTION()
#define END_PROTECTION()
#else
#define START_PROTECTION() VirtualizerStart()
#define END_PROTECTION() VirtualizerEnd()
#endif

#define VIRTUALIZER_START \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0C \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
 
#define VIRTUALIZER_END \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0D \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \

 #define VIRTUALIZER_MUTATE1_START \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0C \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x01 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \

 #define VIRTUALIZER_MUTATE2_START \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0C \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x02 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \

 #define VIRTUALIZER_MUTATE3_START \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0C \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x03 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \


#define VIRTUALIZER1_START \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0C \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x01 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
 
#define VIRTUALIZER1_END \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0D \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \


#define VIRTUALIZER2_START \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0C \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x02 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
 
#define VIRTUALIZER2_END \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0D \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \

#define VIRTUALIZER3_START \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0C \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x03 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
 
#define VIRTUALIZER3_END \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0D \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \

#define VIRTUALIZER4_START \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0C \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x04 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
 
#define VIRTUALIZER4_END \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0D \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \


#define VIRTUALIZER5_START \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0C \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x05 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
 
#define VIRTUALIZER5_END \
	__asm _emit 0xEB \
	__asm _emit 0x10 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
	__asm _emit 0x0D \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x00 \
	__asm _emit 0x43 \
	__asm _emit 0x56 \
	__asm _emit 0x20 \
	__asm _emit 0x20 \
