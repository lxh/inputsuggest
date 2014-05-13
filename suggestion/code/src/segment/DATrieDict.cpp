// DATrieDict.cpp: implementation of the CDATrieDict class.
//
//////////////////////////////////////////////////////////////////////

#include "segment/DATrieDict.h"
#include <set>
#include <cassert>
#include <iostream>
#include <algorithm>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDATrieDict::CDATrieDict()
{
	m_nHandleSize=0;
	m_nDArraySize=0;
	m_pDArray = NULL;
	m_pwCharMap = NULL;
	m_pWordData = NULL;
}

CDATrieDict::~CDATrieDict()
{
	if( !m_pDArray)
		free(m_pDArray);
	if( !m_pwCharMap)
		free(m_pwCharMap);
	if( !m_pWordData)
		free(m_pWordData);
}

bool CDATrieDict::load(const char*fn)
{
	FILE *fp = fopen(fn, "rb");

	if( !fp)
		return false;


	//load version info
	memset( szVersion, 0, VERSION_SIZE*sizeof(char));
	fread(szVersion, sizeof(char), VERSION_SIZE, fp);

	//CharID Ϊshort����
	if( !m_pwCharMap ) //����65536���ռ䣬���ڴ洢
		m_pwCharMap = (CharID*)malloc(CHARSET_SIZE*sizeof(CharID));
	memset( m_pwCharMap, 0, CHARSET_SIZE*sizeof(CharID));
	//���ļ��ж�ȡ65536��short
	fread(m_pwCharMap, sizeof(CharID), CHARSET_SIZE, fp);

	if( m_pWordData)
		free(m_pWordData);
	//��һ������Ϊhangle�Ĵ�С
	fread(&m_nHandleSize, sizeof(int), 1, fp);
  m_pWordData = (WordData*)malloc(m_nHandleSize*sizeof(WordData));
	memset( m_pWordData, 0, m_nHandleSize*sizeof(WordData));
	//����Ϊm_nHandleSize��WordData�������ʵ�λ�ã�Ƶ�ʣ�nTermID��idf��
	fread(m_pWordData, sizeof(WordData), m_nHandleSize, fp);

	if( m_pDArray )
		free(m_pDArray);
	fread(&m_nDArraySize, sizeof(int), 1, fp);
	//��һ������Ϊ
	m_pDArray = (DANode*) malloc(m_nDArraySize*sizeof(DANode));
	memset(m_pDArray, 0, m_nDArraySize*sizeof(DANode));
	//��ȡm_nDArraySize��DANode��base, check, handle��
	fread(m_pDArray, sizeof(DANode), m_nDArraySize, fp);
	fclose(fp);

	return true;
}

int CDATrieDict::lookup(const char* p, const WordData*&pwdBeg, size_t &nwdLen)
{
	int len=0, base, check, state=0;
	WORD w;
	CharID cid;

	const char *q=p;
	base=m_pDArray[state].base;

	nwdLen=0;
	pwdBeg=NULL;

	while( *q)
	{
		if( *q&0x80) //����
		{
			w=*(WORD*)q; //���������
			q+=2;
		} //�Ǻ���
		else
		{
			w=*q;
			++q;
		}
		
		cid = m_pwCharMap[w]; //ָ����cid��ͷ�ĵط�
		if( !cid || base+cid >= (int)m_nDArraySize) //���cid����ط�Ϊ�գ�����Խ��
			break;

		check=m_pDArray[ base+cid ].check;
		if( check != state )
			break;
		
		state = base+cid;
		base= m_pDArray[ state ].base;
		if( base & 0x80000000 )
		{ //ƥ������
			len = q-p;
			base&=0x7FFFFFFF;
			
			pwdBeg = m_pWordData+ (m_pDArray[state].handle >> 4);
			nwdLen = m_pDArray[state].handle & 0x3F;
		}
	}

	return len;
}


int CDATrieDict::lookup(const char* p, int nEnd, const WordData*&pwdBeg, size_t &nwdLen)
{
	int len=0, base, check, state=0;
	WORD w;
	CharID cid;

	const char *q=p;
	base=m_pDArray[state].base;

	nwdLen=0;
	pwdBeg=NULL;

	while( *q)
	{
		if( *q&0x80)
		{
			w=*(WORD*)q;
			q+=2;
		}
		else
		{
			w=*q;
			++q;
		}
		
		cid = m_pwCharMap[w];
		if( !cid || base+cid >= (int)m_nDArraySize)
			break;

		check=m_pDArray[ base+cid ].check;
		if( check != state )
			break;
		
		state = base+cid;
		base= m_pDArray[ state ].base;
		if( base & 0x80000000 )
		{
			if ( (q - p) > nEnd ) {
				break;
			}

			len = q-p;
			base&=0x7FFFFFFF;
			
			pwdBeg = m_pWordData+ (m_pDArray[state].handle >> 4);
			nwdLen = m_pDArray[state].handle & 0x3F;
		}
	}

	return len;
}

