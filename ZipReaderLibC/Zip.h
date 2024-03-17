#pragma once

#include "ZipHeader.h"
#include <fstream>
using namespace std;


enum ZipReturn
{
	ZipGood,
	ZipLocalFileHeaderError,
	ZipCentralDirError,
	ZipEndOfCentralDirectoryError,
	Zip64EndOfCentralDirError,
	Zip64EndOfCentralDirectoryLocatorError,
	ZipFileNameToLong,
	ZipErrorOpeningFile,
	ZipErrorFileNotFound,
	ZipErrorReadingFile,
};



class Zip
{
	const unsigned int EndOfCentralDirSignature = 0x06054b50;
	const unsigned int Zip64EndOfCentralDirSignature = 0x06064b50;
	const unsigned int Zip64EndOfCentralDirectoryLocator = 0x07064b50;


	const unsigned int LocalFileHeaderSignature = 0x04034b50;
	const unsigned int CentralDirectoryHeaderSignature = 0x02014b50;


	fstream _zipFs;

public:
	unsigned int _localFilesCount;

	unsigned long long int _centralDirSize;
	unsigned long long int _centralDirStart;
	unsigned long long int _endOfCentralDir64;

	unsigned char* _fileComment;

	long long int fileSize;
	bool ExtraDataFoundOnEndOfFile = false;

	bool _zip64;

	unsigned long long int offset = 0;

	ZipHeader* _centralDirectoryHeaders;
	ZipHeader* _localFileHeaders;



public:
	ZipReturn ZipFileOpen(const wchar_t* zipFilePath);

private:
	ZipReturn ZipFileReadHeaders();
	ZipReturn FindEndOfCentralDirSignature();
	ZipReturn EndOfCentralDirRead();
	ZipReturn Zip64EndOfCentralDirRead();
	ZipReturn Zip64EndOfCentralDirectoryLocatorRead();

	ZipReturn CentralDirectoryHeaderReader(unsigned long long offset, ZipHeader* centralFile);
	ZipReturn LocalFileHeaderReader(unsigned long long relativeOffsetOfLocalHeader, unsigned long long compressedSize, ZipHeader* localFile);

};

