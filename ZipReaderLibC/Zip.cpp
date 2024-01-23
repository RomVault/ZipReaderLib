#include "Zip.h"
#include <iostream>
#include <fstream>
using namespace std;


ZipReturn Zip::ZipFileOpen(const char* zipFilePath)
{
	_zipFs.open(zipFilePath, ios::binary | ios::in);

	ZipReturn ret = ZipFileReadHeaders();
	_zipFs.close();
	return ret;
}

ZipReturn Zip::ZipFileReadHeaders()
{
	ZipReturn zRet;
	zRet = FindEndOfCentralDirSignature();
	if (zRet != ZipGood)
	{
		return zRet;
	}

	unsigned long endOfCentralDir = _zipFs.tellg();
	zRet = EndOfCentralDirRead();
	if (zRet != ZipGood)
	{
		return zRet;
	}

	// check if ZIP64 header is required
	bool zip64Required = (_centralDirStart == 0xffffffff || _centralDirSize == 0xffffffff || _localFilesCount == 0xffff);

	_zipFs.seekg(endOfCentralDir - 20, ios::beg);

	zRet = Zip64EndOfCentralDirectoryLocatorRead();
	if (zRet == ZipGood)
	{
		_zipFs.seekg((long)_endOfCentralDir64, ios::beg);
		zRet = Zip64EndOfCentralDirRead();
		if (zRet == ZipGood)
		{
			_zip64 = true;
			endOfCentralDir = _endOfCentralDir64;
		}
	}

	if (zip64Required && !_zip64)
		return Zip64EndOfCentralDirError;

	offset = (endOfCentralDir - _centralDirSize) - _centralDirStart;

	_centralDirStart += offset;

	// now read the central directory
	_zipFs.seekg(_centralDirStart, ios::beg);

	/*
	_centralDirectoryHeaders = new ZipHeader[_localFilesCount];
	for (int i = 0; i < _localFilesCount; i++)
	{
		zRet = CentralDirectoryHeaderRead(_zipFs, offset, out ZipHeader CentralFile);
		if (zRet != ZipGood)
			return zRet;

		_centralDirectoryHeaders[i] = CentralFile;
	}

	_localFileHeaders = new ZipHeader[_localFilesCount];
	for (int i = 0; i < _localFilesCount; i++)
	{
		zRet = LocalFileHeaderRead(_zipFs, _centralDirectoryHeaders[i].RelativeOffsetOfLocalHeader, _centralDirectoryHeaders[i].CompressedSize, out ZipHeader LocalFile);
		if (zRet != ZipGood)
			return zRet;

		_localFileHeaders[i] = LocalFile;
	}
	*/

	return ZipGood;
}


ZipReturn Zip::FindEndOfCentralDirSignature()
{
	_zipFs.seekg(0, ios::end);
	fileSize = _zipFs.tellg();
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

		_zipFs.seekg(fileSize - backPosition, ios::beg);
		_zipFs.read((char*)buffer, readSize);


		for (long i = readSize - 4; i >= 0; i--)
		{
			if (buffer[i] != 0x50 || buffer[i + 1] != 0x4b || buffer[i + 2] != 0x05 || buffer[i + 3] != 0x06)
			{
				continue;
			}

			_zipFs.seekg(fileSize - backPosition + i, ios::beg);

			return ZipGood;
		}
	}
	return ZipCentralDirError;
}


ZipReturn Zip::EndOfCentralDirRead()
{
	unsigned int thisSignature;
	_zipFs.read((char*)&thisSignature, 4);
	if (thisSignature != EndOfCentralDirSignature)
	{
		return ZipEndOfCentralDirectoryError;
	}

	unsigned short tUShort;
	_zipFs.read((char*)&tUShort, 2); // NumberOfThisDisk
	if (tUShort != 0)
	{
		return ZipEndOfCentralDirectoryError;
	}

	_zipFs.read((char*)&tUShort, 2); // NumberOfThisDiskCenterDir
	if (tUShort != 0)
	{
		return ZipEndOfCentralDirectoryError;
	}

	_zipFs.read((char*)&_localFilesCount, 2); // TotalNumberOfEntriesDisk

	_zipFs.read((char*)&tUShort, 2); // TotalNumber of entries in the central directory 
	if (tUShort != _localFilesCount)
	{
		return ZipEndOfCentralDirectoryError;
	}

	_zipFs.read((char*)&_centralDirSize, 4); // SizeOfCentralDir
	_zipFs.read((char*)&_centralDirStart, 4); // Offset

	unsigned short zipFileCommentLength;
	_zipFs.read((char*)&zipFileCommentLength, 2); // ZipFileCommentLength

	_fileComment = new unsigned char[zipFileCommentLength];
	_zipFs.read((char*)_fileComment, zipFileCommentLength); // ZipFileComment

	ExtraDataFoundOnEndOfFile = _zipFs.tellg() != fileSize;

	return ZipGood;
}


ZipReturn Zip::Zip64EndOfCentralDirRead()
{
	unsigned int thisSignature;
	_zipFs.read((char*)&thisSignature, 4);
	if (thisSignature != Zip64EndOfCentralDirSignature)
	{
		return ZipEndOfCentralDirectoryError;
	}

	unsigned long tULong;
	_zipFs.read((char*)&tULong, 8); // Size of zip64 end of central directory record
	if (tULong != 44)
	{
		return Zip64EndOfCentralDirError;
	}

	unsigned short tUShort;
	_zipFs.read((char*)&tUShort, 2); // version made by

	_zipFs.read((char*)&tUShort, 2); // version needed to extract
	if (tUShort != 45)
	{
		return Zip64EndOfCentralDirError;
	}

	unsigned int tUInt;
	_zipFs.read((char*)&tUInt, 4); // number of this disk
	if (tUInt != 0)
	{
		return Zip64EndOfCentralDirError;
	}

	_zipFs.read((char*)&tUInt, 4); // number of the disk with the start of the central directory
	if (tUInt != 0)
	{
		return Zip64EndOfCentralDirError;
	}

	_zipFs.read((char*)&_localFilesCount, 8); // total number of entries in the central directory on this disk

	_zipFs.read((char*)&tULong, 8); // total number of entries in the central directory
	if (tULong != _localFilesCount)
	{
		return Zip64EndOfCentralDirError;
	}

	_zip64 = true;
	_zipFs.read((char*)&_centralDirSize, 8); // size of the central directory

	_zipFs.read((char*)&_centralDirStart, 8); // offset of start of central directory with respect to the starting disk number

	return ZipGood;
}


ZipReturn Zip::Zip64EndOfCentralDirectoryLocatorRead()
{
	unsigned int thisSignature;
	_zipFs.read((char*)&thisSignature, 4);
	if (thisSignature != Zip64EndOfCentralDirectoryLocator)
	{
		return ZipEndOfCentralDirectoryError;
	}

	unsigned int tUInt;
	_zipFs.read((char*)&tUInt, 4); // number of the disk with the start of the zip64 end of central directory
	if (tUInt != 0)
	{
		return Zip64EndOfCentralDirectoryLocatorError;
	}

	_zipFs.read((char*)&_endOfCentralDir64, 8); // relative offset of the zip64 end of central directory record

	_zipFs.read((char*)&tUInt, 4); // total number of disks
	if (tUInt > 1)
	{
		return Zip64EndOfCentralDirectoryLocatorError;
	}

	return ZipGood;
}



