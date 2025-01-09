#pragma once
struct PacketHeader
{
	short checkValue;
	int PacketType;
	int PacketSize;
};

struct Packet
{
	PacketHeader header;
	char payload[128];
};

