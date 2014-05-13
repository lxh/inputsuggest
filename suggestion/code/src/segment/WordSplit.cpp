// WordSplit.cpp: implementation of the CWordSplit class.
//
//////////////////////////////////////////////////////////////////////

#include "segment/WordSplit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDATrieDict CWordSplit::cdict;
CDATrieDict CWordSplit::cdictUnit;

CWordSplit::CWordSplit(const char * dictpath)
{
	direct = dictpath;
	install();
}

void CWordSplit::install()
{
		char *szDict = new char[256];
		char *szDictNumber = new char[256];
		char *szPhraseDict = new char[256];
		char *unitDict = new char[256];
		int npos = strlen(direct);
		memset(szDict, 0, 256);
		memset(szDictNumber, 0, 256);
		memset(szPhraseDict, 0, 256);
		memset(unitDict, 0, 256);
		strncpy(szDict, direct, npos);
		strncpy(szDictNumber, direct, npos);
		strncpy(szPhraseDict, direct, npos);
		strncpy(unitDict, direct, npos);
		if ( *(direct + npos) != '/' ) {
			strcpy(szDict + npos, "/");
			strcpy(szDictNumber + npos, "/");
			strcpy(szPhraseDict + npos, "/");
			strcpy(unitDict + npos, "/");
		}
		npos++;
		strcpy(szDict + npos, "dict.dat");
		strcpy(szDictNumber + npos, "dict_number.dat");
		strcpy(szPhraseDict + npos, "dict_subphrase.dat");
		strcpy(unitDict + npos, "dict_unit.dat");
		
		if (!cdict.load(szDict))
		{
			cout << "Fail to load the double array dictionary, please check:" << endl;
			cout << szDict << endl;
			exit(1);
		}
		if (!funLoadDictNumber(szDictNumber))
		{
			cout << "Fail to load the number dictionary, please check:" << endl;
			cout << szDictNumber << endl;
			exit(1);
		}

		if ( !funLoadPhraseDict(szPhraseDict) )
		{
			cout << "Fail to load the subphrase dictionary, please check:" << endl;
			cout << szPhraseDict << endl;
			exit(1);
		}
		if (!cdictUnit.load(unitDict))
		{
			cout << "Fail to load the unit dictionary, please check:" << endl;
			cout << unitDict << endl;
			exit(1);
		}
		
		delete [] szDict;
		delete [] szDictNumber;
		delete [] szPhraseDict;
		delete [] unitDict;

#ifdef WIN32
	InitializeCriticalSection(&cs);
#else
	pthread_mutex_init( &mutex, NULL );
#endif
	SctSharedData *pSharedData = new SctSharedData();
	sp.push( pSharedData );
	pSctBasicCmp = new SctBasic;
	pBestPhrase = new SctPhrase;
	memset( pSctBasicCmp, 0, sizeof(SctBasic) );
	memset( pBestPhrase, 0, sizeof(SctPhrase) );
}

CWordSplit::~CWordSplit()
{
	delete pBestPhrase;
	delete pSctBasicCmp;

	funFreeSubphraseRes();
	
	while ( !sp.empty() ) 
	{
		delete sp.top();
		sp.pop();
	}
#ifdef WIN32
	DeleteCriticalSection(&cs);
#else
	pthread_mutex_destroy( &mutex );
#endif
}

struct Sct_Version CWordSplit::getVersion()
{
	struct Sct_Version sctVer;
	sctVer.strSplitProgram = SPLIT_VERSION;
	sctVer.strDictPhrase = cdict.szVersion;
	sctVer.strDictSubPhrase = szSubVer;
	sctVer.strDictNumber = szNumVer;
	sctVer.strDictUnit = cdictUnit.szVersion;
	
	return sctVer;
}

size_t CWordSplit::funSplitFile(const char *filein, const char *fileout)
{
	ifstream fpin( filein );
	ofstream fpout( fileout );
	
	if ( !fpin ) 
	{
		cout << "can't open the file: " << filein << endl;
		return 0;
	}
	
	string line;
	int nLine = 0;
	size_t nBytes = 0;
	char *pSplit, *pBasic, *pPhrase;
	
	pSplit = new char[RESULT_MEM_LEN];
	pBasic = new char[RESULT_MEM_LEN];
	pPhrase = new char[RESULT_MEM_LEN];
/*	
	pSplit = new char[MEMLEN];
	pBasic = new char[MEMLEN];
	pPhrase = new char[MEMLEN];
*/
	while ( getline( fpin, line ) ) 
	{
		if ( ++nLine % 20000 == 0 ) 
		{
			cout << ".";
		}
		if ( line.length() == 0 ) 
		{
			continue;
		}
//		cout << line << endl;
		nBytes = line.length();
		funSplitString( line.c_str(), pSplit, pBasic, pPhrase, true, true );
		//funSplitString( line.c_str(), pSplit, pBasic, pPhrase, true, false );
		//funSplitString( line.c_str(), pSplit, pBasic, pPhrase, false, false );
		
//		cout << "Split: " << endl << pSplit << endl;
//		cout << "Basic: " << endl << pBasic << endl;
//		cout << "Phrase:" << endl << pPhrase << endl;

		fpout << line << endl;
		fpout << "Split: " << pSplit << endl;
		fpout << "Basic: " << pBasic << endl;
		fpout << "Phrase:" << pPhrase << endl;
		fpout << endl;
		
	}
	fpin.close();
	fpout.close();
	
	delete [] pSplit;
	delete [] pBasic;
	delete [] pPhrase;
	
	return nBytes;
}

void CWordSplit::funSplitSpeedTest(const char *filein)
{
	ifstream fpin( filein );
	
	if ( !fpin ) 
	{
		cout << "can't open the file: " << filein << endl;
		return;
	}
	
	string line;
	size_t nBytes = 0;
	char *pSplit, *pBasic, *pPhrase;
	vector<string> veclines;
	vector<string>::iterator itrVec;
	clock_t start, finish;

	cout << "Loading the file ..." << endl;
	start = clock();
		
	while ( getline( fpin, line ) ) 
	{
		if ( line.length() == 0 ) 
		{
			continue;
		}
		veclines.push_back( line );
	}
	fpin.close();
	pSplit = new char[RESULT_MEM_LEN];
	pBasic = new char[RESULT_MEM_LEN];
	pPhrase = new char[RESULT_MEM_LEN];

	finish = clock();
	cout << (double)(finish-start)/CLOCKS_PER_SEC << " s" << endl;

	start = clock();
	int i = 1; //1000
	while ( i-- > 0 ) 
	{
		for (itrVec = veclines.begin(); itrVec != veclines.end(); itrVec++) 
		{
			line = *itrVec;
			nBytes += line.length();
			
		funSplitString( line.c_str(), pSplit, pBasic, pPhrase, true, true );
//		funSplitString( line.c_str(), pSplit, pBasic, pPhrase, false, false );

//			cout << "Line: " << endl << line << endl;
//			cout << "Basic: " << endl << pBasic << endl;
//			cout << "Phrase:" << endl << pPhrase << endl;
//			cout << "Split: " << endl << pSplit << endl;
			
		}
	}
	finish = clock();
	
	delete [] pSplit;
	delete [] pBasic;
	delete [] pPhrase;
	
	double duration=(double)(finish-start)/CLOCKS_PER_SEC;
	double dMBytes=(double)nBytes/1024/1024;
	cout << duration << " seconds, " << dMBytes << " MB" << endl;
	cout << dMBytes/duration << "MB/s" << endl;
	
}

