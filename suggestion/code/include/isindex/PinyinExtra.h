#ifndef _PINYIN_EXTRA_H_
#define _PINYIN_EXTRA_H_
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include "util/XHStrUtils.h"

using namespace std;
//混合的拼音的上限
#define MAX_MIX_NUMBER 15

//简拼转换成全拼使用
class CSimpleToSpell {
public:
	//这里是后插入的全拼转换成汉字使用，但是由于后来添加，所以没有对一些注释之类的修正 -->>>有些名称还是沿用之前的名称，失败啊失败
	//处理根据拼音获取汉字
	bool AddChineseSpell_2(const string & strPre, const string & strSpell, const string & strChinese) {
		if(strSpell == "" || strChinese == "") return false;
		
		map<const string, map<const string, map<const string, int> > >::iterator iIterPSS;
		vector<string> vecSplit;
		XHStrUtils::StrTokenizeGBK(vecSplit, strChinese, ",");
		int iSize = vecSplit.size();
		for(int iLoop = 0; iLoop < iSize; iLoop++) {
			string strKey = vecSplit[iLoop];
			iIterPSS = m_mapPreSimpleSpell.find(strPre);
			if(iIterPSS == m_mapPreSimpleSpell.end()) {
				m_mapPreSimpleSpell[strPre][strSpell][strKey] = 1;
			} else {
				map<const string, map<const string, int> >::iterator iIterSSI;
				iIterSSI = iIterPSS->second.find(strSpell);
				if(iIterSSI == iIterPSS->second.end()) {
					iIterPSS->second[strSpell][strKey] = 1;
				} else {
					map<const string, int>::iterator iIterS;
					iIterS = iIterSSI->second.find(strKey);
					if(iIterS == iIterSSI->second.end()) {
						iIterSSI->second[strKey] = 1;
					} else {
						iIterS->second++;
					}
				}
			}
		}
		return true;
	};
	//添加一个全拼
	bool AddSpell(const string &strPre, const string &strKey) {
		char c = strKey[0];
		if(!(c >= 'A' && c <= 'Z')) {
			return true;
		}
		map<const string, map<const string, map<const string, int> > >::iterator iIterPSS;
		string strSimple = GetSimpleSpell(strKey);
		if(strSimple == "") {
			return false;
		}
		iIterPSS = m_mapPreSimpleSpell.find(strPre);
		if(iIterPSS == m_mapPreSimpleSpell.end()) {
			m_mapPreSimpleSpell[strPre][strSimple][strKey] = 1;
		} else {
			map<const string, map<const string, int> >::iterator iIterSSI;
			iIterSSI = iIterPSS->second.find(strSimple);
			if(iIterSSI == iIterPSS->second.end()) {
				iIterPSS->second[strSimple][strKey] = 1;
			} else {
				map<const string, int>::iterator iIterS;
				iIterS = iIterSSI->second.find(strKey);
				if(iIterS == iIterSSI->second.end()) {
					iIterSSI->second[strKey] = 1;
				} else {
					iIterS->second++;
				}
			}
		}
		return true;
	};
	//输入一个简拼,获取一个全拼
	//iMixSearch = 2 代表根据拼音获取汉字
	string GetSpellFromSimple(const string & strPre, const string &strKey, const int iMixSearch) {
		int iArraySize;
		map<const string, vector<string> >::iterator iIterMV;
		iIterMV = m_mapPreSimple.find(strPre);
		if(iIterMV == m_mapPreSimple.end() || iIterMV->second.size() < 1) {
			return "";
		}
		vector<string> & vecT = iIterMV->second;
		iArraySize = vecT.size();
		//折半查找
		int iPos = -1;
		int iLeft = 0, iRight = iArraySize - 1;
		while(1) {
			int iMid = (iLeft + iRight) / 2;
			string strT = vecT[iMid];
			if(strT > strKey) {
			    if(strT.size() > strKey.size() && memcmp(strT.c_str(), strKey.c_str(), strKey.size()) == 0) {
				    iPos = iMid;
				    break;
				}
				iRight = iMid - 1;
			} else if(strT < strKey) {
				iLeft = iMid + 1;
			} else {
				iPos = iMid;
				break;
			}
			if(iLeft > iRight) {
				break;
			}
		}
		if(iPos == -1) return "";
		//查找开始的地方
		for(iLeft = iPos; iLeft >= 0; iLeft--) {
			string strT = vecT[iLeft];
			if(strT.size() >= strKey.size() && memcmp(strT.c_str(), strKey.c_str(), strKey.size()) == 0) {
				continue;
			} else {
				iLeft++;
				break;
			}
		}
		iLeft = iLeft < 0 ? 0 : iLeft;
		//查找结束的地方
		for(iRight = iPos; iRight < iArraySize; iRight++) {
			string strT = vecT[iRight];
			if(strT.size() >= strKey.size() && memcmp(strT.c_str(), strKey.c_str(), strKey.size()) == 0) {
				continue;
			} else {
				iRight--;
				break;
			}
		}
		iRight = iRight > iArraySize - 1 ? iArraySize - 1 : iRight;

		//验证前缀有效性
		map<const string, map<const string, map<const string, int> > >::iterator iIterPSS;
		iIterPSS = m_mapPreSimpleSpell.find(strPre);
		if(iIterPSS == m_mapPreSimpleSpell.end()) {
			return "";
		}

		//把所有匹配上的字符串放到mapTmp中
		map<const string, int> mapTmp, mapTmp2;
		int iLoop;
		for(iLoop = iLeft; iLoop <= iRight; iLoop++) {
			string strT = vecT[iLoop];
			map<const string, map<const string, int> >::iterator iIterSSI;
			iIterSSI = iIterPSS->second.find(strT);
			if(iIterSSI == iIterPSS->second.end()) {
				continue;
			}
			map<const string, int>::iterator iIterSI1, iIterSI2;
			for(iIterSI1 = iIterSSI->second.begin(); iIterSI1 != iIterSSI->second.end(); iIterSI1++) {
			    iIterSI2 = mapTmp.find(iIterSI1->first);
				if(iIterSI2 == mapTmp.end()) {
					mapTmp[iIterSI1->first] = iIterSI1->second;
				} else {
					iIterSI2->second += iIterSI1->second;
				}
			}
		}
		map<const int, string> mapSort;
		map<const string, int>::iterator iIterSI, iIterSI2;
		map<const int, string>::iterator iIterIS;
		vector<int> vecSort;
		for(iIterSI = mapTmp.begin(); iIterSI != mapTmp.end(); iIterSI++) {
			string strKeyLen;
			if(iMixSearch != 2) { //根据简拼获取全拼
				//获取合适长度的字符串,从mapTmp放到mapTmp2
				strKeyLen = GetRightlenString(strKey, iIterSI->first);
			} else { //根据全拼获取汉字
				strKeyLen = GetRightlenString2(strKey, iIterSI->first);
			}
			iIterSI2 = mapTmp2.find(strKeyLen);
			if(iIterSI2 == mapTmp2.end()) {
				mapTmp2[strKeyLen] = iIterSI->second;
			} else {
				iIterSI2->second += iIterSI->second;
			}
		}
		//对字符串进行排序,出现频率高的放到前面
		for(iIterSI = mapTmp2.begin(); iIterSI != mapTmp2.end(); iIterSI++) {
			iIterIS = mapSort.find(iIterSI->second);
			if(iIterIS == mapSort.end()) {
				vecSort.push_back(iIterSI->second);
				mapSort[iIterSI->second] = iIterSI->first;
			} else { //已经存在的用|把多个隔离开
				iIterIS->second += "|" + iIterSI->first;
			}
		}
		string strResult = "";
		sort(vecSort.begin(), vecSort.end());
		int iSize = vecSort.size();
		int jLoop = 0;
		for(iLoop = iSize - 1; iLoop >= 0 && jLoop < MAX_MIX_NUMBER; iLoop--, jLoop++) {
			if(strResult != "") {
				strResult += "|";
			}
			strResult += mapSort[vecSort[iLoop]];
		}
		return strResult;
	};
	bool Sort() {
		map<const string, map<const string, map<const string, int> > >::iterator iIterPSS;
		for(iIterPSS = m_mapPreSimpleSpell.begin(); iIterPSS != m_mapPreSimpleSpell.end(); iIterPSS++) {
			vector<string> vecTmp;
			map<const string, map<const string, int> >::iterator iIterSS;
			for(iIterSS = iIterPSS->second.begin(); iIterSS != iIterPSS->second.end(); iIterSS++) {
				vecTmp.push_back(iIterSS->first);
			}
			sort(vecTmp.begin(), vecTmp.end());
			m_mapPreSimple[iIterPSS->first] = vecTmp;
		}
#if 0
		map<const string, vector<string> >::iterator iIter;
		for(iIter = m_mapPreSimple.begin(); iIter != m_mapPreSimple.end(); iIter++) {
			printf("%s--==-->", iIter->first.c_str());
			for(int iLoop = 0; iLoop < iIter->second.size(); iLoop++) {
				printf("%s#", iIter->second[iLoop].c_str());
				map<const string, int>::iterator iIterSI;
				map<const string, int> & mapTmp = m_mapPreSimpleSpell[iIter->first][iIter->second[iLoop]];
				for(iIterSI = mapTmp.begin(); iIterSI != mapTmp.end(); iIterSI++) {
					printf("|%s:%d", iIterSI->first.c_str(), iIterSI->second);
				}
				printf("\n");
			}
			printf("\n\n");
		}
#endif
		return true;
	};
private:
	string GetSimpleSpell(const string &str) {
		string strSimple = "";
		int iLoop, iLen = str.size();
		bool bChange = false;
		for(iLoop = 0; iLoop < iLen; iLoop++) {
			char c = str[iLoop];
			if(c >= 'a' && c <= 'z') {
				continue;
			}
			if(c >= 'A' && c <= 'Z') {
				bChange = true;
				strSimple += c - 'A' + 'a';
			} else if(c >= '0' && c <= '9') {
				bChange = true;
				strSimple += c;
			} else {
				strSimple += c;
			}
		}
		if(bChange == false) {
			strSimple = "";
		}
		return strSimple;
	};
	/*
	//strModle 简拼  strNeedCut 全拼
	//返回: 刚好能对应简拼的全拼
	//e.g: zg ZhongGuo 返回ZhongG
	string GetRightlenString(const string & strModle, const string & strNeedCut) {
		int iLoop, jLoop = -1;
		for(iLoop = 0; iLoop < strModle.size(); iLoop++) {
			char c = strModle[iLoop];
			if(c >= 'a' && c <= 'z') {
				c = c - 'a' + 'A';
			}
			for(jLoop++; jLoop < strNeedCut.size(); jLoop++) {
				if(c == strNeedCut[jLoop]) break;
			}
			if(jLoop >= strNeedCut.size()) {
				return "";
			}
		}
		string strResult = strNeedCut.substr(0, jLoop + 1);
		return strResult;
	};
	*/
	//strModle 简拼  strNeedCut 全拼
	//返回: 刚好能对应简拼的全拼
	//e.g: zg ZhongGuoYin 返回ZhongGuo
	string GetRightlenString(const string & strModle, const string & strNeedCut) {
		int iLoop, jLoop = -1;
		for(iLoop = 0; iLoop < strModle.size(); iLoop++) {
			char c = strModle[iLoop];
			if(c >= 'a' && c <= 'z') {
				c = c - 'a' + 'A';
			}
			for(jLoop++; jLoop < strNeedCut.size(); jLoop++) {
				if(c == strNeedCut[jLoop]) break;
			}
			if(jLoop >= strNeedCut.size()) {
				return "";
			}
		}
		jLoop++;
		string strResult = strNeedCut.substr(0, jLoop);
		for(; jLoop < strNeedCut.size(); jLoop++) {
			char c = strNeedCut[jLoop];
			if(!(c >= 'a' && c <= 'z')) break;
			strResult += c;
		}
		return strResult;
	};
	//根据全拼获取汉字
	//这里使用简单粗暴不一定正确的方法
	//计算大写字母的个数和数字的个数，查看是否能匹配数字
	//error:ErOuaQu3HaoLou 二区a区3号楼
	string GetRightlenString2(const string & strModle, const string & strNeedCut) {
		int iLoop, iSize = strModle.size();
		int iSizeCut = strNeedCut.size();
		string strRet = "";
		int jLoop = 0;
		for(iLoop = 0; iLoop < iSize; iLoop++) {
			char c = strModle[iLoop];
			if(c >= 'A' && c <= 'Z') {
				//if(jLoop > iSizeCut - 1) return "";
				if(!(strNeedCut[jLoop] & 0x80)) {
					return "";
				}
				strRet += strNeedCut[jLoop++];
				strRet += strNeedCut[jLoop++];
			} else if(c >= '0' && c <= '9') {
				if(strNeedCut[jLoop] != c) {
					//printf("error:%s %s[%s %d]\n", strModle.c_str(), strNeedCut.c_str(), __FILE__, __LINE__);
					return "";
				}
				strRet += strNeedCut[jLoop++];
			}
		}
		if(jLoop > iSizeCut) {
			printf("error:%s %s[%s %d]\n", strModle.c_str(), strNeedCut.c_str(), __FILE__, __LINE__);
		}
		return strRet;
	};

private:
	//前缀对应的简拼
	map<const string, vector<string> > m_mapPreSimple;
	//前缀,简拼对应的全拼出现的次数
	map<const string, map<const string, map<const string, int> > > m_mapPreSimpleSpell;
};



#endif
