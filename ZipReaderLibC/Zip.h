#pragma once

#include <iostream>
#include <fstream>
using namespace std;

class Zip
{
public:
	int openZipFile(const char* zipFilePath);

private:
	int FindEndOfCentralDirSignature(fstream& zipFs);
	int EndOfCentralDirRead(fstream& zipFs);

};