void CWordSplit::funSplitPressureTest(const char *filein, const char *fileout)
{
	ifstream fpin( filein );
	
	if ( !fpin ) 
	{
		cout << "can't open the file: " << filein << endl;
		return;
	}
	
	string line;
	size_t nBytes = 0;
	char *pSplit, *pBasic, *pPhrase;
	vector<string> veclines;
	vector<string>::iterator itrVec;
	clock_t startBegin, start, finish;
	
	while ( getline( fpin, line ) ) 
	{
		if ( line.length() == 0 ) 
		{
			continue;
		}
		veclines.push_back( line );
	}
	fpin.close();
	pSplit = new char[RESULT_MEM_LEN];
	pBasic = new char[RESULT_MEM_LEN];
	pPhrase = new char[RESULT_MEM_LEN];
	
	startBegin = clock();
	int i = 0; //1
	while ( i++ < 100000000 ) 
	{
		nBytes = 0;
		start = clock();
		ofstream fpout( fileout );
		for (itrVec = veclines.begin(); itrVec != veclines.end(); itrVec++) 
		{
			line = *itrVec;
			nBytes += line.length();
			
			funSplitString( line.c_str(), pSplit, pBasic, pPhrase, true, true );
			
			cout << "Line: " << endl << line << endl;
			cout << "Basic: " << endl << pBasic << endl;
			cout << "Phrase:" << endl << pPhrase << endl;
			cout << "Split: " << endl << pSplit << endl;
			
			fpout << line << endl;
			fpout << "Split: " << endl << pSplit << endl;
			fpout << "Basic: " << endl << pBasic << endl;
			fpout << "Phrase:" << endl << pPhrase << endl;
			fpout << endl;
		}
		fpout.close();
		finish=clock();
		double duration=(double)(finish-start)/CLOCKS_PER_SEC;
		double dMBytes=(double)nBytes/1024/1024;
		cout << "Round " << i << ":" << duration << " seconds, " << dMBytes << " MB" << endl;
		cout << dMBytes/duration << "MB/s" << endl;
	}
	delete [] pSplit;
	delete [] pBasic;
	delete [] pPhrase;
	finish=clock();
	
	double duration=(double)(finish-startBegin)/CLOCKS_PER_SEC/60000;
	cout << duration << " hours" << endl;
}


void CWordSplit::funSplitString(const char *pStr, char *pSplit,
								char *pBasic, char *pPhrase,
								bool bPosition, bool bTag, bool bID,bool bIDF)
{
	SctSharedData * pSharedData;
#ifdef WIN32
	EnterCriticalSection(&cs);
#else
	pthread_mutex_lock( &mutex );
#endif
	//如果sp栈为空，则申请一个新的SctSharedData结构，否则从sp中取一个SctSharedData结构
	//sp为缓冲
	if ( sp.empty() ) 
	{
		pSharedData = new SctSharedData();
	}
	else 
	{
		pSharedData = sp.top();
		sp.pop();
	}
#ifdef WIN32
	LeaveCriticalSection(&cs);
#else
	pthread_mutex_unlock( &mutex );
#endif

	//strcpy( pSharedData->pStr, pStr );
	//pCap2Ara->funConvNum( pStr, pSharedData->pStr );
	//funConvertNum( pSharedData->pStr, strlen(pSharedData->pStr) );
	//const char *pLine = pSharedData->pStr;
	//cout << pStr << endl;
	//申请空间，存储要分词的字符串
	char *pConvTemp = new char[strlen(pStr) + 1];
	strcpy( pConvTemp, pStr );
	//把一些特殊字符去掉，用空格代替
	funConvertNum( pConvTemp, strlen(pConvTemp) );
	const char *pLine = pConvTemp;
	//cout << "Len: " << strlen(pLine) << endl;

	//分割后的总长度跟输入串的长度不相等，说明分割错误。
	if ( strlen(pLine) != funGetBlockLen(pLine, pSharedData) )
	{
		cout << "Error! fault in funGetBlockLen()" << endl;
		cout << "	string: " << pLine << endl;
		cout << "	Original Len: " << strlen(pLine) << endl;
		cout << "	Splitted Len: " << funGetBlockLen(pLine, pSharedData) << endl;
		return;
	}
	
	pSharedData->nID = 0;
	
	size_t nBeg = 0;
	size_t nPosBasic = 0, nPosPhrase = 0, nPosBest = 0;
	size_t i = 0;

	while ( i < pSharedData->nBlock )
	{
		pSharedData->nStart = nBeg;
		pSharedData->nStrLen = pSharedData->szBlock[i];
		//cout << pSharedData->nStrLen << endl;
		pSharedData->nSubPhrase = 0; // initialize the number of the sub phrase
		memset( pSharedData->szSctBasic, 0, sizeof(SctBasic) * pSharedData->nStrLen );
		memset( pSharedData->szIntBasicEnd, 0, sizeof(int) * pSharedData->nStrLen );
		memset( pSharedData->szNumPhrase, 0, sizeof(int) * pSharedData->nStrLen );
		memset( pSharedData->szBestPhrase, 0, sizeof(SctPhrase) * pSharedData->nStrLen );

		funSplit( pLine, pSharedData, nBeg );
		funConstructBasic( pLine, pBasic, pSharedData, nPosBasic, nBeg, pSharedData->szBlock[i], bPosition, bTag, bID, bIDF );
		funConstructPhrase( pLine, pPhrase, pSharedData, nPosPhrase, bPosition, bTag, bID, bIDF  );
		funBestSplit( pLine, pSplit, pSharedData, nPosBest, bPosition, bTag, bID, bIDF  );
		nBeg += pSharedData->szBlock[i];
		i++;
	}
	*(pBasic + nPosBasic) = '\0';
	*(pPhrase + nPosPhrase) = '\0';
	*(pSplit + nPosBest) = '\0';
	
	delete[] pConvTemp;
#ifdef WIN32
	EnterCriticalSection(&cs);
#else
	pthread_mutex_lock( &mutex );
#endif
	if ( sp.size() < 5 ) 
	{
		sp.push( pSharedData );
	}
	else 
	{
		delete pSharedData;
	}
#ifdef WIN32
	LeaveCriticalSection(&cs);
#else
	pthread_mutex_unlock( &mutex );
#endif
}

