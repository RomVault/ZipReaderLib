#pragma once
#include <climits>


class ZipHeader
{
public:

	ZipHeader() {};

	//string Filename;
	unsigned long long UncompressedSize;
	//unsigned char[] CRC;
	unsigned long long CompressedSize;
	unsigned short CompressionMethod;

	unsigned int CRC;

	//DOS Datetime from main ZIP headers.
	long long HeaderLastModified;

	//NTFS Datetime from extended Data.
	long long ModifiedTime = LONG_MIN;
	long long CreatedTime = LONG_MIN;
	long long AccessedTime = LONG_MIN;

	// only in central directory
	unsigned long long RelativeOffsetOfLocalHeader;

	unsigned short fileNameLength;
	unsigned short extraFieldLength;
	unsigned short fileCommentLength;

	char* bFileNameHeader;
	char* bFileName;
	char* bExtraField;
	char* bFileComment;

	unsigned long long DataLocation;

	bool IsZip64;
	bool ExtraDataFound;

	unsigned short VersionMadeBy;
	unsigned short VersionNeededToExtract;
	unsigned short GeneralPurposeBitFlag;
};