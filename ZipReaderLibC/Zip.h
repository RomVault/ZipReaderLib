#pragma once

#include "ZipHeader.h"
#include <iostream>
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

	unsigned int _centralDirSize;
	unsigned int _centralDirStart;
	unsigned long _endOfCentralDir64;

	unsigned char* _fileComment;

	long fileSize;
	bool ExtraDataFoundOnEndOfFile = false;

	bool _zip64;

	unsigned long offset = 0;

	ZipHeader* _centralDirectoryHeaders;
	ZipHeader* _localFileHeaders;



public:
	ZipReturn ZipFileOpen(const char* zipFilePath);

private:
	ZipReturn ZipFileReadHeaders();
	ZipReturn FindEndOfCentralDirSignature();
	ZipReturn EndOfCentralDirRead();
	ZipReturn Zip64EndOfCentralDirRead();
	ZipReturn Zip64EndOfCentralDirectoryLocatorRead();

	ZipReturn CentralDirectoryHeaderReader(unsigned long offset, ZipHeader* centralFile);
	ZipReturn LocalFileHeaderReader(unsigned long relativeOffsetOfLocalHeader, unsigned long compressedSize, ZipHeader* localFile);

};

