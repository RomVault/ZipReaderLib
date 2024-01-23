using System.Text;

namespace ZipReaderLib
{
    public class ZipHeader
    {
        public string Filename { get; internal set; }
        public ulong UncompressedSize { get; internal set; }
        public byte[] CRC { get; internal set; }
        public ulong CompressedSize { get; internal set; }
        public ushort CompressionMethod { get; internal set; }



        //DOS Datetime from main ZIP headers.
        public long HeaderLastModified { get; internal set; }

        //NTFS Datetime from extended Data.
        public long? ModifiedTime { get; internal set; }
        public long? CreatedTime { get; internal set; }
        public long? AccessedTime { get; internal set; }

        // only in central directory
        internal ulong RelativeOffsetOfLocalHeader;

        internal byte[] bFileName;
        internal byte[] bExtraField;
        internal ulong DataLocation;
        internal byte[] bFileComment;

        public bool IsZip64 { get; internal set; }
        public bool ExtraDataFound { get; internal set; }

        public ushort VersionMadeBy { get; internal set; }  
        public ushort VersionNeededToExtract { get; internal set; }
        internal ushort GeneralPurposeBitFlag { get; set; }

    }

}
