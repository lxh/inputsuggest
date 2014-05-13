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

//每次切分的长度，该值设置得太小，速度会受影响。
//#define MAX_LEN (10 * 1024)
//#define MAX_LEN (20)
//#define MAX_LEN (65555)
#define MAX_LEN (1024 * 1024)

#define MAX_LETTER_NUMBER_LEN 26 //连续的字母、数字最大个数限制

// 存储短语的临时变量，仅存储位置，[%d-%d]
// 最大长度为：3([-]) + sizeof(int) * 2 + 1('\0') = 68
#define POS_LEN 10
#define WORDLEN 68

// 用于存放共享数据的结构体SctSharedData中成员长度的限制。有可能比MAX_LEN大，交叉歧义字段导致。
#define MEMLEN (MAX_LEN + MAX_LETTER_NUMBER_LEN)

#define PHRASE_LEN 40
#define P2Node_LEN_1 (PHRASE_LEN * 10)
#define P2Node_LEN_2 100

#pragma pack(1)
struct SctBasic 
{
	U_CHAR nLen; //词条长度
	U_CHAR nPOS; // 标注
	size_t nID; //Word ID
};
#pragma pack()

#pragma pack(1)
struct SctPhrase 
{
	size_t nBeg;
	U_CHAR nLen; //词条长度
	U_CHAR nBasic; //基本词的个数
	U_CHAR nPOS; // 标注
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
	U_CHAR nPOS; // 标注
	U_SHORT nSlice;
	U_SHORT nFreq;
	size_t nTermID;
	int nPre1, nPre2;
};
#pragma pack()

#pragma pack(1)
struct SctSharedData 
{
	size_t nBlock; //一共分成了几块
	int nStart; // begin position
	int nID; // 每次分一行时，都需初始化
	int nStrLen; // 每次分一行时，都需初始化

	int nLookupPhrase;
	int nSubPhrase; //子短语的个数
	int nTermID; //Load sub phrase.
	char *pStr;
	char *pTemp;
	U_CHAR nPOS;
	size_t *szBlock; //记录每一块的长度
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
	// 基本词结构体
	#pragma pack(1)
	struct Sct_Basic 
	{
		U_CHAR nBeg; // 开始位置
		U_CHAR nPOS; // 标注
	};
	#pragma pack()

	// 子短语结构体
	#pragma pack(1)
	struct Sct_Phrase 
	{
		U_CHAR nBeg; // 开始位置
		U_CHAR nLen; // 基本词长度
		U_CHAR nBasicId; // 基本词的id，从0开始编号
		U_CHAR nPOS; // 标注
		U_CHAR nBasic; // 基本词的个数
	};
	#pragma pack()

	#pragma pack(1)
	struct Sct_Phrase_Basic 
	{
		U_CHAR nBasic; // 基本词的个数
		U_CHAR nPhrase; // 子短语的个数
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

	char szSubVer[VERSION_SIZE]; //存储subphrase dict版本号
	char szNumVer[VERSION_SIZE]; //存储number dict版本号

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
