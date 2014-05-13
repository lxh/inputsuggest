#include "util/XHStrUtils.h"

#if NEED_ICONV
char convertPool[CONVERTPOOLLEN];
//#if WIN32
int lcl_convert(const char* instr, int inlen, char* outstr, const char* inputformat, const char * outputformat)
{
	int retvv = 0;
	const char *inarr = instr;
	char ** inb = (char **)&inarr;
/*    
#else
int lcl_convert(char* instr, int inlen, char* outstr, const char* inputformat, const char * outputformat)
{
	int retvv = 0;
	char *inarr = instr;
	char ** inb = &inarr;
#endif
*/

	char *outv = outstr;
	char **retv = &outstr;

	size_t inlength = inlen;
	size_t outlength = CONVERTPOOLLEN;

	iconv_t cd = iconv_open(outputformat,inputformat);
	if(cd == 0) return -1;

	if ( -1 == iconv(cd,inb,&inlength,retv,&outlength)) retvv = -1;
	else  retvv = 0;
	iconv_close(cd);

	outstr = outv;
	instr = inarr;

	return retvv;
}
void XHStrUtils::strFormatConverse(string &input,string inputFormat, string outputFormat)
{
	char *sgsrc = new char[input.size()+1];
	//memset(sgsrc, '\0', input.size()+1);
	strcpy(sgsrc, input.c_str());
	//memset(convertPool, '\0', CONVERTPOOLLEN);

	string outfmt = outputFormat + "//IGNORE";
	lcl_convert(sgsrc, input.size()+1, convertPool,inputFormat.c_str(), outfmt.c_str());

	input = convertPool;

	delete []sgsrc;
}
#endif
//切分一行csv的文件，但是如果该行里面存在有多行的单元格时，可能会出问题
int XHStrUtils::SplitCsvString(const string & strIn, vector<string> & vecOut)
{
	int iLoop;
	int iSize = strIn.size();
	string strTmp = "";
	int iRet = 0;
	for(iLoop = 0; iLoop < iSize + 1; iLoop++) {
		if(iLoop == iSize) {
			vecOut.push_back(strTmp);
			break;
		}
		char c = strIn[iLoop];
		if(c == ',') { //one seg over
			vecOut.push_back(strTmp);
			strTmp = "";
			continue;
		}
		if(c == '"' && strTmp == "") { //with special char "
			for(iLoop++; iLoop < iSize; iLoop++) {
				c = strIn[iLoop];
				if(c == '"') {
					if(iLoop == iSize - 1) { //last
						break;
					}
					iLoop++;
					c = strIn[iLoop];
					if(c == '"') {
						strTmp += '"';
					} else if(c == ',') {
						iLoop--;
						break;
					} else {
						strTmp += c;
						break;
					}
				} else {
					strTmp += c;
				}
			}
			if(iLoop == iSize) { //error exit
				vecOut.push_back(strTmp);
				iRet = -1;
				break;
			}
		} else {
			strTmp += c;
		}
	}
	return iRet;
}

//切分一个gbk编码的（包含扩展gbk，也就是第一个字节的最高位为1，第二个字节的最高位不是1）字符串。
void XHStrUtils::StrTokenizeGBK(vector<string> &v, const string& src, string tok)
{
	//vector<string> v;
	if( src.empty()) {
		return;
	}
	if(tok.empty() ) {
		v.push_back(src);
		return;
	}

	char cSplit = tok[0];
	int pre_index = 0, index = 0, len = 0;
	int iLoop = 0;
	for(iLoop = 0; iLoop < src.size(); iLoop++) {
		string strTmp = "";
		for(; iLoop < src.size(); iLoop++) {
			char c = src[iLoop];
			if(c == cSplit) {
				v.push_back(strTmp);
				break;
			} else if(c & 0x80) {
				strTmp += c;
				if(iLoop == src.size() - 1) {
					v.push_back(strTmp);
					break;
				} else {
					strTmp += src[iLoop + 1];
					iLoop++;
				}
			} else {
				strTmp += c;
			}
			if(iLoop == src.size() - 1) {
				v.push_back(strTmp);
				break;
			}
		}
	}
}

//切分一个字符串用指定的一个字节做分隔符
void XHStrUtils::StrTokenize(vector<string> &v, const string& src, string tok, bool trim)
{
	if( src.empty()) {
		return;
	}
	if(tok.empty() ) {
		v.push_back(src);
		return;
	}

	int pre_index = 0, index = 0, len = 0;
	while( (index = src.find_first_of(tok, pre_index)) != string::npos ) {
		if( (len = index-pre_index)!=0 ) {
			v.push_back(src.substr(pre_index, len));
		} else if(trim == false) {
			v.push_back("");
		}
		pre_index = index+1;
	}
	string endstr = src.substr(pre_index);
	if(trim == false) {
		v.push_back(endstr.empty() ? "" : endstr);
	} else if(!endstr.empty()) {
		v.push_back(endstr);
	}
}

