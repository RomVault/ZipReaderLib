#pragma once
class CRC
{
public:
	CRC();
	~CRC();
	void Reset();
	void SlurpBlock(unsigned char* block, int offset, int count);
	unsigned int Crc32ResultU();
};

