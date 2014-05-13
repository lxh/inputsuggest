#include "segment/WordData.h"

bool operator <(const WordData& wd1, const WordData& wd2)
{
	return wd1.nFreq < wd2.nFreq;
}

bool operator ==(const WordData& wd1, const WordData& wd2)
{
	return wd1.nFreq == wd2.nFreq;
}