//切分一个字符串，用指定的字符串做分隔符
void XHStrUtils::StrTokenizeStr(vector<string>& v, const string& src, string tok, bool trim)
{
	if( src.empty()) {
		return ;
	}
	if(tok.empty() ) {
		v.push_back(src);
		return ;
	}

	int pre_index = 0, index = 0, len = 0;
	while( (index = src.find(tok, pre_index)) != string::npos )
	{
		if( (len = index-pre_index)!=0 )
			v.push_back(src.substr(pre_index, len));
		else if(trim==false)
			v.push_back("");
		pre_index = index +	tok.size();
	}
	string endstr = src.substr(pre_index);
	if( trim==false ) v.push_back( endstr.empty()?"":endstr );
	else if( !endstr.empty() ) v.push_back(endstr);
}


void XHStrUtils::FiltNoiseSymbol(string & str)
{
	string retv = str;
	str = ""; 
	for(size_t i = 0; i < retv.size(); i++) {   
		unsigned char ch = (unsigned char)retv[i];
		if( ch >= 0x80) {   
			i++;
			if(i < retv.size()) {   
				unsigned char sch = (unsigned char)retv[i];
				if( (ch == 0xA3 && (sch >= 0xB0 && sch <= 0xB9) ) || //数字
					( ch == 0xA3 && (  (sch >= 0xE1 && sch <= 0xFA)) )    //全角小写字母
					) {
					str += (sch - 0x80);
				} else if(  ch == 0xA3 && ( (sch >= 0xC1 && sch <= 0xDA)  ) ) {    //全角大写字母
					str += (sch - 0x80) + 0x20;
				} else if(ch >= 0xA1 && ch < 0xAA) { //非中文字符
					str += ' ';
				} else if(ch == 0xA3 && ((sch == 0xA8) || (sch == 0xA9))) { //全角()
					str += ' ';
				} else {
					str += ch; 
					str += sch;
				}   
			} else {
				continue;
			}
		} else {
			if ( (ch >= 0x41 && ch <= 0x5A) ) { //大写字母
				str += ch + 0x20;
			} else if ( (ch >= 0x61 && ch <= 0x7A) || //字母
				(ch >= 0x30 && ch <= 0x39)// || /数字
				//(ch == 0x28 ) || //(
				//(ch == 0x29 ) //)
				) {
				str += ch;
			} else {
				str += ' ';
			}
		}
	}
}

bool XHStrUtils::isspace(char ch)         
{                                    
    return (ch == ' ' || ch == '\t');
}

void XHStrUtils::trimleft(string & s)
{
	string::iterator it;

	for(it = s.begin(); it != s.end(); it++) {
		if(!XHStrUtils::isspace(*it)) {
			break;
		}
	}

	s.erase(s.begin(), it);
}

void XHStrUtils::trimright(string & s)
{
	string::difference_type dt;
	string::reverse_iterator it;

	for(it = s.rbegin(); it != s.rend(); it++) {
		if(!XHStrUtils::isspace(*it)) {
			break;
		}
	}
	dt = s.rend() - it;
	s.erase(s.begin() + dt, s.end());
}

void XHStrUtils::trim(string & s)
{
	trimleft(s);
	trimright(s);
}

void XHStrUtils::ToCase(string & s, bool bUp)
{
	int iLoop;
	int iSize = s.size();
	if(bUp) {
		for(iLoop = 0; iLoop < iSize; iLoop++) {
			if(s[iLoop] >= 'a' && s[iLoop] <= 'z') {
				s[iLoop] = s[iLoop] - 'a' + 'A';
			}
		}
	} else {
		for(iLoop = 0; iLoop < iSize; iLoop++) {
			if(s[iLoop] >= 'A' && s[iLoop] <= 'Z') {
				s[iLoop] = s[iLoop] - 'A' + 'a';
			}
		}
	}
}

void XHStrUtils::JoinMultiSpace(string & s)
{
	int iSize = s.size();
	int iLoop;
	string strRet = "";
	bool bSpace = false;
	for(iLoop = 0; iLoop < iSize; iLoop++) {
		if(s[iLoop] == ' ') {
			if(bSpace) {
				continue;
			}
			strRet += s[iLoop];
			bSpace = true;
		} else {
			bSpace = false;
			strRet += s[iLoop];
		}
	}
	s = strRet;
}
