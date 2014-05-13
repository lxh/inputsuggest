#ifndef __MEMCMP_H_
#define __MEMCMP_H_
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include "common/SysCommon.h"

using namespace std;

//nine grid�Ƚ�
//��ת�������ִ����бȽ�
//s1 ���ִ�
static int MEMCMP_NINEGRID(const char * s1, const char * s2, const int iLen)
{
    //                     a b c d e f g h i j k l m n o p q r s t u v w x y z
    static char szLow[] = {2,2,2,3,3,3,4,4,4,5,5,5,6,6,6,7,7,7,7,8,8,8,9,9,9,9};
	static char sCharToNum[0xff] = {0};
	static bool bFirst = true;
    int iLoop;
	if(bFirst) {
		bFirst = false;
		for(iLoop = 0; iLoop < sizeof(szLow) / sizeof(szLow[0]); iLoop++) {
			sCharToNum['a' + iLoop] = szLow[iLoop] + '0';
			sCharToNum['A' + iLoop] = szLow[iLoop] + '0';
		}
	}
    //if(iLen == 1 && s1[0] == 0 && s2[0] == 0) return 0;
    for(iLoop = 0; iLoop < iLen; iLoop++) {
        char c = s2[iLoop];
		if(sCharToNum[c] != s1[iLoop]) return 1;
    }
    return 0;
}

//�Ƚϻ�ϵ�strMix, �Ƿ�չ��֮���strFull��ƥ��
//e.g: mix->zgyinh   full:zhongguoyinh �򷵻�true�� extraΪ:zhongguoyinh
//e.g: mix->zgyinha  full:zhongguoyinh �򷵻�true�� extraΪ:zhongguoyinha
//e.g: mix->zgyinha  full:zhongguoyih  �򷵻�false
static bool MEMCPY_SimpleFull(const string & strMix, const string & strFull, string & strExtra)
{
    int iMixLen = strMix.size();
    int iFullLen = strFull.size();
    int iMixLoop = 0, iFullLoop = 0;
    int iUpCaseMark = 0;
    vector<string> vecMix, vecFull;
    strExtra = "";
    for(iMixLoop = 0; iMixLoop < iMixLen; iMixLoop++) {
        const char c1 = strMix[iMixLoop];
        if(c1 >= 'A' && c1 <= 'Z') {
            string strTmp = "";
            strTmp += c1;    
            if(iMixLoop != iMixLen - 1) {
                while(1) {
                    if(iMixLoop == iMixLen - 1) break;
                    char c2 = strMix[iMixLoop + 1];
                    if(c2 >= 'A' && c2 <= 'Z') break;
                    strTmp += c2;
                    iMixLoop++;
                }
            }
            vecMix.push_back(strTmp);
        }
    }
    for(iFullLoop = 0; iFullLoop < iFullLen; iFullLoop++) {
        char c1 = strFull[iFullLoop];
        if(c1 >= 'A' && c1 <= 'Z') {
            string strTmp = "";
            strTmp += c1;    
            if(iFullLoop != iFullLen - 1) {
                while(1) {
                    if(iFullLoop == iFullLen - 1) break;
                    char c2 = strFull[iFullLoop + 1];
                    if(c2 >= 'A' && c2 <= 'Z') break;
                    strTmp += c2;
                    iFullLoop++;
                }
            }
            vecFull.push_back(strTmp);
        }
    }
    if(vecMix.size() > vecFull.size()) return false;
    for(iMixLoop = 0; iMixLoop < vecMix.size(); iMixLoop++) {
        if(vecMix[iMixLoop] == vecFull[iMixLoop] || vecMix[iMixLoop].size() == 1 && vecMix[iMixLoop][0] == vecFull[iMixLoop][0]) {
            if(vecMix[iMixLoop].size() == 1 && iMixLoop == vecMix.size() - 1) strExtra += vecFull[iMixLoop][0];
            else strExtra += vecFull[iMixLoop];
            continue;
        }
        if(iMixLoop == vecMix.size() - 1) {
            strExtra += vecMix[iMixLoop];
            if(vecMix[iMixLoop].size() < vecFull[iMixLoop].size()) {
                if(vecFull[iMixLoop].substr(0, vecMix[iMixLoop].size()) == vecMix[iMixLoop]) return true;
            } else if(vecMix[iMixLoop].size() > vecFull[iMixLoop].size() && vecMix[iMixLoop].substr(0, vecFull[iMixLoop].size()) == vecFull[iMixLoop]) {
                return true;
            }
        }
        return false;
    }
    //strExtra = strFull;
    return true;
}

//���Դ�Сд
//����0 ��ͬ,������ͬ
//����s1�Ĵ�Сд
//���s2Ϊ��д,s1����Ϊ��д,���s2ΪСд,s1Ϊ��дСд����
static int MEMCMP(const char * s1, const char * s2, const int iLen)
{
    int iLoop;
    static int iFirst = 1;
    static char szNoCase[0xff];
    if(iFirst == 1) {
        for(iLoop = 0; iLoop < sizeof(szNoCase); iLoop++) {
            if(iLoop >= 'A' && iLoop <= 'Z') {
                szNoCase[iLoop] = iLoop - 'A' + 'a';
            } else {
                szNoCase[iLoop] = iLoop;
            }
        }
        iFirst = 0;
    }

    for(iLoop = 0; iLoop < iLen; iLoop++) {
        char c = s2[iLoop];
        if(c >= 'A' && c <= 'Z') {
            if(s1[iLoop] != c) {
                return 1;
            }
        } else if(szNoCase[s1[iLoop]] != szNoCase[c]) {
            return 1;
        }
    }
    return 0;
}
#endif
