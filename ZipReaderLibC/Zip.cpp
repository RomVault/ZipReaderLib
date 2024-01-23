#include "Zip.h"
#include <iostream>
#include <fstream>
using namespace std;



ZipReturn Zip::ZipFileOpen(const char* zipFilePath)
{
	fstream zipFs;
	zipFs.open(zipFilePath, ios::binary | ios::in);
	return ZipFileReadHeaders(zipFs);
}

ZipReturn Zip::ZipFileReadHeaders(fstream& zipFs)
{
	ZipReturn zRet;
	zRet = FindEndOfCentralDirSignature(zipFs);
	if (zRet != ZipGood)
	{
		zipFs.close();
		return zRet;
	}

	unsigned long endOfCentralDir = zipFs.tellg();
	zRet = EndOfCentralDirRead(zipFs);
	if (zRet != ZipGood)
	{
		zipFs.close();
		return zRet;
	}

	zipFs.close();
	return zRet;
}

ZipReturn Zip::FindEndOfCentralDirSignature(fstream& zipFs)
{
	zipFs.seekg(0, ios::end);
	long fileSize = zipFs.tellg();
	long maxBackSearch = 0xffff;

	if (fileSize < maxBackSearch)
	{
		maxBackSearch = fileSize;
	}

	const long buffSize = 0x400;

	unsigned char buffer[buffSize + 4];

	long backPosition = 4;
	while (backPosition < maxBackSearch)
	{
		backPosition += buffSize;
		if (backPosition > maxBackSearch)
		{
			backPosition = maxBackSearch;
		}

		long readSize = backPosition > buffSize + 4 ? buffSize + 4 : backPosition;

		zipFs.seekg(fileSize - backPosition, ios::beg);
		zipFs.read((char*)buffer, readSize);


		for (long i = readSize - 4; i >= 0; i--)
		{
			if (buffer[i] != 0x50 || buffer[i + 1] != 0x4b || buffer[i + 2] != 0x05 || buffer[i + 3] != 0x06)
			{
				continue;
			}

			zipFs.seekg(fileSize - backPosition + i, ios::beg);

			return ZipGood;
		}
	}
	return ZipCentralDirError;
}

const unsigned int EndOfCentralDirSignature = 0x06054b50;
const unsigned int Zip64EndOfCentralDirSignature = 0x06064b50;
const unsigned int Zip64EndOfCentralDirectoryLocator = 0x07064b50;

ZipReturn Zip::EndOfCentralDirRead(fstream& zipFs)
{
	unsigned int thisSignature;
	zipFs.read((char*)&thisSignature, 4);
	if (thisSignature != EndOfCentralDirSignature)
	{
		return ZipEndOfCentralDirectoryError;
	}

	unsigned short tUShort;
	zipFs.read((char*)&tUShort, 2); // NumberOfThisDisk
	if (tUShort != 0)
	{
		return ZipEndOfCentralDirectoryError;
	}

	zipFs.read((char*)&tUShort, 2); // NumberOfThisDiskCenterDir
	if (tUShort != 0)
	{
		return ZipEndOfCentralDirectoryError;
	}

	zipFs.read((char*)&_localFilesCount, 2); // TotalNumberOfEntriesDisk

	zipFs.read((char*)&tUShort, 2); // TotalNumber of entries in the central directory 
	if (tUShort != _localFilesCount)
	{
		return ZipEndOfCentralDirectoryError;
	}

	zipFs.read((char*)&_centralDirSize, 4); // SizeOfCentralDir
	zipFs.read((char*)&_centralDirStart, 4); // Offset

	unsigned short zipFileCommentLength;
	zipFs.read((char*)&zipFileCommentLength, 2); // ZipFileCommentLength

	_fileComment = new unsigned char[zipFileCommentLength];
	zipFs.read((char*)_fileComment, zipFileCommentLength); // ZipFileComment

	//	ExtraDataFoundOnEndOfFile = _zipFs.Position != _zipFs.Length;

	return ZipGood;
}