bool CWordSplit::funLoadDictNumber( const char * filename )
{
	FILE *fp = fopen( filename, "rb" );
	if( !fp )
	{
		return false;
	}	

	pair< map<char *, int, ptrCmp>::iterator, bool> ret;
	int n;

	//load version info
	memset( szNumVer, 0, VERSION_SIZE*sizeof(char));
	fread(szNumVer, sizeof(char), VERSION_SIZE, fp);

	fread( &n, sizeof(int), 1, fp );
	while ( n-- ) 
	{
		char *p = new char[3];
		int nFreq;
		memset( p, 0, 3 );
		fread( p, sizeof(char), 3, fp );
		fread( &nFreq, sizeof(int), 1, fp );
		ret = mapNumber.insert( map<char *, int, ptrCmp>::value_type(p, nFreq) );
		if ( !ret.second ) 
		{
			cout << "Failed to insert:" << p << endl;
		}
	}
	fclose(fp);

	return true;
}

bool CWordSplit::funLoadPhraseDict( const char * filename )
{
	FILE *fp = fopen( filename, "rb" );
	if( !fp )
	{
		return false;
	}
	//load version info
	memset( szSubVer, 0, VERSION_SIZE*sizeof(char));
	fread(szSubVer, sizeof(char), VERSION_SIZE, fp);

	fread( &nTermTotal, sizeof(int), 1, fp );
  pSzPhraseBasic = (Sct_Phrase_Basic **)malloc((nTermTotal + 1) * sizeof(Sct_Phrase_Basic *));

	for ( int i = 1; i <= nTermTotal; i++ ) 
	{
		pSzPhraseBasic[i] = ( struct Sct_Phrase_Basic * )malloc( sizeof(struct Sct_Phrase_Basic) );
		fread( &pSzPhraseBasic[i]->nBasic, sizeof(unsigned char), 1, fp );
		pSzPhraseBasic[i]->pSct_Basic = ( struct Sct_Basic * )malloc( pSzPhraseBasic[i]->nBasic *
			sizeof(struct Sct_Basic) );
		fread( pSzPhraseBasic[i]->pSct_Basic, sizeof(struct Sct_Basic), pSzPhraseBasic[i]->nBasic, fp );
		
		fread( &pSzPhraseBasic[i]->nPhrase, sizeof(unsigned char), 1, fp );
		if ( pSzPhraseBasic[i]->nPhrase != 0 ) 
		{
			pSzPhraseBasic[i]->pSct_Phrase = ( struct Sct_Phrase * )malloc
				( pSzPhraseBasic[i]->nPhrase * sizeof(struct Sct_Phrase) );
			fread( pSzPhraseBasic[i]->pSct_Phrase, sizeof(struct Sct_Phrase), pSzPhraseBasic[i]->nPhrase, fp );
		}
	}
	fclose(fp);

	return true;
}

void CWordSplit::funFreeSubphraseRes()
{
	for ( int i = 1; i < nTermTotal; i++ )
	{
		free(pSzPhraseBasic[i]);
	}
	free(pSzPhraseBasic);

	for ( itrMapNumber = mapNumber.begin(); itrMapNumber != mapNumber.end(); itrMapNumber++ )
	{
		if ( itrMapNumber->first )
		{
			free( itrMapNumber->first );
		}
	}
}

//把一些特殊字符去掉用空格代替
void CWordSplit::funConvertNum(char *pdata, int nlen)
{
	char *pdata_end, *pnew_data, *phead;

	if(!pdata || !nlen)
		return;

	phead = pdata;
	pdata_end = pdata + nlen;
	pnew_data = pdata;

	while(*pdata && pdata < pdata_end)
	{
		if(*pdata < 0) //大于127（可能是汉字）
		{
			if(pdata + 1 < pdata_end) //汉字至少为两个字符
			{
				if(*(unsigned char *)pdata == 0xa3) //如果前面为0xa3 特殊字符
				{
					*pnew_data = (*(pdata+1)&0x7f);
					if(*pnew_data >= 'A' && *pnew_data <= 'Z')
						*pnew_data = *pnew_data + 0x20;
					pdata += 2;
					pnew_data++;
				}
				else if(*(unsigned char *)pdata == 0xa1 && *(unsigned char *)(pdata+1) == 0xa1) //空格
				{
					*pnew_data = 0x20;
					pdata += 2;
					pnew_data++;
				}
				else //gbk字符
				{
					*(short *)pnew_data = *(short *)pdata;
					pdata += 2;
					pnew_data += 2;
				}
			}
			else //如果大于127，但是是最后一个字符，则不管，继续往下走
			{
				pdata++;
			}
		}
		else
		{//如果不是字符，则用空格替换
			if(*pdata >= 'A' && *pdata <= 'Z')
				*pnew_data = *pdata + 0x20;
			else
				*pnew_data = *pdata;
			pdata++;
			pnew_data++;
		}
	}
	
	*pnew_data = 0;
	nlen = pnew_data - phead;
	return;
}

/* 
 * Function: split the input string into several parts of fixed length.
 * pLine: input string
 * pSharedData: shared data structure
 * return: pSharedData->szBlock: an array of every part's length
 *         pSharedData->nBlock: part number
 *         size_t total length after spliting
*/
size_t CWordSplit::funGetBlockLen(const char *pLine, SctSharedData * pSharedData)
{
	pSharedData->nBlock = 0;
	// pLine的长度小于MAX_LEN，则直接返回pLine的长度。
	if ( strlen(pLine) <= MAX_LEN )
	{
		pSharedData->szBlock[pSharedData->nBlock++] = strlen(pLine);
		return strlen(pLine);
	}

	const WordData * pwdBeg;
	size_t nwdLen, nBlockLen = 0, nTotalLen = 0;
	int nBeg = 0, n, nTmp, nBegPre, nNum;
	bool isNumber;
	
	while ( *(pLine + nBeg) )
	{
		n = *(pLine + nBeg) & 0x80 ? 2:1;
		nBlockLen += n;

		if ( nBlockLen >= MAX_LEN )
		{
			nTmp = cdict.lookup( pLine + nBeg, pwdBeg, nwdLen );			
			if ( !nTmp )
			{
				nBegPre = nBeg;
				isNumber = false;

				funGetCapitalNumber(pLine + nBeg, nBeg);
				if ( nBeg > nBegPre ) {
					nNum = nBeg - nBegPre;
					isNumber = true;
				}
				else {
					isNumber = funGetLetter(pLine + nBeg, nBeg);
					if ( isNumber ) {
						nNum = nBeg - nBegPre;
					}
					else {
						isNumber = funGetArabicNumber(pLine + nBeg, nBeg);
						if ( isNumber ) {
							nNum = nBeg - nBegPre;
						}
					}
				}

				if ( isNumber )
				{
					nTmp = cdictUnit.lookup( pLine + nBeg, pwdBeg, nwdLen );
					if ( nTmp )
					{
						nBlockLen += nNum + nTmp - n;
						nBeg += nTmp;
					}
					else
					{
						nBlockLen += nNum - n;
					}
				}
				else
				{
					nBeg += n;
				}
			}
			else if ( nTmp > n )
			{
				funGetMaxDisaLink( pLine, nBeg, nTmp );
				nBlockLen += nTmp - n;
				nBeg += nTmp;
			}
			else
			{
				nBeg += n;
			}
			pSharedData->szBlock[pSharedData->nBlock++] = nBlockLen;
			nTotalLen += nBlockLen;
			nBlockLen = 0;
			if ( (strlen(pLine) - nBeg) <= MAX_LEN )
			{
				pSharedData->szBlock[pSharedData->nBlock++] = strlen(pLine) - nBeg;
				nTotalLen += (strlen(pLine) - nBeg);
				break;
			}
		}
		else
		{
			nBeg += n;
		}
	}

	if ( nBlockLen > 0 )
	{
		pSharedData->szBlock[pSharedData->nBlock++] = nBlockLen;
		nTotalLen += nBlockLen;
	}

	return nTotalLen;
}


