#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "isindex/WordStringInfo.h"
#include "util/XHStrUtils.h"
//������Ҫ�Ĺ����Ǵ����ַ���(����&ƴ��)�ĵ���-->>�����м������ַ�����λ��
//�Լ��������ŷŵ�һ���ַ�������Ҫ�Ŀռ�
//���ں��ִʵ䣬����һ�������Ĺ�����д��չ��Ϣ
//������ִʵ�����Ĵ���ڸ�ʽ��ǰ�����˱仯����ôԭ��Ҳ��������

//������Ҫд��չ��Ϣ���ֶ�
int CWordStringInfo::AnlyInfo(const string & strInFile, const string & strOutFile, const string & strOutFileExtra, const int & iType)
{
	m_strInFile       = strInFile;
	m_strOutFile      = strOutFile;
	m_strOutFileExtra = strOutFileExtra;
	m_iType           = iType;

	if(m_iType == EXTRA_INFO_SET_INSIDE) {
		m_iWordField = 0;
	} else {
		m_iWordField = 1;
	}

	Process();
	return 0;
}
//����0����û���ҵ�
unsigned int CWordStringInfo::GetWordPos(const string & strKey)
{
	map<const string, unsigned int>::iterator iIter;
	iIter = m_mapWordPos.find(strKey);
	if(iIter != m_mapWordPos.end()) {
		return iIter->second;
	}
	return 0;
}

//���ַ����������洢�����ʵ�
//Ȼ����ݸôʵ�����м��ѯ�Ľ��
//�����м�ƴ��/��ƴ,��չ�м�ƴ��/��ƴ
void CWordStringInfo::Reverse()
{
	map<const string, unsigned int>::iterator iIter;
	int iLoop;
	for(iIter = m_mapWordPos.begin(); iIter != m_mapWordPos.end(); iIter++) {
		string strKey = iIter->first;
		string strKeyReverse = "";
		for(iLoop = strKey.size() - 1; iLoop >= 0; iLoop--) {
			strKeyReverse += strKey[iLoop];
		}
		m_vecReverseSort.push_back(make_pair(strKeyReverse, iIter->second));
	}
	sort(m_vecReverseSort.begin(), m_vecReverseSort.end(), PairLess);
	m_iReverseVecSize = m_vecReverseSort.size();
}

//�����۰���ҵķ�����ȡһ���ʵ�λ��
unsigned int CWordStringInfo::GetReverseWordPos(const string & strKey)
{
	string strReverseKey = "";
	int iLoop;
	for(iLoop = strKey.size() - 1; iLoop >= 0; iLoop--) {
		strReverseKey += strKey[iLoop];
	}
	int iLeft = 0;
	int iRight = m_iReverseVecSize - 1;
	int iRetPos = -1;
	while(1) {
		int iMid = (iLeft + iRight + 1) / 2;
		if(iRight - iLeft < 2) {
			int iLR = iRight;
			if(m_vecReverseSort[iLeft].first > strReverseKey) {
				iLR = iLeft;
			}
			iRetPos = m_vecReverseSort[iLR].second + m_vecReverseSort[iLR].first.size() - strReverseKey.size();
#if 0 //test
			string strRe = "";
			for(iLoop = m_vecReverseSort[iLR].first.size() - 1; iLoop >= 0; iLoop--) {
				strRe += m_vecReverseSort[iLR].first[iLoop];
			}

			printf("key in:%s; reverse:%s; match:%s; ret:%s; iRetPos:%d; %s\n", strKey.c_str(), strReverseKey.c_str(), m_vecReverseSort[iLR].first.c_str(), strRe.c_str()+ m_vecReverseSort[iLR].first.size() - strReverseKey.size(), iRetPos, strRe.c_str());
#endif
			return iRetPos;
		}
		if(m_vecReverseSort[iMid].first >= strReverseKey) {
			iRight = iMid;
		} else {
			iLeft = iMid;
		}
	}
	printf("error:(exit)-->GetReverseWordPos, strKey:%s[%s %d]\n", strKey.c_str(), __FILE__, __LINE__);
	return -1;
}

