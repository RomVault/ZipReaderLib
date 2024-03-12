using System.Text;

namespace ZipReaderLib
{
    internal class Program
    {
        static void Main(string[] args)
        {
            Console.OutputEncoding = Encoding.UTF8;
            if (Directory.Exists(args[0]))
                OpenDir(args[0]);
            else
                OpenZip(args[0]);
        }


        static void OpenDir(string dir)
        {
            DirectoryInfo di = new DirectoryInfo(dir);
            FileInfo[] fi = di.GetFiles("*.zip");
            foreach (FileInfo f in fi)
            {
                OpenZip(f.FullName);
            }
            DirectoryInfo[] arrdi = di.GetDirectories();
            foreach (DirectoryInfo d in arrdi)
            {
                OpenDir(d.FullName);
            }
        }

        static void OpenZip(string filename)
        {

            Zip zip = new Zip();
            zip.ZipFileOpen(filename);
            Console.WriteLine($"File: {filename}");
            Console.WriteLine("------------+-------------+----------+------+-----------+---------------------+---------------------+-----------------------");
            Console.WriteLine("Comp-Size     UnComp-Size   CRC        VNTE   Comp        Header Date/Time      Extended Date/Time    Name");
            Console.WriteLine("------------+-------------+----------+------+-----------+---------------------+---------------------+-----------------------");

            for (int i = 0; i < zip.LocalFilesCount; i++)
            {
                ZipHeader lf = zip.GetLocalFile(i);
                string HeaderModTime = zipDateTimeToString(lf.HeaderLastModified);
                HeaderModTime = !string.IsNullOrWhiteSpace(HeaderModTime) ? HeaderModTime : "                   ";
                string ExtendedDataModTime = lf.ModifiedTime != null ? zipDateTimeToString((long)lf.ModifiedTime) : "                   ";

                Console.WriteLine($"{lf.CompressedSize,11}   {lf.UncompressedSize,11}   {BitConverter.ToUInt32(lf.CRC):X8}   {lf.VersionNeededToExtract,4}   {compMethod(lf.CompressionMethod)}   {HeaderModTime}   {ExtendedDataModTime}   {lf.Filename}");
            }
            Console.WriteLine("------------+-------------+----------+------+----------+---------------------+---------------------+-----------------------");
            for (int i = 0; i < zip.LocalFilesCount; i++)
            {
                ZipHeader lf = zip.GetCentralFile(i);
                string HeaderModTime = zipDateTimeToString(lf.HeaderLastModified);
                HeaderModTime = !string.IsNullOrWhiteSpace(HeaderModTime) ? HeaderModTime : "                   ";
                string ExtendedDataModTime = lf.ModifiedTime != null ? zipDateTimeToString((long)lf.ModifiedTime) : "                   ";

                Console.WriteLine($"{lf.CompressedSize,11}   {lf.UncompressedSize,11}   {BitConverter.ToUInt32(lf.CRC):X8}   {lf.VersionNeededToExtract,4}   {compMethod(lf.CompressionMethod)}   {HeaderModTime}   {ExtendedDataModTime}   {lf.Filename}");
            }
            Console.WriteLine("------------+-------------+----------+------+----------+---------------------+---------------------+-----------------------");

        }


        public static string zipDateTimeToString(long? zipFileDateTime)
        {
            if (zipFileDateTime == null || zipFileDateTime == 0 || zipFileDateTime == long.MinValue)
                return "";

            if (zipFileDateTime > 0xffffffff)
            {
                if (zipFileDateTime < DateTime.MinValue.Ticks || zipFileDateTime > DateTime.MaxValue.Ticks)
                    return "";

                var t = new DateTime((long)zipFileDateTime);

                return $"{t.Year:D4}/{t.Month:D2}/{t.Day:D2} {t.Hour:D2}:{t.Minute:D2}:{t.Second:D2}";
            }

            ushort dosFileDate = (ushort)((zipFileDateTime >> 16) & 0xffff);
            ushort dosFileTime = (ushort)(zipFileDateTime & 0xffff);

            int second = (dosFileTime & 0x1f) << 1;
            int minute = (dosFileTime >> 5) & 0x3f;
            int hour = (dosFileTime >> 11) & 0x1f;

            int day = dosFileDate & 0x1f;
            int month = (dosFileDate >> 5) & 0x0f;
            int year = ((dosFileDate >> 9) & 0x7f) + 1980;

            return $"{year:D4}/{month:D2}/{day:D2} {hour:D2}:{minute:D2}:{second:D2}";
        }

        private static string compMethod(int comp)
        {
            switch (comp)
            {
                case 0: return "Stored  ";
                case 1: return "Shrunk  ";
                case 5: return "CompF 4 ";
                case 6: return "Imploded";
                case 8: return "Deflated";
                default: return "Unknown";
            }
        }
    }
}