/* 
 * Function: locate at the position of the last character
 * nBeg: start
 * nLen: the length of the str to process
 * return: the length of the last character
*/
int CWordSplit::funLocateLastCharacter(const char *pLine, int &nBeg, int nLen)
{
	nLen += nBeg;
	int nMove = ( *(pLine + nBeg) & 0x80 ? 2 : 1 );
	while ( nBeg + nMove < nLen )
	{
		nBeg += nMove;
		nMove = ( *(pLine + nBeg) & 0x80 ? 2 : 1 );
	}
	return nLen - nBeg;
}

int CWordSplit::funGetCapitalNumFreq(const char *pLine, int &nBeg, int n)
{
	char * p = (char *)malloc( (n + 1) * sizeof(char) );
	int nFreq;
	strncpy( p, pLine + nBeg, n );
	*( p + n ) = '\0';
	itrMapNumber = mapNumber.find( p );
	if ( itrMapNumber != mapNumber.end() )
	{
		nFreq = itrMapNumber->second;
	}
	else
	{
		nFreq = 0;
	}
	free( p );
	return nFreq;
}

void CWordSplit::funInsertNoDisaWord(SctSharedData * pSharedData, int &nBeg, int &n)
{
	//yuanzy modified add basic dict termid  20100531
	if ( pSharedData->nTermID > 0 && pSharedData->nTermID <= nTermTotal) 
	{
		funLoadSubPhrase( nBeg, n, pSharedData );
	}
	else 
	{
		SctBasic sctBasic;
		sctBasic.nID = pSharedData->nID++;
		sctBasic.nLen = n;
		sctBasic.nPOS = pSharedData->nPOS;
		pSharedData->szSctBasic[nBeg - pSharedData->nStart] = sctBasic;
	}
}

// find the disambiguation link by using Max Match
void CWordSplit::funGetMaxDisaLink(const char *pLine, int &nBeg, int &n)
{
	//if ( (*(pLine + nBeg) & 0x80 ? 2:1) == n )
	//{
	//	return;
	//}

	const WordData *pwdBeg;
	int nTmp, nBestStart, nBestLen, nPre, i, in;
	size_t nwdLen, nLen;

	nPre = n;
	nBestStart = nBeg;
	nBestLen = n;
	i = nBeg;
	in = nBeg + n;

	while (1)
	{
		if ( ! *(pLine + in) )
		{
			break;
		}
		i += ( *(pLine + i) & 0x80 ? 2:1 );
		while ( i < in )
		{
			nLen = cdict.lookup( pLine + i, pwdBeg, nwdLen );
			nTmp = i + nLen - nBeg;
			if ( nTmp > nBestLen )
			{
				nBestStart = i;
				nBestLen = nTmp;
			}
			i += ( *(pLine + i) & 0x80 ? 2:1 );
		}
		if ( nBestLen == nPre )
		{
			break;
		}
		else
		{
			nPre = nBestLen;
			i = nBestStart;
			in = nBeg + nBestLen;
		}
	}
	
	n = nBestLen;
}


