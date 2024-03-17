#include <iostream>
#include "Zip.h"
#include <Windows.h>

int main()
{
	SetConsoleOutputCP(CP_UTF8);

	Zip zip = Zip();

	//   T:\TotalDosCollection\TDC Daily\Games\Images\CD\1996\Pandora Akte, Die v1.00 (De) (1996)(Access Software, Inc.) [bin-cue].zip
	//    T:\TotalDosCollection\TDC Daily\Games\Images\CD\1996\Pandora Directive, The v1.00 (1996)(Access Software, Inc.) [bin-cue].zip

	const wchar_t* filename = L"T:\\TotalDosCollection\\TDC Daily\\Games\\Images\\CD\\1996\\Pandora Directive, The v1.00 (1996)(Access Software, Inc.) [bin-cue].zip";

	ZipReturn zr = zip.ZipFileOpen(filename);

	for (unsigned int i = 0; i < zip._localFilesCount; i++)
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

