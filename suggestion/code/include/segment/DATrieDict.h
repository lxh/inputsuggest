// DATrieDict.h: interface for the CDATrieDict class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATRIEDICT_H__2EB49B28_BE09_4D8C_A80A_B063A8C9D519__INCLUDED_)
#define AFX_DATRIEDICT_H__2EB49B28_BE09_4D8C_A80A_B063A8C9D519__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//#pragma warning (disable:4786)

#include <string.h>
#include "segment/WordData.h"

#define CHARSET_SIZE 65536

typedef unsigned short WORD;
typedef unsigned short CharID;

class CDATrieDict  
{
public:

struct DANode 
{
	int base, check, handle;
};

	char szVersion[VERSION_SIZE]; //´æ´¢°æ±¾ºÅ
	int lookup(const char* p, const WordData*&pwdBeg, size_t &nwdLen);
	int lookup(const char* p, int nEnd, const WordData*&pwdBeg, size_t &nwdLen);
	int lookupBest(const char* p, const WordData*&pwdBeg, size_t &nwdLen);
	int lookup_phrase(const char* p, SubPhraseData *szLookupPhrase, int &nLookupPharse);
	int lookup_phrase(const char* p, int nEnd, SubPhraseData *szLookupPhrase, int &nLookupPharse);
	int lookup_phrase_min(const char* p, int nEnd, SubPhraseData *szLookupPhrase, int &nLookupPharse);

	bool load(const char*fn);
	CDATrieDict();
	virtual ~CDATrieDict();

private:
	CharID* m_pwCharMap;
	DANode* m_pDArray;
	WordData* m_pWordData;
	int m_nDArraySize, m_nHandleSize;

};

#endif // !defined(AFX_DATRIEDICT_H__2EB49B28_BE09_4D8C_A80A_B063A8C9D519__INCLUDED_)