int CDATrieDict::lookupBest(const char* p, const WordData*&pwdBeg, size_t &nwdLen)
{
	int len=0, base, check, state=0, nFreq=-1;
	WORD w;
	CharID cid;
	const WordData* pwdBegKeep=NULL;

	const char *q=p;
	base=m_pDArray[state].base;

	nwdLen=0;
	pwdBeg=NULL;

	while( *q)
	{
		if( *q&0x80)
		{
			w=*(WORD*)q;
			q+=2;
		}
		else
		{
			w=*q;
			++q;
		}
		
		cid = m_pwCharMap[w];
		if( !cid || base+cid >= (int)m_nDArraySize)
			break;

		check=m_pDArray[ base+cid ].check;
		if( check != state )
			break;
		
		state = base+cid;
		base= m_pDArray[ state ].base;
		if( base & 0x80000000 )
		{			
			base&=0x7FFFFFFF;
			
			pwdBeg = m_pWordData+ (m_pDArray[state].handle >> 4);
			nwdLen = m_pDArray[state].handle & 0x3F;

			if ( nFreq < pwdBeg->nFreq ) 
			{
				nFreq = pwdBeg->nFreq;
				pwdBegKeep = pwdBeg;
				len = q-p;
			}
			else if ( len == 2 ) 
			{
				nFreq = pwdBeg->nFreq;
				pwdBegKeep = pwdBeg;
				len = q-p;
			}
		}
	}

	pwdBeg = pwdBegKeep;
	return len;
}


int CDATrieDict::lookup_phrase(const char* p, SubPhraseData *szLookupPhrase, int &nLookupPharse)
{
	const WordData* pwdBeg;
	size_t nwdLen;
	int len=0, base, check, state=0;
	WORD w;
	CharID cid;

	const char *q=p;
	nLookupPharse = 0;

	base=m_pDArray[state].base;

	nwdLen=0;
	pwdBeg=NULL;

	while( *q)
	{
		if( *q&0x80)
		{
			w=*(WORD*)q;
			q+=2;
		}
		else
		{
			w=*q;
			++q;
		}
		
		cid = m_pwCharMap[w];
		if( !cid || base+cid >= (int)m_nDArraySize)
			break;

		check=m_pDArray[ base+cid ].check;
		if( check != state )
			break;
		
		state = base+cid;
		base= m_pDArray[ state ].base;
		if( base & 0x80000000 )
		{
			base&=0x7FFFFFFF;
			len = q-p;
			
			pwdBeg = m_pWordData+ (m_pDArray[state].handle >> 4);
			nwdLen = m_pDArray[state].handle & 0x3F;

			SubPhraseData sd;			
			sd.nFreq = pwdBeg->nFreq;
			sd.nLen = len;
			sd.nPOS = pwdBeg->nPOS;
			sd.nTermID = pwdBeg->nTermID;
			szLookupPhrase[nLookupPharse++] = sd;
		}
	}

	return len;
}

int CDATrieDict::lookup_phrase(const char* p, int nEnd, SubPhraseData *szLookupPhrase, int &nLookupPharse)
{
	const WordData* pwdBeg;
	size_t nwdLen;
	int len=0, base, check, state=0;
	WORD w;
	CharID cid;

	const char *q=p;
	nLookupPharse = 0;

	base=m_pDArray[state].base;

	nwdLen=0;
	pwdBeg=NULL;

	while( *q)
	{
		if( *q&0x80)
		{
			w=*(WORD*)q;
			q+=2;
		}
		else
		{
			w=*q;
			++q;
		}
		
		cid = m_pwCharMap[w];
		if( !cid || base+cid >= (int)m_nDArraySize)
			break;

		check=m_pDArray[ base+cid ].check;
		if( check != state )
			break;
		
		state = base+cid;
		base= m_pDArray[ state ].base;
		if( base & 0x80000000 )
		{
			base&=0x7FFFFFFF;
			if ( (q - p) > nEnd ) 
			{
				break;
			}
			len = q-p;
			
			pwdBeg = m_pWordData+ (m_pDArray[state].handle >> 4);
			nwdLen = m_pDArray[state].handle & 0x3F;

			SubPhraseData sd;
			sd.nFreq = pwdBeg->nFreq;
			sd.nLen = len;
			sd.nPOS = pwdBeg->nPOS;
			sd.nTermID = pwdBeg->nTermID;
			szLookupPhrase[nLookupPharse++] = sd;
		}
	}

	return len;
}

int CDATrieDict::lookup_phrase_min(const char* p, int nEnd, SubPhraseData *szLookupPhrase, int &nLookupPharse)
{
	const WordData* pwdBeg;
	size_t nwdLen;
	int len=0, base, check, state=0;
	int nReturn = 0;
	WORD w;
	CharID cid;

	const char *q=p;
	nLookupPharse = 0;

	base=m_pDArray[state].base;

	nwdLen=0;
	pwdBeg=NULL;

	while( *q)
	{
		if( *q&0x80)
		{
			w=*(WORD*)q;
			q+=2;
		}
		else
		{
			w=*q;
			++q;
		}
		
		cid = m_pwCharMap[w];
		if( !cid || base+cid >= (int)m_nDArraySize)
			break;

		check=m_pDArray[ base+cid ].check;
		if( check != state )
			break;
		
		state = base+cid;
		base= m_pDArray[ state ].base;
		if( base & 0x80000000 )
		{
			base&=0x7FFFFFFF;
			if ( (q - p) > nEnd ) 
			{
				break;
			}
			len = q-p;

			if ( 0 == nReturn ) 
			{
				nReturn = len;
			}			
			
			pwdBeg = m_pWordData+ (m_pDArray[state].handle >> 4);
			nwdLen = m_pDArray[state].handle & 0x3F;

			SubPhraseData sd;
			sd.nFreq = pwdBeg->nFreq;
			sd.nLen = len;
			sd.nPOS = pwdBeg->nPOS;
			sd.nTermID = pwdBeg->nTermID;
			szLookupPhrase[nLookupPharse++] = sd;
		}
		else if( (q - p) < 3 ) 
		{ //Single word(Out Of Vocabulary)
			SubPhraseData sd;
			sd.nFreq = 0;
			sd.nLen = q - p;
			sd.nPOS = 19; //Other
			sd.nTermID = 0;
			szLookupPhrase[nLookupPharse++] = sd;
			nReturn = q - p;
		}
	}

	return nReturn;
}
