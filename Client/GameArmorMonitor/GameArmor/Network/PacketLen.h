#pragma once

#define PACKETLEN_MAX 0xB000

struct PacketLenStruct
{ 
	struct PacketLenStruct *left, *parent, *right; 
	unsigned int key; 
	int value; 
};

class PacketLen
{
public:
	PacketLen();
	~PacketLen();

	void Boot();
	void Load();

	int GetPacketLen(unsigned short id);
	void SetPacketLen(unsigned short id, int val);

private:
	int packet_len[PACKETLEN_MAX];

	void Traverse(struct PacketLenStruct *node);

	unsigned int highest_id;
	unsigned int lowest_id;

	unsigned char *packet_start;
	unsigned char *packet_end;
};
