// DArray.cpp : Defines the entry point for the console application.
//

#include "segment/WordSplit.h"
#include "segment/WordSplitter.h"
#include <iostream>
using namespace std;

void funInputSplit()
{
	cout << "Begin to Load dict..." << endl;
	CWordSplit cwordSplit("../dictdir");
//	cwordSplit.install();
	cout << "Finish Loading the dict" << endl;
	char *pSplit, *pBasic, *pPhrase;
	string line;
	
	pSplit = new char[MEMLEN];
	pPhrase = new char[MEMLEN];
	pBasic = new char[MEMLEN];


	while( cin )
	{
		cout << "please input a string:" << endl;
		getline(cin, line);
		if( line.empty())
			continue;

//		line = "上海市长寿路15号";
//		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, false, false );
		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, true, true,true,true );
		cout << "Split: " << endl << pSplit << endl;
		cout << "Basic: " << endl << pBasic << endl;
		cout << "Phrase:" << endl << pPhrase << endl;
		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, true, true,true );
		cout << "Split: " << endl << pSplit << endl;
		cout << "Basic: " << endl << pBasic << endl;
		cout << "Phrase:" << endl << pPhrase << endl;
		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, false, false,true );
		cout << "Split: " << endl << pSplit << endl;
		cout << "Basic: " << endl << pBasic << endl;
		cout << "Phrase:" << endl << pPhrase << endl;
		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, true, true );
		cout << "Split: " << endl << pSplit << endl;
		cout << "Basic: " << endl << pBasic << endl;
		cout << "Phrase:" << endl << pPhrase << endl;
		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, false, false );
		cout << "Split: " << endl << pSplit << endl;
		cout << "Basic: " << endl << pBasic << endl;
		cout << "Phrase:" << endl << pPhrase << endl;
		cout << endl;
	}

	delete [] pSplit;
	delete [] pBasic;
	delete [] pPhrase;
}

void funInputSplit(string dictpath)
{
	cout << "Begin to Load dict..." << endl;
	CWordSplit cwordSplit("../dictdir");
//	cwordSplit.install();
	cout << "Finish Loading the dict" << endl;
	char *pSplit, *pBasic, *pPhrase;
	string line;
	
	pSplit = new char[MEMLEN];
	pPhrase = new char[MEMLEN];
	pBasic = new char[MEMLEN];


    ifstream in(dictpath.c_str());
	while( getline(in,line))
	{
		if( line.empty())
			continue;

//		line = "上海市长寿路15号";
//		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, false, false );
		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, true, true,true,true );
		cout << "Split: " << endl << pSplit << endl;
		cout << "Basic: " << endl << pBasic << endl;
		cout << "Phrase:" << endl << pPhrase << endl;
        /*
		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, true, true,true );
		cout << "Split: " << endl << pSplit << endl;
		cout << "Basic: " << endl << pBasic << endl;
		cout << "Phrase:" << endl << pPhrase << endl;
		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, false, false,true );
		cout << "Split: " << endl << pSplit << endl;
		cout << "Basic: " << endl << pBasic << endl;
		cout << "Phrase:" << endl << pPhrase << endl;
		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, true, true );
		cout << "Split: " << endl << pSplit << endl;
		cout << "Basic: " << endl << pBasic << endl;
		cout << "Phrase:" << endl << pPhrase << endl;
		cwordSplit.funSplitString( line.c_str(), pSplit, pBasic, pPhrase, false, false );
		cout << "Split: " << endl << pSplit << endl;
		cout << "Basic: " << endl << pBasic << endl;
		cout << "Phrase:" << endl << pPhrase << endl;
        */
		cout << endl;
	}

	delete [] pSplit;
	delete [] pBasic;
	delete [] pPhrase;
}
void test_WordSplitter()
{
	WordSplitsResult wsr;
    WordSplitter::initial("../dictdir");
    string line;
	while( cin )
	{
		cout << "please input a string:" << endl;
		getline(cin, line);
		if( line.empty())
			continue;

        WordSplitter::getInstance().split(line, wsr);

		cout<<wsr<<endl;
    }
}
void test_WordSplitter(string inpath)
{
	WordSplitsResult wsr;
    WordSplitter::initial("../dictdir");
    string line;
    ifstream in(inpath.c_str());
	while( getline(in,line) )
	{
		//cout << "please input a string:" << endl;
		//getline(cin, line);
		if( line.empty())
			continue;

        WordSplitter::getInstance().split(line, wsr);

        cout<<line<<endl;
		cout<<wsr<<endl;
    }

}

int main(int argc, char* argv[])
{
#if 1    
    /*
	if (argc == 2)
	{
		cout << "Begin to Load dict..." << endl;
		CWordSplit cwordSplit("dictdir");
		cout << "Finish Loading the dict" << endl;

		struct Sct_Version sctVer = cwordSplit.getVersion();
		cout << "Version:" << endl;
		cout << sctVer.strSplitProgram << endl;
		cout << sctVer.strDictPhrase << endl;
		cout << sctVer.strDictSubPhrase << endl;
		cout << sctVer.strDictNumber << endl;
		cout << sctVer.strDictUnit << endl;
		return 0;
	}
	funInputSplit();
    */
    funInputSplit(argv[1]);
#else
    test_WordSplitter();
    //test_WordSplitter(argv[1]);
#endif

	struct tm when;
	time_t now;

	time( &now );
	when = *localtime( &now );
	printf( "\nCurrent time is %s\n", asctime( &when ) );
    

	return 0;
}
