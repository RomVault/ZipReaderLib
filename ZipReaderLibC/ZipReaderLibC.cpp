#include <iostream>
#include "Zip.h"

int main()
{
	std::cout << "Hello World!\n";

	Zip zip = Zip();
	ZipReturn zr = zip.ZipFileOpen("D:\\TDC\\DateProblems\\Indianapolis 500- The Simulation (1993)(Electronic Arts, Inc.) [Racing - Driving, Simulation, Sports][!] - Copy.zip");
}
