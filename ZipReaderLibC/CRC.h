#pragma once
class CRC
{
public:
	CRC();
	~CRC();
	void Reset();
	void SlurpBlock(char* block, int offset, int count);
	unsigned int Crc32ResultU();
};

