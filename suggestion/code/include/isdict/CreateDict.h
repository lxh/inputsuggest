#ifndef _CREATE_DICT_H_
#define _CREATE_DICT_H_
#include <vector>
#include <string>
#include <map>
#include "util/SysConfigSet.h"
#include "util/WriteBigFile.h"

using namespace std;
#define DICT_MAX_SIZE 32

typedef struct _WordToSpell {
	string strWord;
	map<string, vector<string> > mapSimple;
	vector<string> vecFull;
}WordToSpell;

typedef struct _RecordInfo {
	int iWeight;
	vector<string> vecIds; //��Ӧ�ĺ����±�
	vector<string> vecExtra2; //��չ��Ϣ�
}RecordInfo;

class CCreateDict {
public:
	int Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet);

private:
	int Process();
	int CreateAllDict_First();
private:
	int WriteOneDict(const string & strPre, const string & strKey, CWriteBigFile & wFD, const int iWeight, const int iCount, const string & strExtra2 = "");
	void Init();
	void InitParamter();
	void InitLoadSplitProc();
	void SplitWord(const string & strIn, vector<string> &vecOut);
	void SplitWordNoPinyin(const string & strIn, vector<WordToSpell> & vecPinyin);
	void SplitWordWithPinyin(const string & strIn, vector<WordToSpell> & vecPinyin);
	int WriteDictFirst(const string & strPre, const string & strExPre, const string & strKey, CWriteBigFile * & writeFds, const int iWeight, const int iCount);
	int FormatJoinOneFile(const int iFieldId, map<const string, RecordInfo> & mapRecord);
	int WriteJoinOneFile(const int iFieldId, map<const string, RecordInfo> & mapRecord);
	int CreateAllDict_Second();

private:
	CSysConfigSet * m_pSysConfigSet; //�����ļ�����Ϣ
	string          m_strTaskName;   //���������
	string          m_strDataPath;   //���ݵ�·��
	bool            m_bDictNeedCreate[DICT_MAX_SIZE]; //��Ҫ����Ĵʵ�
	bool            m_bNeedPinyinDeal; //�Ƿ���ƴ���Ĵ������û��ƴ������ô�Ͳ���Ҫ���ƴ��)

	//���ڷִʵļ�������
	int  m_iMinSplitCenterWordLen;   //�м��ѯ�Ĵ���̳���Ϊ����
	int  m_iMinSplitCenterSearchLen; //��̵Ŀ��Խ��зִʵĳ���
	int  m_iMaxSplitCenterSearchLen; //��Ŀ��Խ��зִʵĳ���
	int  m_iMaxSplitNum;             //����ֳܷɶ��ٸ���
	string  m_strTmpSuffix; //��ʱ�ļ��ĺ�׺
};

#endif
