#ifndef _FORMAT_FILE_H_
#define _FORMAT_FILE_H_
#include <string>

using namespace std;

class CFormatFile {
public:
	int RunFormat(const string & strFileIn, const string & strPath, const string & strFileOut, int iMustWrite);

private:

	int Process();

private:
	string m_strFileIn;
	string m_strFileOut;
	int m_iMustWrite;
};

#endif
