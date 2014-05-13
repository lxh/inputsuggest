// WordSplit.h: interface for the CWordSplit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORDSPLIT_H__28306FBD_CE99_4C52_8B81_8421CD66FB90__INCLUDED_)
#define AFX_WORDSPLIT_H__28306FBD_CE99_4C52_8B81_8421CD66FB90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "segment/DATrieDict.h"

#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <stack>
#include <ctime>
using namespace std;

#define SPLIT_VERSION "wordsplit 3.3.0"

#define RESULT_MEM_LEN (1024 * 1024)

//Block Number
#define BLOCK_NUM (1024 * 1024)

//ÿ���зֵĳ��ȣ���ֵ���õ�̫С���ٶȻ���Ӱ�졣
//#define MAX_LEN (10 * 1024)
//#define MAX_LEN (20)
//#define MAX_LEN (65555)
#define MAX_LEN (1024 * 1024)

#define MAX_LETTER_NUMBER_LEN 26 //��������ĸ����������������

// �洢�������ʱ���������洢λ�ã�[%d-%d]
// ��󳤶�Ϊ��3([-]) + sizeof(int) * 2 + 1('\0') = 68
#define POS_LEN 10
#define WORDLEN 68

// ���ڴ�Ź������ݵĽṹ��SctSharedData�г�Ա���ȵ����ơ��п��ܱ�MAX_LEN�󣬽��������ֶε��¡�
#define MEMLEN (MAX_LEN + MAX_LETTER_NUMBER_LEN)

#define PHRASE_LEN 40
#define P2Node_LEN_1 (PHRASE_LEN * 10)
#define P2Node_LEN_2 100

#pragma pack(1)
struct SctBasic 
{
	U_CHAR nLen; //��������
	U_CHAR nPOS; // ��ע
	size_t nID; //Word ID
};
#pragma pack()

#pragma pack(1)
struct SctPhrase 
{
	size_t nBeg;
	U_CHAR nLen; //��������
	U_CHAR nBasic; //�����ʵĸ���
	U_CHAR nPOS; // ��ע
	U_SHORT nID; //Word ID
	bool isMaxPhrase;
	SctPhrase() 
	{
		nBeg = 0;
		nID = 0;
		nLen = 0;
		nBasic = 0;
		nPOS = 19;
		isMaxPhrase = false;
	}
};
#pragma pack()

#pragma pack(1)
struct Sct_Node 
{
	size_t nBeg;
	U_CHAR nLen;
	U_CHAR nPOS; // ��ע
	U_SHORT nSlice;
	U_SHORT nFreq;
	size_t nTermID;
	int nPre1, nPre2;
};
#pragma pack()

#pragma pack(1)
struct SctSharedData 
{
	size_t nBlock; //һ���ֳ��˼���
	int nStart; // begin position
	int nID; // ÿ�η�һ��ʱ�������ʼ��
	int nStrLen; // ÿ�η�һ��ʱ�������ʼ��

	int nLookupPhrase;
	int nSubPhrase; //�Ӷ���ĸ���
	int nTermID; //Load sub phrase.
	char *pStr;
	char *pTemp;
	U_CHAR nPOS;
	size_t *szBlock; //��¼ÿһ��ĳ���
	int *szIntBasicEnd;
	int *szNumPhrase;
	SctBasic *szSctBasic;
	SubPhraseData *szLookupPhrase;
	SctPhrase *szBestPhrase;
	SctPhrase *szSubPhrase;
	Sct_Node **p2Node;
	int *pSize2;
	SctSharedData() 
	{
		pStr = new char[MEMLEN];
		pTemp = new char[WORDLEN];
		szBlock = new size_t[BLOCK_NUM];
		szIntBasicEnd = new int[MEMLEN];
		szNumPhrase = new int[MEMLEN];
		szSctBasic = new SctBasic[MEMLEN];
		szLookupPhrase = new SubPhraseData[64];
		szBestPhrase = new SctPhrase[MEMLEN];
		szSubPhrase = new SctPhrase[MEMLEN];
		p2Node = (Sct_Node **)malloc(P2Node_LEN_1 * sizeof(Sct_Node *));
		for (int i = 0; i < P2Node_LEN_1; i++) 
		{
			p2Node[i] = (Sct_Node *)malloc(P2Node_LEN_2 * sizeof(Sct_Node)); // the number of sub phrases at most
		}
		pSize2 = (int *)malloc(P2Node_LEN_1 * sizeof(int));
	}
	~SctSharedData() 
	{
		delete [] pStr;
		delete [] pTemp;
		delete [] szBlock;
		delete [] szIntBasicEnd;
		delete [] szNumPhrase;
		delete [] szSctBasic;
		delete [] szLookupPhrase;
		delete [] szBestPhrase;
		delete [] szSubPhrase;
		for ( int i = 0; i < P2Node_LEN_1; i++ ) 
		{
//			if ( p2Node[i] ) 
//			{
//				free( p2Node[i] );
//			}
			free( p2Node[i] );
		}
		free( p2Node );
		free( pSize2 );
	}
};
#pragma pack()

