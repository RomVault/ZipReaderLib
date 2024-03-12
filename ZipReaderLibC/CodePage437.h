#pragma once
class CodePage437
{
public:
	char* Enc(char* cIn, int len);
	int EncLen(char* cIn, int len);

private:
	int charLen(unsigned char c);
	void charEnc(unsigned char c, char* res, int &relLen);

};

