using System.Text;

namespace ZipReaderLib
{
    public partial class Zip
    {
        private uint _localFilesCount;
        private ZipHeader[] _centralDirectoryHeaders;
        private ZipHeader[] _localFileHeaders;
        public int LocalFilesCount => _localFileHeaders.Length;

        public ZipHeader GetCentralFile(int i)
        {
            return _centralDirectoryHeaders[i];
        }
        public ZipHeader GetLocalFile(int i)
        {
            return _localFileHeaders[i];
        }



        private byte[] _fileComment;
        public string FileComment => CompressUtils.GetString(_fileComment);



        private Stream _zipFs;
        internal ulong _centralDirStart;
        internal ulong _centralDirSize;
        private ulong _endOfCentralDir64;

        private bool _zip64;

        internal ulong offset = 0;
        internal bool ExtraDataFoundOnEndOfFile = false;


        public Zip()
        {
            CompressUtils.EncodeSetup();
        }

        #region OpenClose

        public ZipReturn ZipFileOpen(string newFilename)
        {
            ZipFileClose();
            Stream inStream;
            try
            {
                if (!File.Exists(newFilename))
                {
                    ZipFileClose();
                    return ZipReturn.ZipErrorFileNotFound;
                }

                inStream = File.OpenRead(newFilename);
            }
            catch (PathTooLongException)
            {
                ZipFileClose();
                return ZipReturn.ZipFileNameToLong;
            }
            catch (Exception)
            {
                ZipFileClose();
                return ZipReturn.ZipErrorOpeningFile;
            }

            return ZipFileOpen(inStream);
        }


        public ZipReturn ZipFileOpen(Stream inStream)
        {
            ZipFileClose();
            _zip64 = false;
            _centralDirStart = 0;
            _centralDirSize = 0;
            _zipFs = inStream;

            ZipReturn retV = ZipFileReadHeaders();
            ZipFileClose();
            return retV;
        }


        public void ZipFileClose()
        {
            if (_zipFs != null)
            {
                _zipFs.Close();
                _zipFs.Dispose();
            }
        }
        #endregion


        #region MainBodyReader

        private ZipReturn ZipFileReadHeaders()
        {
            try
            {
                ZipReturn zRet = FindEndOfCentralDirSignature();
                if (zRet != ZipReturn.ZipGood)
                {
                    return zRet;
                }

                ulong endOfCentralDir = (ulong)_zipFs.Position;
                zRet = EndOfCentralDirRead();
                if (zRet != ZipReturn.ZipGood)
                {
                    return zRet;
                }

                // check if ZIP64 header is required
                bool zip64Required = (_centralDirStart == 0xffffffff || _centralDirSize == 0xffffffff || _localFilesCount == 0xffff);

                // check for a ZIP64 header
                _zipFs.Position = (long)endOfCentralDir - 20;
                zRet = Zip64EndOfCentralDirectoryLocatorRead();
                if (zRet == ZipReturn.ZipGood)
                {
                    _zipFs.Position = (long)_endOfCentralDir64;
                    zRet = Zip64EndOfCentralDirRead();
                    if (zRet == ZipReturn.ZipGood)
                    {
                        _zip64 = true;
                        endOfCentralDir = _endOfCentralDir64;
                    }
                }

                if (zip64Required && !_zip64)
                    return ZipReturn.Zip64EndOfCentralDirError;

                offset = (endOfCentralDir - _centralDirSize) - _centralDirStart;

                _centralDirStart += offset;

                // now read the central directory
                _zipFs.Position = (long)_centralDirStart;

                _centralDirectoryHeaders = new ZipHeader[_localFilesCount];
                for (int i = 0; i < _localFilesCount; i++)
                {
                    zRet = CentralDirectoryHeaderRead(offset, out ZipHeader CentralFile);
                    if (zRet != ZipReturn.ZipGood)
                        return zRet;

                    _centralDirectoryHeaders[i] = CentralFile;
                }

                _localFileHeaders = new ZipHeader[_localFilesCount];
                for (int i = 0; i < _localFilesCount; i++)
                {
                    zRet = LocalFileHeaderRead(_centralDirectoryHeaders[i].RelativeOffsetOfLocalHeader, _centralDirectoryHeaders[i].CompressedSize, out ZipHeader LocalFile);
                    if (zRet != ZipReturn.ZipGood)
                        return zRet;

                    _localFileHeaders[i] = LocalFile;
                }

                return ZipReturn.ZipGood;
            }
            catch
            {
                return ZipReturn.ZipErrorReadingFile;
            }
        }
        #endregion


