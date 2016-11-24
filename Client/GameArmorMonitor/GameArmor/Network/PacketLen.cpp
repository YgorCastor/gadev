#include "PacketLen.h"

#include <stdlib.h>
#include <Windows.h>
#include "../../Utils/MemoryTools.h"

static unsigned int plenAddr;

void insert_jmp(unsigned char *start, void *target)
{
	start[0] = 0xE9;

	*((unsigned int *)(start + 1)) = (unsigned int)target - (unsigned int)start - 5;
}

void insert_call(unsigned char *start, void *target)
{
	start[0] = 0xE8;

	*((DWORD *)(start + 1)) = (unsigned int)target - (unsigned int)start - 5;
}

void insert_mov_ecx_to_addr(BYTE *start, unsigned int *addr)
{
	start[0] = 0x89;
	start[1] = 0x0d;

	*((DWORD *)(start + 2)) = (unsigned int)addr;
}

void insert_plen_copy(unsigned char *start) 
{
	unsigned char *cave = (unsigned char *)VirtualAlloc(NULL, 16, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	unsigned char *mov = &cave[0];
	unsigned char *call = &cave[6];
	unsigned char *jmp = &cave[11];
	void *target = (start + 5) + *((unsigned int *)(start + 1));

	insert_mov_ecx_to_addr(mov, &plenAddr);
	insert_call(call, target);
	insert_jmp(jmp, start + 5);

	DWORD oldprot;
	VirtualProtect(start, 5, PAGE_EXECUTE_READWRITE, &oldprot);
	insert_jmp(start, cave);
	VirtualProtect(start, 5, oldprot, &oldprot);
}

unsigned char *SkipInstruction(unsigned char *func) 
{
    if (*func != 0xCC)
    {
        // Skip prefixes F0h, F2h, F3h, 66h, 67h, D8h-DFh, 2Eh, 36h, 3Eh, 26h, 64h and 65h
        int operandSize = 4;
        int FPU = 0;
        while(*func == 0xF0 || 
              *func == 0xF2 || 
              *func == 0xF3 || 
             (*func & 0xFC) == 0x64 || 
             (*func & 0xF8) == 0xD8 ||
             (*func & 0x7E) == 0x62)
        { 
            if(*func == 0x66) 
            { 
                operandSize = 2; 
            }
            else if((*func & 0xF8) == 0xD8) 
            {
                FPU = *func++;
                break;
            }

            func++;
        }

        // Skip two-byte opcode byte 
        bool twoByte = false; 
        if(*func == 0x0F) 
        { 
            twoByte = true; 
            func++; 
        } 

        // Skip opcode byte 
        unsigned char opcode = *func++; 

        // Skip mod R/M byte 
        unsigned char modRM = 0xFF; 
        if(FPU) 
        { 
            if((opcode & 0xC0) != 0xC0) 
            { 
                modRM = opcode; 
            } 
        } 
        else if(!twoByte) 
        { 
            if((opcode & 0xC4) == 0x00 || 
               (opcode & 0xF4) == 0x60 && ((opcode & 0x0A) == 0x02 || (opcode & 0x09) == 0x9) || 
               (opcode & 0xF0) == 0x80 || 
               (opcode & 0xF8) == 0xC0 && (opcode & 0x0E) != 0x02 || 
               (opcode & 0xFC) == 0xD0 || 
               (opcode & 0xF6) == 0xF6) 
            { 
                modRM = *func++; 
            } 
        } 
        else 
        { 
            if((opcode & 0xF0) == 0x00 && (opcode & 0x0F) >= 0x04 && (opcode & 0x0D) != 0x0D || 
               (opcode & 0xF0) == 0x30 || 
               opcode == 0x77 || 
               (opcode & 0xF0) == 0x80 || 
               (opcode & 0xF0) == 0xA0 && (opcode & 0x07) <= 0x02 || 
               (opcode & 0xF8) == 0xC8) 
            { 
                // No mod R/M byte 
            } 
            else 
            { 
                modRM = *func++; 
            } 
        } 

        // Skip SIB
        if((modRM & 0x07) == 0x04 &&
           (modRM & 0xC0) != 0xC0)
        {
            func += 1;   // SIB
        }

        // Skip displacement
        if((modRM & 0xC5) == 0x05) func += 4;   // Dword displacement, no base 
        if((modRM & 0xC0) == 0x40) func += 1;   // Byte displacement 
        if((modRM & 0xC0) == 0x80) func += 4;   // Dword displacement 

        // Skip immediate 
        if(FPU) 
        { 
            // Can't have immediate operand 
        } 
        else if(!twoByte) 
        { 
            if((opcode & 0xC7) == 0x04 || 
               (opcode & 0xFE) == 0x6A ||   // PUSH/POP/IMUL 
               (opcode & 0xF0) == 0x70 ||   // Jcc 
               opcode == 0x80 || 
               opcode == 0x83 || 
               (opcode & 0xFD) == 0xA0 ||   // MOV 
               opcode == 0xA8 ||            // TEST 
               (opcode & 0xF8) == 0xB0 ||   // MOV
               (opcode & 0xFE) == 0xC0 ||   // RCL 
               opcode == 0xC6 ||            // MOV 
               opcode == 0xCD ||            // INT 
               (opcode & 0xFE) == 0xD4 ||   // AAD/AAM 
               (opcode & 0xF8) == 0xE0 ||   // LOOP/JCXZ 
               opcode == 0xEB || 
               opcode == 0xF6 && (modRM & 0x30) == 0x00)   // TEST 
            { 
                func += 1; 
            } 
            else if((opcode & 0xF7) == 0xC2) 
            { 
                func += 2;   // RET 
            } 
            else if((opcode & 0xFC) == 0x80 || 
                    (opcode & 0xC7) == 0x05 || 
                    (opcode & 0xF8) == 0xB8 ||
                    (opcode & 0xFE) == 0xE8 ||      // CALL/Jcc 
                    (opcode & 0xFE) == 0x68 || 
                    (opcode & 0xFC) == 0xA0 || 
                    (opcode & 0xEE) == 0xA8 || 
                    opcode == 0xC7 || 
                    opcode == 0xF7 && (modRM & 0x30) == 0x00) 
            { 
                func += operandSize; 
            } 
        } 
        else 
        { 
            if(opcode == 0xBA ||            // BT 
               opcode == 0x0F ||            // 3DNow! 
               (opcode & 0xFC) == 0x70 ||   // PSLLW 
               (opcode & 0xF7) == 0xA4 ||   // SHLD 
               opcode == 0xC2 || 
               opcode == 0xC4 || 
               opcode == 0xC5 || 
               opcode == 0xC6) 
            { 
                func += 1; 
            } 
            else if((opcode & 0xF0) == 0x80) 
            {
                func += operandSize;   // Jcc -i
            }
        }
    } 
	else 
		func++;

    return func;
}

PacketLen::PacketLen()
{
}

PacketLen::~PacketLen()
{
}

extern char *defaultEndAddress;

void PacketLen::Boot()
{
	int add = 0;

	this->packet_start = (unsigned char *)GaFindPattern("C7 44 ?w 87 01 00 00");
	add = 8;

	if (!this->packet_start) 
	{
		packet_start = (unsigned char *)GaFindPattern("C7 85 ?d 87 01 00 00");
		add = 10;
	}

	if (!this->packet_start) 
	{
		packet_start = (unsigned char *)GaFindPattern("C7 45 ?b 87 01 00 00");
		add = 7;
	}

	this->packet_start += add;
	this->packet_end = (unsigned char *)GaFindPatternEx((char *)this->packet_start, defaultEndAddress, "8B E5 5D C3");

	int calls = 0;
	while (this->packet_start < this->packet_end) 
	{
		if (this->packet_start[0] == 0xE8) 
		{
			calls++;
			
			if (calls == 2) 
				break;
		}

		if (this->packet_start[0] == 0xC3)
			break;

		this->packet_start = SkipInstruction(this->packet_start);
	}

	insert_plen_copy(this->packet_start);

	this->lowest_id = 0x64;
	this->highest_id = 0x02AF;

	memset(this->packet_len, 0x00, sizeof(this->packet_len));
}

void PacketLen::Load()
{
	if (!plenAddr)
		return;

	this->Traverse(((struct PacketLenStruct *)(*(unsigned int*)(plenAddr + 4)))->parent);
}

int PacketLen::GetPacketLen(unsigned short id)
{
	if (id > this->highest_id)
		return 0;

	if (id < this->lowest_id)
		return 0;

	return this->packet_len[id];
}

void PacketLen::SetPacketLen(unsigned short id, int val)
{
	if (id < 0 || id > PACKETLEN_MAX)
		return;

	if (id > this->highest_id)
		this->highest_id = id;
	
	if (id < this->lowest_id)
		this->lowest_id = id;

	if (val <= 0)
		val = 1;

	this->packet_len[id] = val;
}

void PacketLen::Traverse(struct PacketLenStruct *node)
{
	if (node->parent == NULL || node->key >= PACKETLEN_MAX || node->key == 0)
		return;

	this->Traverse(node->left);
	this->SetPacketLen(node->key, node->value);
	this->Traverse(node->right);
}
