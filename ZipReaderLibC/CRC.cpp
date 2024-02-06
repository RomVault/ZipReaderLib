#include "CRC.h"

unsigned int polynomial = 0xEDB88320;
int crcNumTables = 8;


unsigned int* CRC32Lookup;
unsigned int _crc;



CRC::CRC()
{
    CRC32Lookup = new unsigned int[256 * crcNumTables];
    int i;
    for (i = 0; i < 256; i++)
    {
        unsigned int r = (unsigned int)i;
        for (int j = 0; j < 8; j++)
        {
            r = (r >> 1) ^ (polynomial & ~((r & 1) - 1));
        }

        CRC32Lookup[i] = r;
    }

    for (; i < 256 * crcNumTables; i++)
    {
        unsigned int r = CRC32Lookup[i - 256];
        CRC32Lookup[i] = CRC32Lookup[r & 0xFF] ^ (r >> 8);
    }
    Reset();
}

void CRC::Reset()
{
    _crc = 0xffffffffu;
}


void CRC::SlurpBlock(unsigned char* block, int offset, int count)
{
    unsigned int crc = _crc;

    for (; (offset & 7) != 0 && count != 0; count--)
        crc = (crc >> 8) ^ CRC32Lookup[(char)crc ^ block[offset++]];

    if (count >= 8)
    {
        int end = (count - 8) & ~7;
        count -= end;
        end += offset;

        while (offset != end)
        {
            crc ^= (unsigned int)(block[offset] + (block[offset + 1] << 8) + (block[offset + 2] << 16) + (block[offset + 3] << 24));
            unsigned int high = (unsigned int)(block[offset + 4] + (block[offset + 5] << 8) + (block[offset + 6] << 16) + (block[offset + 7] << 24));
            offset += 8;

            crc = CRC32Lookup[(char)crc + 0x700]
                ^ CRC32Lookup[(char)(crc >>= 8) + 0x600]
                ^ CRC32Lookup[(char)(crc >>= 8) + 0x500]
                ^ CRC32Lookup[(char)(crc >> 8) + 0x400]
                ^ CRC32Lookup[(char)high + 0x300]
                ^ CRC32Lookup[(char)(high >>= 8) + 0x200]
                ^ CRC32Lookup[(char)(high >>= 8) + 0x100]
                ^ CRC32Lookup[(char)(high >> 8) + 0x000];
        }
    }

    while (count-- != 0)
    {
        crc = (crc >> 8) ^ CRC32Lookup[(char)crc ^ block[offset++]];
    }

    _crc = crc;

}

unsigned int CRC::Crc32ResultU()
{
	return ~_crc;
}

CRC::~CRC()
{
	delete[] CRC32Lookup;
}