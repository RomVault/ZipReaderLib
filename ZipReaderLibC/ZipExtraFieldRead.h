#pragma once
#include "ZipHeader.h"
#include "Zip.h"
class ZipExtraFieldRead
{

private:
	static unsigned short ToUInt16(char* bytes, int offset);
	static unsigned int ToUInt32(char* bytes, int offset);
	static int ToInt32(char* bytes, int offset);
	static unsigned long long int ToUInt64(char* bytes, int offset);
	static long long int ToInt64(char* bytes, int offset);


	static long long int UtcTicksFromNtfsDateTime(long long ntfsTicks);
	static long long int UtcTicksFromUnixDateTime(int linuxSeconds);


public:
	static ZipReturn ReadExtraField(ZipHeader* zipHeader, bool centralDir);
};