        #region EndOfCentralDirReaders
        private const uint EndOfCentralDirSignature = 0x06054b50;
        private const uint Zip64EndOfCentralDirSignature = 0x06064b50;
        private const uint Zip64EndOfCentralDirectoryLocator = 0x07064b50;

        private ZipReturn FindEndOfCentralDirSignature()
        {
            long fileSize = _zipFs.Length;
            long maxBackSearch = 0xffff;

            if (_zipFs.Length < maxBackSearch)
            {
                maxBackSearch = fileSize;
            }

            const long buffSize = 0x400;

            byte[] buffer = new byte[buffSize + 4];

            long backPosition = 4;
            while (backPosition < maxBackSearch)
            {
                backPosition += buffSize;
                if (backPosition > maxBackSearch)
                {
                    backPosition = maxBackSearch;
                }

                long readSize = backPosition > buffSize + 4 ? buffSize + 4 : backPosition;

                _zipFs.Position = fileSize - backPosition;

                _zipFs.Read(buffer, 0, (int)readSize);


                for (long i = readSize - 4; i >= 0; i--)
                {
                    if (buffer[i] != 0x50 || buffer[i + 1] != 0x4b || buffer[i + 2] != 0x05 || buffer[i + 3] != 0x06)
                    {
                        continue;
                    }

                    _zipFs.Position = fileSize - backPosition + i;
                    return ZipReturn.ZipGood;
                }
            }
            return ZipReturn.ZipCentralDirError;
        }


        private ZipReturn EndOfCentralDirRead()
        {
            using BinaryReader zipBr = new(_zipFs, Encoding.UTF8, true);
            uint thisSignature = zipBr.ReadUInt32();
            if (thisSignature != EndOfCentralDirSignature)
            {
                return ZipReturn.ZipEndOfCentralDirectoryError;
            }

            ushort tUShort = zipBr.ReadUInt16(); // NumberOfThisDisk
            if (tUShort != 0)
            {
                return ZipReturn.ZipEndOfCentralDirectoryError;
            }

            tUShort = zipBr.ReadUInt16(); // NumberOfThisDiskCenterDir
            if (tUShort != 0)
            {
                return ZipReturn.ZipEndOfCentralDirectoryError;
            }

            _localFilesCount = zipBr.ReadUInt16(); // TotalNumberOfEntriesDisk

            tUShort = zipBr.ReadUInt16(); // TotalNumber of entries in the central directory 
            if (tUShort != _localFilesCount)
            {
                return ZipReturn.ZipEndOfCentralDirectoryError;
            }

            _centralDirSize = zipBr.ReadUInt32(); // SizeOfCentralDir
            _centralDirStart = zipBr.ReadUInt32(); // Offset

            ushort zipFileCommentLength = zipBr.ReadUInt16();

            _fileComment = zipBr.ReadBytes(zipFileCommentLength);

            ExtraDataFoundOnEndOfFile = _zipFs.Position != _zipFs.Length;

            return ZipReturn.ZipGood;
        }


