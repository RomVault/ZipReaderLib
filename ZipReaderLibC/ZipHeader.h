#pragma once
#include <climits>


class ZipHeader
{
public:

	ZipHeader() {};

	//string Filename;
	unsigned long long int UncompressedSize;
	//unsigned char[] CRC;
	unsigned long long int CompressedSize;
	unsigned short CompressionMethod;

	unsigned int CRC;

	//DOS Datetime from main ZIP headers.
	long long int HeaderLastModified;

	//NTFS Datetime from extended Data.
	long long int ModifiedTime = LONG_MIN;
	long long int CreatedTime = LONG_MIN;
	long long int AccessedTime = LONG_MIN;

	// only in central directory
	unsigned long long int RelativeOffsetOfLocalHeader;

	unsigned short fileNameLength;
	unsigned short extraFieldLength;
	unsigned short fileCommentLength;

	char* bFileNameHeader;
	char* bFileName;
	char* bExtraField;
	char* bFileComment;

	unsigned long long int DataLocation;

	bool IsZip64;
	bool ExtraDataFound;

	unsigned short VersionMadeBy;
	unsigned short VersionNeededToExtract;
	unsigned short GeneralPurposeBitFlag;
};