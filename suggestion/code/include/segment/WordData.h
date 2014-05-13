#ifndef WORD_DATA_HEADER_FILE
#define WORD_DATA_HEADER_FILE

#include <map>
//#include <set>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <algorithm>
using namespace std;

typedef unsigned char U_CHAR;
typedef unsigned short U_SHORT;

#define VERSION_SIZE 64

#pragma pack(1)
struct WordData 
{
	U_CHAR nPOS;
	U_SHORT nFreq;
	int nTermID;
  int idf;
};
#pragma pack()

#pragma pack(1)
struct SubPhraseData 
{
	U_CHAR nPOS;
	U_CHAR nLen;
	U_SHORT nFreq;
	int nTermID;
};
#pragma pack()

bool operator <(const WordData& wd1, const WordData& wd2);
bool operator ==(const WordData& wd1, const WordData& wd2);

#endif