void CWordSplit::funDisambiguation(const char *pLine, int &nBeg, int &n, int &nFreq,
								   SctSharedData * pSharedData)
{
	int nTemp, nBegPre;
	int nEnd = nBeg + n;
	
	int i, j, k, l;
	int nSize1 = 0;
	int nBestSize1 = -1, nBestSize2 = -1, nBestSlice = 1000, nBestFreq = 0;
	bool bNumFlag;

	Sct_Node sct_Node;

	nBegPre = nBeg;
	
	//while ( nBegPre <= nEnd ) {
	while ( nBegPre < nEnd )
	{
		bNumFlag = false;
		nTemp = cdict.lookup_phrase_min( pLine + nBegPre, nEnd - nBegPre,
			pSharedData->szLookupPhrase, pSharedData->nLookupPhrase );
		j = 0;
		if ( !nTemp )
		{
			nTemp = (*(pLine + nBegPre) & 0x80 ? 2 : 1);
			pSharedData->nLookupPhrase = 0;
			SubPhraseData sd;
			sd.nPOS = 19;
			
			sd.nFreq = funGetCapitalNumFreq(pLine, nBegPre, nTemp);
			sd.nLen = nTemp;
			sd.nTermID = 0;
			pSharedData->szLookupPhrase[pSharedData->nLookupPhrase++] = sd;
		}
		else {
			int nTmp = (*(pLine + nBegPre) & 0x80 ? 2 : 1);
			int nFreq = funGetCapitalNumFreq(pLine, nBegPre, nTmp);
			if ( nFreq > 0 )
			{
				bNumFlag = true;
				SubPhraseData sd;
				sd.nPOS = 20; // Z
				sd.nFreq = nFreq;
				sd.nLen = nTmp;
				sd.nTermID = 0;
				pSharedData->szLookupPhrase[pSharedData->nLookupPhrase++] = sd;
				nTemp = nTmp;
			}
		}
		if ( 0 == nSize1 ) {
			for ( i = pSharedData->nLookupPhrase - 1; i >= 0; i-- )
			{
				//Sct_Node sct_Node;
				sct_Node.nPre1 = -1;
				sct_Node.nPre2 = -1;
				sct_Node.nBeg = nBegPre;
				sct_Node.nLen = pSharedData->szLookupPhrase[i].nLen;
				if (pSharedData->szLookupPhrase[i].nPOS == 8)
				{
					sct_Node.nSlice = 0;
				}
				else if ( bNumFlag || (sct_Node.nLen > 2 ) ) 
				{
					//sct_Node.nSlice = 1;
					sct_Node.nSlice = 2;
				}
				else
				{
					//sct_Node.nSlice = 2;
					sct_Node.nSlice = 3;
				}
				sct_Node.nFreq = pSharedData->szLookupPhrase[i].nFreq;
				sct_Node.nPOS = pSharedData->szLookupPhrase[i].nPOS;
				sct_Node.nTermID = pSharedData->szLookupPhrase[i].nTermID;
				pSharedData->p2Node[nSize1][j++] = sct_Node;
			}
			pSharedData->pSize2[nSize1] = j;
			nSize1++;
			nBegPre += nTemp;
			continue;
		}
		Sct_Node sctNode;
		Sct_Node sctNodeBest;

		sctNodeBest.nBeg = 0; // not necessary
		sctNodeBest.nFreq = 0; // not necessary
		sctNodeBest.nLen = 0; // not necessary
		sctNodeBest.nPOS = 0; // not necessary
		sctNodeBest.nPre1 = -1; // not necessary
		sctNodeBest.nPre2 = -1; // not necessary
		sctNodeBest.nSlice = 0; // not necessary
		sctNodeBest.nTermID = 0; // not necessary

		bool bFirst;
		for ( i = pSharedData->nLookupPhrase - 1; i >= 0; i-- )
		{
			bFirst = true;
			for ( k=0; k < nSize1; k++ ) 
			{
				for ( l=0; l < pSharedData->pSize2[k]; l++ )
				{
					//if (( pSharedData->p2Node[k][l].nSlice <= nSliceMax ) &&
					//	((pSharedData->p2Node[k][l].nBeg + pSharedData->p2Node[k][l].nLen) == nBegPre) ){
					if ( (pSharedData->p2Node[k][l].nBeg + pSharedData->p2Node[k][l].nLen) == static_cast<size_t>(nBegPre) )
					{
						sctNode.nPre1 = k;
						sctNode.nPre2 = l;
						sctNode.nBeg = nBegPre;
						sctNode.nLen = pSharedData->szLookupPhrase[i].nLen;
						if (pSharedData->szLookupPhrase[i].nPOS == 8)
						{
							sctNode.nSlice = pSharedData->p2Node[k][l].nSlice;
						}
						else if ( bNumFlag || (sctNode.nLen > 2) )
						{
							sctNode.nSlice = pSharedData->p2Node[k][l].nSlice + 2;
						}
						else 
						{
							sctNode.nSlice = pSharedData->p2Node[k][l].nSlice + 3;
						}
						sctNode.nFreq = pSharedData->szLookupPhrase[i].nFreq
							+ pSharedData->p2Node[k][l].nFreq;
						sctNode.nPOS = pSharedData->szLookupPhrase[i].nPOS;
						sctNode.nTermID = pSharedData->szLookupPhrase[i].nTermID;
						if( bFirst )
						{
							sctNodeBest = sctNode;
							bFirst = false;
						}
						else if((sctNodeBest.nSlice > sctNode.nSlice) || ((sctNodeBest.nSlice == sctNode.nSlice) && (sctNodeBest.nFreq < sctNode.nFreq))) {
							sctNodeBest = sctNode;
						}
					}
				} // for l
			} // for k
			if ( j >= P2Node_LEN_2 )
			{
				//cout << "Warning! Out of memory! P2Node_LEN_2 is too small. j:" << j << endl;
				//cout << pLine << endl;
				funInsertNoDisaWord( pSharedData, nBeg, n );
				return;
				//exit(1);
			}
			pSharedData->p2Node[nSize1][j++] = sctNodeBest;
		} // for i
		pSharedData->pSize2[nSize1] = j;
		nSize1++;
		if ( nSize1 >= P2Node_LEN_1 ) 
		{
			//cout << "Warning! Out of memory! P2Node_LEN_1 is too small" << endl;
			funInsertNoDisaWord( pSharedData, nBeg, n );
			return;
			//exit(1);
		}

		nBegPre += nTemp;
	} // while ( nBegPre < nEnd )
	
	// find a best result
	for ( k=0; k < nSize1; k++ )
	{
		for ( l=0; l < pSharedData->pSize2[k]; l++ ) 
		{
			Sct_Node sct_Node = pSharedData->p2Node[k][l];
			if ( sct_Node.nBeg + sct_Node.nLen == static_cast<size_t>(nEnd) ) 
			{
				if ( sct_Node.nSlice < nBestSlice )
				{
					nBestSlice = sct_Node.nSlice;
					nBestFreq = sct_Node.nFreq;
					nBestSize1 = k;
					nBestSize2 = l;
				}
				else if ( sct_Node.nSlice == nBestSlice ) 
				{
					if ( sct_Node.nFreq > nBestFreq )
					{
						nBestFreq = sct_Node.nFreq;
						nBestSize1 = k;
						nBestSize2 = l;
					}
				}
			}
		} // for l
	} // for k
	
	if ( (nBestSize1 == -1) || (nBestSize2 == -1) )
	{
		cout << "Error! void CWordSplit : : funDisambiguation! " << pLine << endl;
		return;
	}
	

	// back trace
	i = nBestSize1;
	j = nBestSize2;
	SubPhraseData sct_suffix;
	sct_suffix.nFreq = i; // nFreq: the first dimension
	sct_suffix.nLen = j; // nLen: the second dimension
	sct_suffix.nPOS = 0; // not use
	sct_suffix.nTermID = 0; // not use
	pSharedData->nLookupPhrase = 0;
	pSharedData->szLookupPhrase[pSharedData->nLookupPhrase++] = sct_suffix;
	while ( pSharedData->p2Node[i][j].nPre1 != -1 )
	{
		k = pSharedData->p2Node[i][j].nPre1;
		l = pSharedData->p2Node[i][j].nPre2;
		i = k;
		j = l;
		sct_suffix.nFreq = i; // nFreq: the first dimension
		sct_suffix.nLen = j; // nLen: the second dimension
		pSharedData->szLookupPhrase[pSharedData->nLookupPhrase++] = sct_suffix;
	}

	for ( i = pSharedData->nLookupPhrase - 1; i >= 0 ; i-- )
	{
		Sct_Node sct_Node = pSharedData->p2Node[pSharedData->szLookupPhrase[i].nFreq][pSharedData->szLookupPhrase[i].nLen];
		//yuanzy modified add basic dict termid  20100531
		if ( sct_Node.nTermID && sct_Node.nTermID <= static_cast<size_t>(nTermTotal))
		{
			pSharedData->nTermID = sct_Node.nTermID;
			funLoadSubPhrase( sct_Node.nBeg, sct_Node.nLen, pSharedData );
		}
		else 
		{
			SctBasic sctBasic;
			sctBasic.nID = pSharedData->nID++;
			sctBasic.nLen = sct_Node.nLen;
			sctBasic.nPOS = sct_Node.nPOS;
			pSharedData->szSctBasic[sct_Node.nBeg - pSharedData->nStart] = sctBasic;
		}
	}
	//funPrintDisambiguation(pLine, nSize1, pSharedData);
	n = nEnd - nBeg;
}

