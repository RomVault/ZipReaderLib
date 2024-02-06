#include <iostream>
#include "Zip.h"

int main()
{
	Zip zip = Zip();
	ZipReturn zr = zip.ZipFileOpen("\\\\10.0.4.9\\Dos-Files\\TotalDosCollection\\TDC Daily\\Games\\Files\\1988\\Bosse des Maths 3ème, La (Fr) (1988)(Coktel Vision) [Educational].zip");

	for (int i = 0; i < zip._localFilesCount; i++)
	{
		std::cout << "File: " << i << " " << zip._centralDirectoryHeaders[i].bFileName << " " << zip._centralDirectoryHeaders[i].UncompressedSize << " " << std::hex << zip._centralDirectoryHeaders[i].CRC << std::dec << " ";

		unsigned short dosFileDate = (unsigned short)((zip._centralDirectoryHeaders[i].HeaderLastModified >> 16) & 0xffff);
		unsigned short dosFileTime = (unsigned short)(zip._centralDirectoryHeaders[i].HeaderLastModified & 0xffff);

		int second = (dosFileTime & 0x1f) << 1;
		int minute = (dosFileTime >> 5) & 0x3f;
		int hour = (dosFileTime >> 11) & 0x1f;

		int day = dosFileDate & 0x1f;
		int month = (dosFileDate >> 5) & 0x0f;
		int year = ((dosFileDate >> 9) & 0x7f) + 1980;

		std::cout << year << "/" << month << "/" << day << " " << hour << ":" << minute << ":" << second << std::endl;
	}

}