        private ZipReturn Zip64EndOfCentralDirRead()
        {
            using BinaryReader zipBr = new(_zipFs, Encoding.UTF8, true);
            uint thisSignature = zipBr.ReadUInt32();
            if (thisSignature != Zip64EndOfCentralDirSignature)
            {
                return ZipReturn.ZipEndOfCentralDirectoryError;
            }

            ulong tULong = zipBr.ReadUInt64(); // Size of zip64 end of central directory record
            if (tULong != 44)
            {
                return ZipReturn.Zip64EndOfCentralDirError;
            }

            zipBr.ReadUInt16(); // version made by

            ushort tUShort = zipBr.ReadUInt16(); // version needed to extract
            if (tUShort != 45)
            {
                return ZipReturn.Zip64EndOfCentralDirError;
            }

            uint tUInt = zipBr.ReadUInt32(); // number of this disk
            if (tUInt != 0)
            {
                return ZipReturn.Zip64EndOfCentralDirError;
            }

            tUInt = zipBr.ReadUInt32(); // number of the disk with the start of the central directory
            if (tUInt != 0)
            {
                return ZipReturn.Zip64EndOfCentralDirError;
            }

            _localFilesCount = (uint)zipBr.ReadUInt64(); // total number of entries in the central directory on this disk

            tULong = zipBr.ReadUInt64(); // total number of entries in the central directory
            if (tULong != _localFilesCount)
            {
                return ZipReturn.Zip64EndOfCentralDirError;
            }

            _zip64 = true;
            _centralDirSize = zipBr.ReadUInt64(); // size of central directory

            _centralDirStart = zipBr.ReadUInt64(); // offset of start of central directory with respect to the starting disk number

            return ZipReturn.ZipGood;
        }


        private ZipReturn Zip64EndOfCentralDirectoryLocatorRead()
        {
            using BinaryReader zipBr = new(_zipFs, Encoding.UTF8, true);
            uint thisSignature = zipBr.ReadUInt32();
            if (thisSignature != Zip64EndOfCentralDirectoryLocator)
            {
                return ZipReturn.ZipEndOfCentralDirectoryError;
            }

            uint tUInt = zipBr.ReadUInt32(); // number of the disk with the start of the zip64 end of central directory
            if (tUInt != 0)
            {
                return ZipReturn.Zip64EndOfCentralDirectoryLocatorError;
            }

            _endOfCentralDir64 = zipBr.ReadUInt64(); // relative offset of the zip64 end of central directory record

            tUInt = zipBr.ReadUInt32(); // total number of disks
            if (tUInt > 1)
            {
                return ZipReturn.Zip64EndOfCentralDirectoryLocatorError;
            }

            return ZipReturn.ZipGood;
        }


        #endregion


        #region HeaderReaders
        private const uint LocalFileHeaderSignature = 0x04034b50;
        private const uint CentralDirectoryHeaderSignature = 0x02014b50;

        internal ZipReturn CentralDirectoryHeaderRead(ulong offset, out ZipHeader centralFile)
        {
            try
            {
                centralFile = new ZipHeader();
                using BinaryReader br = new(_zipFs, Encoding.UTF8, true);
                uint thisSignature = br.ReadUInt32();
                if (thisSignature != CentralDirectoryHeaderSignature)
                    return ZipReturn.ZipCentralDirError;

                centralFile.VersionMadeBy = br.ReadUInt16(); // Version Made By
                centralFile.VersionNeededToExtract = br.ReadUInt16(); // Version Needed To Extract

                centralFile.GeneralPurposeBitFlag = br.ReadUInt16();
                centralFile.CompressionMethod = br.ReadUInt16();

                ushort lastModFileTime = br.ReadUInt16();
                ushort lastModFileDate = br.ReadUInt16();

                centralFile.HeaderLastModified = CompressUtils.CombineDosDateTime(lastModFileDate, lastModFileTime);

                centralFile.CRC = ReadCRC(br);

                centralFile.CompressedSize = br.ReadUInt32();
                centralFile.UncompressedSize = br.ReadUInt32();

                ushort fileNameLength = br.ReadUInt16();
                ushort extraFieldLength = br.ReadUInt16();
                ushort fileCommentLength = br.ReadUInt16();

                br.ReadUInt16(); // diskNumberStart
                br.ReadUInt16(); // internalFileAttributes
                br.ReadUInt32(); // externalFileAttributes

                centralFile.RelativeOffsetOfLocalHeader = br.ReadUInt32();


                centralFile.bFileName = br.ReadBytes(fileNameLength);
                centralFile.Filename = (centralFile.GeneralPurposeBitFlag & (1 << 11)) == 0
                    ? CompressUtils.GetString(centralFile.bFileName)
                    : Encoding.UTF8.GetString(centralFile.bFileName, 0, fileNameLength);

                centralFile.IsZip64 = false;
                centralFile.ExtraDataFound = false;
                if (extraFieldLength > 0)
                {
                    centralFile.bExtraField = br.ReadBytes(extraFieldLength);
                    ZipReturn zr = ZipExtraFieldRead.ExtraFieldRead(centralFile, true);
                    if (zr != ZipReturn.ZipGood)
                        return zr;
                }

                centralFile.RelativeOffsetOfLocalHeader += offset;

                if (fileCommentLength > 0)
                {
                    centralFile.bFileComment = br.ReadBytes(fileCommentLength);
                }

                return ZipReturn.ZipGood;
            }
            catch
            {
                centralFile = null;
                return ZipReturn.ZipCentralDirError;
            }
        }