void CWordSplit::funPrintDisambiguation(const char *pLine, int nSize1, SctSharedData * pSharedData)
{
	int k,l;
	for ( k=0; k < nSize1; k++ ) 
	{
		for ( l=0; l < pSharedData->pSize2[k]; l++ )
		{
				Sct_Node sct_Node = pSharedData->p2Node[k][l];
				cout << sct_Node.nPre1 << " " << sct_Node.nPre2 << endl;
				cout << sct_Node.nBeg << " "  << sct_Node.nLen << endl;
				cout << sct_Node.nSlice << " "  << sct_Node.nFreq << endl;
		} // for l
	} // for k
}

vector<int> CWordSplit::getTermID(const char*pStr,int len)
{
	const WordData * pwdBeg;
  vector<int> result;
	if(len > 100) 
  {
    result.push_back(0);
    result.push_back(0);
		return result;
  }
	if( !*pStr&0x80)
	{
    result.push_back(0);
    result.push_back(0);
		return result;
	}
	char temp[100];
	strncpy(temp,pStr,len);
	temp[len] = '\0';
	size_t nwdLen;
	
	int n = cdict.lookup( temp , pwdBeg, nwdLen );
	if( !n )
	{
    result.push_back(0);
    result.push_back(0);
		return result;
	}
	else
	{
		result.push_back(pwdBeg[0].nTermID);
		result.push_back(pwdBeg[0].idf);
    return result;
	}
}

/*
分词程序
*/
void CWordSplit::funSplit(const char *pLine, SctSharedData * pSharedData, int nBeg)
{
	const WordData * pwdBeg;
	size_t nwdLen;
	int n = 0, nFreq;
	//int nBeg = 0, nPre;
	int nPre, nEnd;

	nEnd = pSharedData->nStrLen + nBeg;
	
	while( nBeg < nEnd )
	{
		n = cdict.lookup( pLine + nBeg, pwdBeg, nwdLen );
		if( !n )
		{
			funNumBasic( pLine, nBeg, n, pSharedData);
		}
		else
		{
			if ( nwdLen <= 0 )
			{
				cout << "Error: CWordSplit::funSplit(...) [nwdLen <= 0]" << endl;
				nBeg += n;
				continue;
			}
			nFreq = pwdBeg[0].nFreq;
			pSharedData->nTermID = pwdBeg[0].nTermID;
			pSharedData->nPOS = pwdBeg[0].nPOS;
			nPre = n;

			if ( n > (*(pLine + nBeg) & 0x80 ? 2:1) ) //if n's length is a character's length, there is no disambiguation.
			{
				funGetMaxDisaLink(pLine, nBeg, n);
			}
			
			if ( nPre == n )
			{ // no disambiguation
				funInsertNoDisaWord( pSharedData, nBeg, n );
			}
			else
			{
				funDisambiguation(pLine, nBeg, n, nFreq, pSharedData);
			}
		}
		nBeg += n;
	}
}

void CWordSplit::funLoadSubPhrase(int nBeg, int n, SctSharedData * pSharedData)
{
	if ( (pSharedData->nTermID < 1) || (pSharedData->nTermID > nTermTotal) ) 
	{
		cout << "sub phrase not found: " << pSharedData->nTermID << endl;
		return;
	}
	else
	{
		int i;
		int nID = pSharedData->nID;
		Sct_Phrase_Basic *pSctSubPhrase = pSzPhraseBasic[pSharedData->nTermID];
		for ( i = 0; i < pSctSubPhrase->nBasic - 1; i++)
		{
			SctBasic sctBasic;
			sctBasic.nID = pSharedData->nID++;
			sctBasic.nLen = pSctSubPhrase->pSct_Basic[i + 1].nBeg - pSctSubPhrase->pSct_Basic[i].nBeg;
			sctBasic.nPOS = pSctSubPhrase->pSct_Basic[i].nPOS;
			pSharedData->szSctBasic[nBeg + pSctSubPhrase->pSct_Basic[i].nBeg - pSharedData->nStart] = sctBasic;
		}
		SctBasic sctBasic;
		sctBasic.nID = pSharedData->nID++;
		sctBasic.nLen = n - pSctSubPhrase->pSct_Basic[i].nBeg;
		sctBasic.nPOS = pSctSubPhrase->pSct_Basic[i].nPOS;
		pSharedData->szSctBasic[nBeg + pSctSubPhrase->pSct_Basic[i].nBeg - pSharedData->nStart] = sctBasic;

		for ( i = 0; i < pSctSubPhrase->nPhrase; i++)
		{
			SctPhrase sctPhrase;
			sctPhrase.nBeg = nBeg + pSctSubPhrase->pSct_Phrase[i].nBeg - pSharedData->nStart;
			sctPhrase.nID = nID + pSctSubPhrase->pSct_Phrase[i].nBasicId;
			sctPhrase.nLen = pSctSubPhrase->pSct_Phrase[i].nLen;
			sctPhrase.nBasic = pSctSubPhrase->pSct_Phrase[i].nBasic;
			sctPhrase.nPOS = pSctSubPhrase->pSct_Phrase[i].nPOS;
			if ( sctPhrase.nLen == n)
			{
				sctPhrase.isMaxPhrase = true;
			}
			pSharedData->szSubPhrase[pSharedData->nSubPhrase++] = sctPhrase;
		}
	}
}

