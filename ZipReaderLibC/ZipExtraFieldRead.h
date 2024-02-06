#pragma once
#include "ZipHeader.h"
#include "Zip.h"
class ZipExtraFieldRead
{

private:
	static unsigned short ToUInt16(unsigned char* bytes, int offset);
	static unsigned int ToUInt32(unsigned char* bytes, int offset);
	static int ToInt32(unsigned char* bytes, int offset);
	static unsigned long long ToUInt64(unsigned char* bytes, int offset);
	static long long ToInt64(unsigned char* bytes, int offset);


	static long long UtcTicksFromNtfsDateTime(long long ntfsTicks);
	static long long UtcTicksFromUnixDateTime(int linuxSeconds);


public:
	static ZipReturn ReadExtraField(ZipHeader* zipHeader, bool centralDir);
};

