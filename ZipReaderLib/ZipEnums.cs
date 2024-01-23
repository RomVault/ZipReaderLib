namespace ZipReaderLib
{
    public enum ZipReturn
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
    }
}