struct ptrCmp
{
    bool operator()( const char * s1, const char * s2 ) const
    {
        return strcmp( s1, s2 ) < 0;
    }
};

struct Sct_Version
{
	string strSplitProgram;
	string strDictPhrase;
	string strDictSubPhrase;
	string strDictNumber;
	string strDictUnit;
};

class CWordSplit
{
public:
	CWordSplit(const char * dictpath);
	virtual ~CWordSplit();
	struct Sct_Version getVersion();
	size_t funSplitFile(const char *filein, const char *fileout);
	void funSplitSpeedTest(const char *filein);
	void funSplitPressureTest(const char *filein, const char *fileout);
	void funSplitString(const char *pStr, char *pSplit, char *pBasic,
	char *pPhrase, bool bPosition, bool bTag,  bool bID = false, bool bIDF=false);
	void install();
	vector<int> getTermID(const char *pStr, int len);

private:
	// �����ʽṹ��
	#pragma pack(1)
	struct Sct_Basic 
	{
		U_CHAR nBeg; // ��ʼλ��
		U_CHAR nPOS; // ��ע
	};
	#pragma pack()

	// �Ӷ���ṹ��
	#pragma pack(1)
	struct Sct_Phrase 
	{
		U_CHAR nBeg; // ��ʼλ��
		U_CHAR nLen; // �����ʳ���
		U_CHAR nBasicId; // �����ʵ�id����0��ʼ���
		U_CHAR nPOS; // ��ע
		U_CHAR nBasic; // �����ʵĸ���
	};
	#pragma pack()

	#pragma pack(1)
	struct Sct_Phrase_Basic 
	{
		U_CHAR nBasic; // �����ʵĸ���
		U_CHAR nPhrase; // �Ӷ���ĸ���
		struct Sct_Basic *pSct_Basic;
		struct Sct_Phrase *pSct_Phrase;
	};
	#pragma pack()

#ifdef WIN32
	CRITICAL_SECTION cs;
#else
	pthread_mutex_t mutex;
#endif

	stack<SctSharedData *> sp;
	
	SctBasic *pSctBasicCmp;
	SctPhrase *pBestPhrase;

	static CDATrieDict cdict;
	static CDATrieDict cdictUnit;
	const char* direct;

	// Capital Number
	map<char *, int, ptrCmp> mapNumber;
	map<char *, int, ptrCmp>::iterator itrMapNumber;

	char szSubVer[VERSION_SIZE]; //�洢subphrase dict�汾��
	char szNumVer[VERSION_SIZE]; //�洢number dict�汾��

	// segmented phrase which composed of basic and sub phrase.
	int nTermTotal;
	Sct_Phrase_Basic **pSzPhraseBasic;

	bool funLoadDictNumber( const char * filename );
	bool funLoadPhraseDict( const char * filename );
	void funFreeSubphraseRes();
	void funConvertNum(char *pdata, int nlen);
	size_t funGetBlockLen(const char *pdata, SctSharedData * pSharedData);
	int funLocateLastCharacter(const char *pLine, int &nBeg, int nLen);
	int funGetCapitalNumFreq(const char *pLine, int &nBeg, int n);
	void funInsertNoDisaWord(SctSharedData * pSharedData, int &nBeg, int &n);
	void funGetMaxDisaLink(const char *pLine, int &nBeg, int &n);
	void funDisambiguation(const char *pLine, int &nBeg, int &n, int &nFreq,
		SctSharedData * pSharedData);
	void funPrintDisambiguation(const char *pLine, int nSize1, SctSharedData * pSharedData);
	void funSplit(const char *pLine, SctSharedData * pSharedData, int nBeg);
	void funLoadSubPhrase(int nBeg, int n, SctSharedData * pSharedData);
	void funConstructBasic(const char * pLine, char * pBasic,
									SctSharedData * pSharedData, size_t &nPos,
									size_t &nStart, size_t &nEnd,
									bool bPosition, bool bTag, bool bID, bool bIDF);
	void funConstructPhrase(const char * pStr, char * pPhrase,
		SctSharedData * pSharedData, size_t &nPosPhrase, bool bPosition, bool bTag, bool bID, bool bIDF);
	void funBestSplit(const char * pStr, char * pSplit,
		SctSharedData * pSharedData, size_t &nPosBest, bool bPosition, bool bTag, bool bID, bool bIDF);
	void funNumPhrase(const char *pLine, SctSharedData * pSharedData,
							  int nBegPre, int &nBeg, int &n);
	void funInsertSctBasic(int &nBeg, int &n, SctSharedData * pSharedData);
	bool funNumBasic(const char *pLine, int &nBeg, int &n, SctSharedData * pSharedData);
	void funGetCapitalNumber(const char * p, int &nBeg);
	bool funGetLetter(const char *p, int &nBeg);
	bool funGetArabicNumber(const char *p, int &nBeg);
};

#endif // !defined(AFX_WORDSPLIT_H__28306FBD_CE99_4C52_8B81_8421CD66FB90__INCLUDED_)
