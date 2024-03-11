#include "CodePage437.h"


const int charLen[] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,2,2,2,3,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	2,2,2,2,2,2,2,2,2,2,2,2,3,2,2,3,
	3,2,3,3,3,3,2,3,2,3,2,3,3,2,3,2 };

const int charEnc[] = {
0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
0xC387,0xC3BC,0xC3A9,0xC3A2,0xC3A4,0xC3A0,0xC3A5,0xC3A7,0xC3AA,0xC3AB,0xC3A8,0xC3AF,0xC3AE,0xC3AC,0xC384,0xC385,
0xC389,0xC3A6,0xC386,0xC3B4,0xC3B6,0xC3B2,0xC3BB,0xC3B9,0xC3BF,0xC396,0xC39C,0xC2A2,0xC2A3,0xC2A5,0xE282A7,0xC692,
0xC3A1,0xC3AD,0xC3B3,0xC3BA,0xC3B1,0xC391,0xC2AA,0xC2BA,0xC2BF,0xE28C90,0xC2AC,0xC2BD,0xC2BC,0xC2A1,0xC2AB,0xC2BB,
0xE29691,0xE29692,0xE29693,0xE29482,0xE294A4,0xE295A1,0xE295A2,0xE29596,0xE29595,0xE295A3,0xE29591,0xE29597,0xE2959D,0xE2959C,0xE2959B,0xE29490,
0xE29494,0xE294B4,0xE294AC,0xE2949C,0xE29480,0xE294BC,0xE2959E,0xE2959F,0xE2959A,0xE29594,0xE295A9,0xE295A6,0xE295A0,0xE29590,0xE295AC,0xE295A7,
0xE295A8,0xE295A4,0xE295A5,0xE29599,0xE29598,0xE29592,0xE29593,0xE295AB,0xE295AA,0xE29498,0xE2948C,0xE29688,0xE29684,0xE2968C,0xE29690,0xE29680,
0xCEB1,0xC39F,0xCE93,0xCF80,0xCEA3,0xCF83,0xC2B5,0xCF84,0xCEA6,0xCE98,0xCEA9,0xCEB4,0xE2889E,0xCF86,0xCEB5,0xE288A9,
0xE289A1,0xC2B1,0xE289A5,0xE289A4,0xE28CA0,0xE28CA1,0xC3B7,0xE28988,0xC2B0,0xE28899,0xC2B7,0xE2889A,0xE281BF,0xC2B2,0xE296A0,0xC2A0
};

char* CodePage437::Enc(char* cIn, int len)
{
	char* res = new char[EncLen(cIn, len)+1];
	int relLen = 0;
	for (int i = 0; i < len; i++)
	{
		int c = (unsigned char)cIn[i];
		int l = charLen[c];
		int e = charEnc[c];
		if (l == 1)
		{
			res[relLen++] = e & 0xff;
		}
		else if (l == 2)
		{
			res[relLen++] = (e >> 8) & 0xff;
			res[relLen++] = (e >> 0) & 0xff;
		}
		else if (l == 3)
		{
			res[relLen++] = (e >> 16) & 0xff;
			res[relLen++] = (e >> 8) & 0xff;
			res[relLen++] = (e >> 0) & 0xff;
		}
	}
	res[relLen++] = 0;
	return res;
}

int CodePage437::EncLen(char* cIn, int len)
{
	int resLen = 0;
	for (int i = 0; i < len; i++)
		resLen += charLen[(unsigned char)cIn[i]];
	return resLen;
}