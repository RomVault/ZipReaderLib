using System.Text;

namespace ZipReaderLib
{
    public static class CompressUtils
    {


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