void CWordStringInfo::Release()
{
	m_mapWordPos.erase(m_mapWordPos.begin(), m_mapWordPos.end());
}

int CWordStringInfo::Process()
{
	char szZero[4] = {0};
	CReadBigFile fdR;
	fdR.Open(m_strInFile.c_str());

	int iFdOut = open(m_strOutFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	int iFdOutExtra = -1;
	unsigned int uiExtraPos = 4;
	unsigned int uiWordPos = 4;
	if(m_iType == EXTRA_INFO_SET_INSIDE) {
		iFdOutExtra = open(m_strOutFileExtra.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		write(iFdOutExtra, szZero, 4);
	}
	write(iFdOut, szZero, 4);
	char *p;
	while(fdR.GetLine(p)) {
		vector<string> vecSplit;
		XHStrUtils::StrTokenize(vecSplit, p, ",");
		if(vecSplit.size() < m_iWordField + 1) {
			printf("error: the word field is not good:%s:%d\n", p, vecSplit.size());
			exit(-1);
		}
		string strWord = vecSplit[m_iWordField];
		if(m_iType == EXTRA_INFO_SET_INSIDE) {
			//��չ������������
			//��һ����Ϊ����ԭ��,�ڶ�����Ϊ��������չ��Ϣ
			//bool bDiffWord = false;
			string strOrigKey = vecSplit[2];
			string strPre = vecSplit[1];
			//�Ѹ�ʽ��֮�����Ʒ����˱仯��ԭʼ�Ĵ���Ҳ�ŵ�����-->��չ��Ϣ
			if(strWord != strOrigKey) { //��ʽ��֮�����˱仯�Ĵ���
				//bDiffWord = true;
				write(iFdOut, &uiExtraPos, 4);
				uiExtraPos += strOrigKey.size() + 1;
				write(iFdOutExtra, strOrigKey.c_str(), strOrigKey.size() + 1);
				uiWordPos += 4;
			}
			//д��չ��Ϣ�����ݣ��ָ�����Ϊ|
			char * m = p;
			while(m[0] != 0 && m[0]!='|') {
				m++;
				if(m[0] & 0x80) m++;
			}
			if(m[0] != 0) { //m=='|'
				m++;
				string strExtra = m;
				unsigned int uiExtraSize = strExtra.size();
				//���һ�����ݵ���չ��Ϣֱ�ӳ���4G��ֱ�Ӻ���
				if(uiExtraPos < uiExtraPos + uiExtraSize) { //û�г���4G
					//if(!bDiffWord) {
						write(iFdOut, &uiExtraPos, 4);
						uiWordPos += 4;
					//}
					write(iFdOutExtra, strExtra.c_str(), uiExtraSize + 1);
					uiExtraPos += uiExtraSize + 1;
				} else {
					printf("error: the extra info is too big(ignore):%s\n", p);
				}
			}
			write(iFdOut, strWord.c_str(), strWord.size() + 1);
			//������ڸ��ǵ���������Ǹ�����û������ģ��Ժ����Ϊ��
			//ר�� Ϊ����ʹ�� ������Ҫһһ��Ӧ����Ϊ������չ 
			m_mapWordPos[strPre + "-" + strWord] = uiWordPos; 
			m_mapWordPos[strWord] = uiWordPos;
			uiWordPos += strWord.size() + 1;
		} else { //�����ƴ������Ҫ�ж��ظ���񣬲�д�ظ������ݣ���ʡ�ڴ�
			if(m_mapWordPos.find(strWord) == m_mapWordPos.end()) {
				write(iFdOut, strWord.c_str(), strWord.size() + 1);
				m_mapWordPos[strWord] = uiWordPos;
				uiWordPos += strWord.size() + 1;
			}
		}
	}
	if(m_iType == EXTRA_INFO_SET_INSIDE) {
		lseek(iFdOutExtra, 0, SEEK_SET);
		write(iFdOutExtra, &uiExtraPos, 4);
		close(iFdOutExtra);
	}
	lseek(iFdOut, 0, SEEK_SET);
	write(iFdOut, &uiWordPos, 4);
	close(iFdOut);
	return 0;
}