        internal ZipReturn LocalFileHeaderRead(ulong RelativeOffsetOfLocalHeader, ulong CompressedSize, out ZipHeader localFile)
        {
            try
            {
                localFile = new ZipHeader();
                using (BinaryReader br = new(_zipFs, Encoding.UTF8, true))
                {
                    localFile.RelativeOffsetOfLocalHeader = RelativeOffsetOfLocalHeader;
                    zipFs.Position = (long)RelativeOffsetOfLocalHeader;
                    uint thisSignature = br.ReadUInt32();
                    if (thisSignature != LocalFileHeaderSignature)
                        return ZipReturn.ZipLocalFileHeaderError;

                    localFile.VersionNeededToExtract = br.ReadUInt16(); // version needed to extract
                    localFile.GeneralPurposeBitFlag = br.ReadUInt16();

                    localFile.CompressionMethod = br.ReadUInt16();

                    ushort lastModFileTime = br.ReadUInt16();
                    ushort lastModFileDate = br.ReadUInt16();

                    localFile.HeaderLastModified = CompressUtils.CombineDosDateTime(lastModFileDate, lastModFileTime);

                    localFile.CRC = ReadCRC(br);
                    localFile.CompressedSize = br.ReadUInt32();
                    localFile.UncompressedSize = br.ReadUInt32();
                    ulong localRelativeOffset = 0;

                    ushort fileNameLength = br.ReadUInt16();
                    ushort extraFieldLength = br.ReadUInt16();


                    localFile.bFileName = br.ReadBytes(fileNameLength);
                    localFile.Filename = (localFile.GeneralPurposeBitFlag & (1 << 11)) == 0
                        ? CompressUtils.GetString(localFile.bFileName)
                        : Encoding.UTF8.GetString(localFile.bFileName, 0, fileNameLength);

                    localFile.IsZip64 = false;
                    localFile.ExtraDataFound = false;
                    if (extraFieldLength > 0)
                    {
                        localFile.bExtraField = br.ReadBytes(extraFieldLength);

                        ZipReturn zr = ZipExtraFieldRead.ExtraFieldRead(localFile, false);
                        if (zr != ZipReturn.ZipGood)
                            return zr;
                    }


                    localFile.DataLocation = (ulong)zipFs.Position;

                    if ((localFile.GeneralPurposeBitFlag & 8) == 8)
                    {
                        zipFs.Position += (long)CompressedSize;

                        localFile.CRC = ReadCRC(br);
                        if (CompressUtils.ByteArrCompare(localFile.CRC, new byte[] { 0x08, 0x07, 0x4b, 0x50 }))
                        {
                            localFile.CRC = ReadCRC(br);
                        }

                        if (localFile.IsZip64)
                        {
                            localFile.CompressedSize = br.ReadUInt64();
                            localFile.UncompressedSize = br.ReadUInt64();
                        }
                        else
                        {
                            localFile.CompressedSize = br.ReadUInt32();
                            localFile.UncompressedSize = br.ReadUInt32();
                        }
                    }
                    return ZipReturn.ZipGood;
                }
            }
            catch
            {
                localFile = null;
                return ZipReturn.ZipLocalFileHeaderError;
            }
        }

        private static byte[] ReadCRC(BinaryReader br)
        {
            byte[] tCRC = new byte[4];
            tCRC[3] = br.ReadByte();
            tCRC[2] = br.ReadByte();
            tCRC[1] = br.ReadByte();
            tCRC[0] = br.ReadByte();
            return tCRC;
        }


        #endregion
    }
}