void CWordSplit::funConstructBasic(const char * pLine, char * pBasic,
									SctSharedData * pSharedData, size_t &nPos,
									size_t &nStart, size_t &nEnd,
									bool bPosition, bool bTag,bool bID,bool bIDF)
{
	SctBasic sctBasic;
	//size_t nPos = 0;
	for (int i = 0; i < pSharedData->nStrLen; i++)
	{
		if ( memcmp(&pSharedData->szSctBasic[i], pSctBasicCmp, sizeof(SctBasic)) == 0 )
		{
			continue;
		}
		sctBasic = pSharedData->szSctBasic[i];
		if ( bPosition ) 
		{
			memset( pSharedData->pTemp, 0, WORDLEN );
			sprintf( pSharedData->pTemp, "[%d-%d]",  i + pSharedData->nStart, sctBasic.nLen );
			strncpy( pBasic + nPos, pSharedData->pTemp, strlen(pSharedData->pTemp) );
			nPos += strlen(pSharedData->pTemp);
		}
		
		strncpy( pBasic + nPos, pLine + i + pSharedData->nStart, sctBasic.nLen );
		nPos += sctBasic.nLen;
		
		if(bID)
		{
      vector<int> result = getTermID(pBasic + nPos-sctBasic.nLen,sctBasic.nLen);
      int id_term = result[0];
      int idf_term = result[1];
      char temp[100];
      memset(temp,'\0',100);
      sprintf( temp, "%d", id_term );
      strncpy( pBasic + nPos, "/", 1 );
      nPos++;
      strncpy( pBasic + nPos, temp,strlen(temp) );
      nPos += strlen(temp);
      if(bIDF)
      {
      memset(temp,'\0',100);
      sprintf( temp, "%d", idf_term );
      strncpy( pBasic + nPos, "/", 1 );
      nPos++;
      strncpy( pBasic + nPos, temp,strlen(temp) );
      nPos += strlen(temp);
      }
		}


		//记录pos
		if ( bTag ) 
		{
			strncpy( pBasic + nPos, "/", 1 );
			nPos++;
			sprintf( pSharedData->pTemp, "%d", (int)sctBasic.nPOS );
			strncpy( pBasic + nPos, pSharedData->pTemp,
				strlen(pSharedData->pTemp) );
			nPos += strlen(pSharedData->pTemp);
		}
		
		strncpy( pBasic + nPos, " ", 1 );
		nPos++;
	}
}

// int nID, 当前基本词的ID
// size_t nWordNum, 包含基本词的个数
// const char *pStr, 原始字符串
// char *pPhrase, 子短语组成的字符串
// int &nBeg, 开始位置
// int &nPhrase, 长度
// int &nPOS, 标注
// size_t &nPosPhrase 已赋值的短语位置
void CWordSplit::funConstructPhrase(const char * pStr, char * pPhrase,
									SctSharedData * pSharedData, size_t &nPosPhrase,
									bool bPosition, bool bTag, bool bID, bool bIDF)
{
	int nID, nBeg, nPhrase = 0, i = 0;
	//size_t nPosPhrase = 0;
	
	for ( i = 0; i < pSharedData->nSubPhrase; i++)
	{
		nBeg = pSharedData->szSubPhrase[i].nBeg;
		nID = pSharedData->szSubPhrase[i].nID;
		nPhrase = pSharedData->szSubPhrase[i].nLen;
		if ( bPosition ) 
		{
			memset( pSharedData->pTemp, 0, WORDLEN );
			sprintf( pSharedData->pTemp, "[%d-%d]", nID, pSharedData->szSubPhrase[i].nBasic );
			strncpy( pPhrase + nPosPhrase, pSharedData->pTemp, strlen(pSharedData->pTemp) );
			nPosPhrase += strlen(pSharedData->pTemp);
		}
		
		strncpy( pPhrase + nPosPhrase, pStr + nBeg + pSharedData->nStart, nPhrase );
		nPosPhrase += nPhrase;
		
		if(bID)
		{
      vector<int> result = getTermID(pPhrase + nPosPhrase-nPhrase,nPhrase);
      int id_term = result[0];
      int idf_term = result[1];
      char temp[100];
      memset(temp,'\0',100);
      sprintf( temp, "%d", id_term );
      strncpy( pPhrase + nPosPhrase, "/", 1 );
      nPosPhrase++;
      strncpy( pPhrase + nPosPhrase, temp,strlen(temp) );
      nPosPhrase += strlen(temp);
      if(bIDF)
      {
      memset(temp,'\0',100);
      sprintf( temp, "%d", idf_term );
      strncpy( pPhrase + nPosPhrase, "/", 1 );
      nPosPhrase++;
      strncpy( pPhrase + nPosPhrase, temp,strlen(temp) );
      nPosPhrase += strlen(temp);
      }
		}

		
		if ( bTag ) 
		{
			strncpy( pPhrase + nPosPhrase, "/", 1 );
			nPosPhrase++;
			sprintf( pSharedData->pTemp, "%d", (int)pSharedData->szSubPhrase[i].nPOS );
			strncpy( pPhrase + nPosPhrase, pSharedData->pTemp,
				strlen(pSharedData->pTemp) );
			nPosPhrase += strlen(pSharedData->pTemp);
		}
		
		strncpy( pPhrase + nPosPhrase, " ", 1 );
		nPosPhrase++;
		
		if ( pSharedData->szSubPhrase[i].isMaxPhrase ) 
		{
			SctPhrase sctPhrase = pSharedData->szSubPhrase[i];
			sctPhrase.nBasic = pSharedData->szSubPhrase[i].nBasic;
			pSharedData->szBestPhrase[nBeg] = sctPhrase;
		}
	}
	//* (pPhrase + nPosPhrase) = '\0';
}

// construct the pResult string.
void CWordSplit::funBestSplit(const char *pStr, char *pSplit,
							  SctSharedData * pSharedData, size_t &nPosBest,
							  bool bPosition, bool bTag, bool bID, bool bIDF)
{
	int n, nID = 0, nBasic = 0, nBeg = 0;
	//size_t nPosSplit = 0;
	unsigned char nPOS;

	while ( nBeg < pSharedData->nStrLen ) 
	{
		if ( memcmp(&pSharedData->szBestPhrase[nBeg], pBestPhrase, sizeof(SctPhrase)) != 0) 
		{
			nID = pSharedData->szBestPhrase[nBeg].nID;
			n = pSharedData->szBestPhrase[nBeg].nLen;
			if ( pSharedData->szBestPhrase[nBeg].isMaxPhrase ) 
			{
				nBasic = pSharedData->szBestPhrase[nBeg].nBasic;
			}
			else
			{
				nBasic = 1;
				cout << "Error: (CWordSplit::funBestSplit)" << endl;
			}
			nPOS = pSharedData->szBestPhrase[nBeg].nPOS;
		}
		else
		{
			if ( memcmp(&pSharedData->szSctBasic[nBeg], pSctBasicCmp, sizeof(SctBasic)) != 0) 
			{
				nID = pSharedData->szSctBasic[nBeg].nID;
				n = pSharedData->szSctBasic[nBeg].nLen;
				nBasic = 1;
				nPOS = pSharedData->szSctBasic[nBeg].nPOS;
			}
			else 
			{
				n = ( *(pStr + nBeg) & 0x80 ? 2:1 );
				nPOS = 19; //"O"
//				cout << "Error! -- CWordSplit::funBestSplit" << endl;
			}
		}
		
		if ( bPosition ) 
		{
			memset( pSharedData->pTemp, 0, WORDLEN );
			sprintf( pSharedData->pTemp, "[%d-%d]",  nID, nBasic );
			strncpy( pSplit + nPosBest, pSharedData->pTemp, strlen(pSharedData->pTemp) );
			nPosBest += strlen(pSharedData->pTemp);
		}
		strncpy( pSplit + nPosBest, pStr + nBeg + pSharedData->nStart, n );
		nPosBest += n;

		if (bID)
		{
      vector<int> result = getTermID(pSplit + nPosBest -n,n);
      int id_term = result[0];
			int idf_term = result[1];
      char temp[100];
			memset(temp,'\0',100);
			sprintf( temp, "%d", id_term );
			strncpy( pSplit + nPosBest, "/", 1 );
			nPosBest++;
			strncpy( pSplit + nPosBest, temp,strlen(temp) );
			nPosBest += strlen(temp);
      if(bIDF)
      {
      memset(temp,'\0',100);
      sprintf( temp, "%d", idf_term );
      strncpy( pSplit + nPosBest, "/", 1 );
      nPosBest++;
      strncpy( pSplit + nPosBest, temp,strlen(temp) );
      nPosBest += strlen(temp);
      }
		}


		if ( bTag )
		{
			strncpy( pSplit + nPosBest, "/", 1 );
			nPosBest++;
			sprintf( pSharedData->pTemp, "%d", (int)nPOS );
			strncpy( pSplit + nPosBest, pSharedData->pTemp, strlen(pSharedData->pTemp) );
			nPosBest += strlen(pSharedData->pTemp);
		}
		strncpy( pSplit + nPosBest, " ", 1 );
		nPosBest++;
		nBeg += n;
	}
	//* (pSplit + nPosBest) = '\0';
}

