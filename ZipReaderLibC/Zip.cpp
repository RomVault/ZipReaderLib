#include "Zip.h"
#include "ZipHeader.h"
#include "ZipExtraFieldRead.h"
#include "CodePage437.h"
#include <fstream>
using namespace std;


ZipReturn Zip::ZipFileOpen(const wchar_t* zipFilePath)
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

	long endOfCentralDir = (long)_zipFs.tellg();
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

	_centralDirectoryHeaders = new ZipHeader[_localFilesCount];
	for (unsigned int i = 0; i < _localFilesCount; i++)
	{
		zRet = CentralDirectoryHeaderReader(offset, &_centralDirectoryHeaders[i]);
		if (zRet != ZipGood)
			return zRet;
	}

	_localFileHeaders = new ZipHeader[_localFilesCount];
	for (unsigned int i = 0; i < _localFilesCount; i++)
	{
		zRet = LocalFileHeaderReader(_centralDirectoryHeaders[i].RelativeOffsetOfLocalHeader, _centralDirectoryHeaders[i].CompressedSize, &_localFileHeaders[i]);
		if (zRet != ZipGood)
			return zRet;
	}

	return ZipGood;
}





ZipReturn Zip::FindEndOfCentralDirSignature()
{
	_zipFs.seekg(0, ios::end);
	fileSize = (long)_zipFs.tellg();
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

ZipReturn Zip::CentralDirectoryHeaderReader(unsigned long long offset, ZipHeader* centralFile)
{
	unsigned int thisSignature;
	_zipFs.read((char*)&thisSignature, 4);
	if (thisSignature != CentralDirectoryHeaderSignature)
		return ZipCentralDirError;

	_zipFs.read((char*)&centralFile->VersionMadeBy, 2); // Version Made By
	_zipFs.read((char*)&centralFile->VersionNeededToExtract, 2); // Version Needed To Extract

	_zipFs.read((char*)&centralFile->GeneralPurposeBitFlag, 2); // General Purpose Bit Flag
	_zipFs.read((char*)&centralFile->CompressionMethod, 2); // Compression Method

	unsigned short lastModFileTime;
	_zipFs.read((char*)&lastModFileTime, 2); // Last Mod File Time

	unsigned short lastModFileDate;
	_zipFs.read((char*)&lastModFileDate, 2); // Last Mod File Date

	centralFile->HeaderLastModified = (lastModFileDate << 16) | lastModFileTime;

	_zipFs.read((char*)&centralFile->CRC, 4); // CRC

	unsigned int tUInt;
	_zipFs.read((char*)&tUInt, 4); // Compressed Size
	centralFile->CompressedSize = tUInt;

	_zipFs.read((char*)&tUInt, 4); // Uncompressed Size
	centralFile->UncompressedSize = tUInt;

	_zipFs.read((char*)&centralFile->fileNameLength, 2); // File Name Length
	_zipFs.read((char*)&centralFile->extraFieldLength, 2); // Extra Field Length
	_zipFs.read((char*)&centralFile->fileCommentLength, 2); // File Comment Length

	unsigned short tUShort;
	_zipFs.read((char*)&tUShort, 2); // Disk Number Start
	_zipFs.read((char*)&tUShort, 2); // InternalFileAttributes
	_zipFs.read((char*)&tUInt, 4); // ExternalFileAttributes

	_zipFs.read((char*)&centralFile->RelativeOffsetOfLocalHeader, 4); // Relative Offset Of Local Header

	centralFile->bFileNameHeader=new char[centralFile->fileNameLength];
	_zipFs.read((char*)centralFile->bFileNameHeader, centralFile->fileNameLength); // File Name

	//if bit 11 of the general purpose bit flag is set, the filename is encoded using UTF-8
	//if bit 11 of the general purpose bit flag is not set, the filename is encoded using Encoding.GetEncoding(437);
	if ((centralFile->GeneralPurposeBitFlag & (1 << 11)))
	{
		centralFile->bFileName = new char[centralFile->fileNameLength + 1];
		memcpy(centralFile->bFileName,centralFile->bFileNameHeader,centralFile->fileNameLength);
		centralFile->bFileName[centralFile->fileNameLength] = 0;
	}
	else
	{
		CodePage437 cp437=CodePage437();
		centralFile->bFileName = cp437.Enc(centralFile->bFileNameHeader, centralFile->fileNameLength);
	}

	centralFile->bExtraField = new char[centralFile->extraFieldLength];
	_zipFs.read((char*)centralFile->bExtraField, centralFile->extraFieldLength); // Extra Field

	if (centralFile->extraFieldLength > 0)
		ZipExtraFieldRead::ReadExtraField(centralFile, true);

	centralFile->RelativeOffsetOfLocalHeader += offset;

	centralFile->bFileComment = new char[centralFile->fileCommentLength];
	_zipFs.read((char*)centralFile->bFileComment, centralFile->fileCommentLength); // File Comment

	return ZipGood;
}

ZipReturn Zip::LocalFileHeaderReader(unsigned long long relativeOffsetOfLocalHeader, unsigned long long compressedSize, ZipHeader* localFile)
{
	localFile->RelativeOffsetOfLocalHeader = relativeOffsetOfLocalHeader;
	_zipFs.seekg((long)relativeOffsetOfLocalHeader, ios::beg);
	unsigned int thisSignature;
	_zipFs.read((char*)&thisSignature, 4);
	if (thisSignature != LocalFileHeaderSignature)
		return ZipCentralDirError;

	_zipFs.read((char*)&localFile->VersionNeededToExtract, 2); // Version Needed To Extract

	_zipFs.read((char*)&localFile->GeneralPurposeBitFlag, 2); // General Purpose Bit Flag
	_zipFs.read((char*)&localFile->CompressionMethod, 2); // Compression Method

	unsigned short lastModFileTime;
	_zipFs.read((char*)&lastModFileTime, 2); // Last Mod File Time

	unsigned short lastModFileDate;
	_zipFs.read((char*)&lastModFileDate, 2); // Last Mod File Date

	localFile->HeaderLastModified = (lastModFileDate << 16) | lastModFileTime;

	_zipFs.read((char*)&localFile->CRC, 4); // CRC

	unsigned int tUInt;
	_zipFs.read((char*)&tUInt, 4); // Compressed Size
	localFile->CompressedSize = tUInt;

	_zipFs.read((char*)&tUInt, 4); // Uncompressed Size
	localFile->UncompressedSize = tUInt;

	_zipFs.read((char*)&localFile->fileNameLength, 2); // File Name Length
	_zipFs.read((char*)&localFile->extraFieldLength, 2); // Extra Field Length
	localFile->fileCommentLength = 0;

	localFile->bFileNameHeader = new char[localFile->fileNameLength];
	_zipFs.read((char*)localFile->bFileNameHeader, localFile->fileNameLength); // File Name

	//if bit 11 of the general purpose bit flag is set, the filename is encoded using UTF-8
	//if bit 11 of the general purpose bit flag is not set, the filename is encoded using Encoding.GetEncoding(437);
	if ((localFile->GeneralPurposeBitFlag & (1 << 11)))
	{
		localFile->bFileName = new char[localFile->fileNameLength + 1];
		memcpy(localFile->bFileName, localFile->bFileNameHeader, localFile->fileNameLength);
		localFile->bFileName[localFile->fileNameLength] = 0;
	}
	else
	{
		CodePage437 cp437 = CodePage437();
		localFile->bFileName = cp437.Enc(localFile->bFileNameHeader, localFile->fileNameLength);
	}



	localFile->bExtraField = new char[localFile->extraFieldLength];
	_zipFs.read((char*)localFile->bExtraField, localFile->extraFieldLength); // Extra Field

	if (localFile->extraFieldLength > 0)
		ZipExtraFieldRead::ReadExtraField(localFile, false);

	localFile->DataLocation = (unsigned long)_zipFs.tellg();

	if (localFile->GeneralPurposeBitFlag & 8)
	{
		_zipFs.seekg(compressedSize, ios::cur);
		_zipFs.read((char*)&localFile->CRC, 4); // CRC
		if (localFile->CRC == 0x08074b50)
		{
			_zipFs.read((char*)&localFile->CRC, 4); // CRC
		}

		if (localFile->IsZip64)
		{
			_zipFs.read((char*)&localFile->CompressedSize, 8); // Compressed Size
			_zipFs.read((char*)&localFile->UncompressedSize, 8); // Uncompressed Size
		}
		else
		{
			_zipFs.read((char*)&tUInt, 4); // Compressed Size
			localFile->CompressedSize = tUInt;

			_zipFs.read((char*)&tUInt, 4); // Uncompressed Size
			localFile->UncompressedSize = tUInt;
		}
	}
	return ZipGood;
}


