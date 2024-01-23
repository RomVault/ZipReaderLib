#pragma once
#include <climits>


class ZipHeader
{
    //string Filename;
    unsigned long UncompressedSize;
    //unsigned char[] CRC;
    unsigned long CompressedSize;
    unsigned short CompressionMethod;



    //DOS Datetime from main ZIP headers.
    long HeaderLastModified;

    //NTFS Datetime from extended Data.
    long ModifiedTime = LONG_MIN;
    long CreatedTime = LONG_MIN;
    long AccessedTime = LONG_MIN;

        // only in central directory
    unsigned long RelativeOffsetOfLocalHeader;

    //unsigned char[] bFileName;
    //unsigned char[] bExtraField;
    unsigned long DataLocation;
    //unsigned char[] bFileComment;

    bool IsZip64;
    bool ExtraDataFound;

    unsigned short VersionMadeBy;
    unsigned short VersionNeededToExtract;
    unsigned short GeneralPurposeBitFlag;
};