void CWordSplit::funNumPhrase(const char *pLine, SctSharedData * pSharedData,
							  int nBegPre, int &nBeg, int &n)
{
	const WordData * pwdBeg;
	size_t nwdLen;
	int nUnit;
	int nTemp;

	nUnit = cdictUnit.lookup( pLine + nBeg, pwdBeg, nwdLen );

	if ( !nUnit )
	{
		return;
	}
	else if ( 2 == nUnit ) 
	{ // 第七门市部
		nTemp = nBeg;
		nTemp = cdict.lookup( pLine + nTemp, pwdBeg, nwdLen );
		if ( nTemp > 2 ) 
		{ // 单字词不算
			return;
		}
	}

	funInsertSctBasic(nBeg, nUnit, pSharedData);

	SctPhrase sctPhrase;
	sctPhrase.nBeg = nBegPre - pSharedData->nStart;
	sctPhrase.nID = pSharedData->nID - 2; // 需要减去当前的单元和数字部分
	sctPhrase.nLen = n + nUnit;
	sctPhrase.nBasic = 2;
	sctPhrase.nPOS = pSharedData->nPOS;
	sctPhrase.isMaxPhrase = true;
	pSharedData->szSubPhrase[pSharedData->nSubPhrase++] = sctPhrase;

	nBeg += nUnit;
	n += nUnit;
}

void CWordSplit::funInsertSctBasic(int &nBeg, int &n, SctSharedData * pSharedData)
{
	SctBasic sctBasic;
	sctBasic.nID = pSharedData->nID++;
	sctBasic.nLen = n;
	//strcpy( sctBasic.szPOS, pSharedData->pPOS );
	sctBasic.nPOS = pSharedData->nPOS;
	pSharedData->szSctBasic[nBeg - pSharedData->nStart] = sctBasic;
}

bool CWordSplit::funNumBasic(const char *pLine, int &nBeg, int &n, SctSharedData * pSharedData)
{
	int nBegPre = nBeg;
	bool isNumber = false;

	funGetCapitalNumber(pLine + nBeg, nBeg);
	if ( nBeg > nBegPre ) {
		n = nBeg - nBegPre;
		isNumber = true;
	}
	else {
		isNumber = funGetLetter(pLine + nBeg, nBeg);
		if ( isNumber ) {
			n = nBeg - nBegPre;
		}
		else {
			isNumber = funGetArabicNumber(pLine + nBeg, nBeg);
			if ( isNumber ) {
				n = nBeg - nBegPre;
			}
		}
	}

	if ( isNumber )
	{
		pSharedData->nPOS = 20; // "Z"
		funInsertSctBasic(nBegPre, n, pSharedData);
		funNumPhrase(pLine, pSharedData, nBegPre, nBeg, n);
	}
	else 
	{
		n = ( *(pLine + nBeg) & 0x80 ? 2 : 1 );
		pSharedData->nPOS = 19; //"O"
		funInsertSctBasic(nBegPre, n, pSharedData);
	}
	nBeg = nBegPre;
	return isNumber;
}

// 提取大写数字
void CWordSplit::funGetCapitalNumber(const char *p, int &nBeg)
{
	if(!p) {
		return;
	}
	size_t nMove;
	int n = 0;
	char * pTemp = (char *)malloc( (2 + 1) * sizeof(char) );
	
	while( *(p + n) ) {
		if( *(p + n) & 0x80 ) {
			nMove = 2;
		}
		else {
			nMove = 1;
		}
		strncpy( pTemp, p + n, nMove );
		*( pTemp + nMove ) = '\0';
		itrMapNumber = mapNumber.find( pTemp );
		if ( itrMapNumber != mapNumber.end() ) {
			n += nMove;
		}
		else {
			break;
		}
		if ( n >= MAX_LETTER_NUMBER_LEN )
		{
			break;
		}
	}
	if (n > 0) {
		nBeg += n;
	}
	free( pTemp );
}

// 提取字母
bool CWordSplit::funGetLetter(const char *p, int &nBeg)
{
	if(!p) {
		return false;
	}
	int n = 0;
	
	while( *(p + n) ) {
		if( *(p + n) & 0x80 ) {
			break;
		}
				
		if( (*(p+n)>='a' && *(p+n)<='z') || (*(p+n)>='A' && *(p+n)<='Z') ) {
			n++;
		}
		else {
			break;
		}
		if ( n >= MAX_LETTER_NUMBER_LEN )
		{
			break;
		}
	}
	if (n > 0) {
		nBeg += n;
		return true;
	}
	else {
		return false;
	}
}

// 提取阿拉伯数字
bool CWordSplit::funGetArabicNumber(const char *p, int &nBeg)
{
	if(!p) {
		return false;
	}
	int n = 0;
	
	while( *(p + n) ) {
		if( *(p + n) & 0x80 ) {
			break;
		}
				
		if( *(p+n) >= '0' && *(p+n) <= '9' ) {
			n++;
		}
		else {
			break;
		}
		if ( n >= MAX_LETTER_NUMBER_LEN )
		{
			break;
		}
	}
	if (n > 0) {
		nBeg += n;
		return true;
	}
	else {
		return false;
	}
}
