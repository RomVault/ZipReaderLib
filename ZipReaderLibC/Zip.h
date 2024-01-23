#pragma once

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

	unsigned int _localFilesCount;
	unsigned int _centralDirSize;
	unsigned int _centralDirStart;
	unsigned char* _fileComment;


public:
	ZipReturn ZipFileOpen(const char* zipFilePath);

private:
	ZipReturn ZipFileReadHeaders(fstream& zipFs);
	ZipReturn FindEndOfCentralDirSignature(fstream& zipFs);
	ZipReturn EndOfCentralDirRead(fstream& zipFs);

};

