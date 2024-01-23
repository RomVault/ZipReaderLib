using System.Text;

namespace ZipReaderLib
{
    public static class CompressUtils
    {

        // according to the zip documents, zip filenames are stored as MS-DOS Code Page 437.
        // (Unless the unicode flag is set, in which case they are stored as UTF-8.
        private static Encoding enc = null;

        public static void EncodeSetup()
        {
            if (enc != null)
                return;
            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
            enc = Encoding.GetEncoding(437);
        }

        public static string GetString(byte[] byteArr)
        {
            return byteArr == null ? null : enc.GetString(byteArr);
        }

        internal static bool ByteArrCompare(byte[] b0, byte[] b1)
        {
            if ((b0 == null) || (b1 == null))
            {
                return false;
            }
            if (b0.Length != b1.Length)
            {
                return false;
            }

            for (int i = 0; i < b0.Length; i++)
            {
                if (b0[i] != b1[i])
                {
                    return false;
                }
            }
            return true;
        }


        private const long FileTimeToUtcTime = 504911232000000000;
        private const long EpochTimeToUtcTime = 621355968000000000;

        private const long TicksPerMillisecond = 10000;
        private const long TicksPerSecond = TicksPerMillisecond * 1000;


        internal static long CombineDosDateTime(ushort dosFileDate, ushort dosFileTime)
        {
            return (dosFileDate << 16) | dosFileTime;
        }

        public static long UtcTicksFromNtfsDateTime(long ntfsTicks)
        {
            return ntfsTicks + FileTimeToUtcTime;
        }


        public static long UtcTicksFromUnixDateTime(int linuxSeconds)
        {
            return linuxSeconds * TicksPerSecond + EpochTimeToUtcTime;
        }

    }
}