#ifndef _SYS_CONFIG_SET_H_
#define _SYS_CONFIG_SET_H_
#include "util/XHStrUtils.h"
#include "util/ConfigReader.h"
#include "common/SysCommon.h"

#define G_INT(p) CSysConfigSet::GetInstance().GetInt(PARA_CONFIG_##p)
#define G_STR(p) CSysConfigSet::GetInstance().GetString(PARA_CONFIG_##p)

#define PARA_CONFIG_addtasksecondname       "addtasksecondname"  //�������ݱ����ڵ�task����ǰ׺
#define PARA_CONFIG_configurepath           "configurepath"
#define PARA_CONFIG_pathdata                "pathdata"
#define PARA_CONFIG_tasklist                "tasklist"
#define PARA_CONFIG_centersearchflag        "centersearchflag"
#define PARA_CONFIG_mincentersearchwordlen  "mincentersearchwordlen"
#define PARA_CONFIG_mincentersearchlen      "mincentersearchlen"
#define PARA_CONFIG_maxcentersearchlen      "maxcentersearchlen"
#define PARA_CONFIG_centerchineseMostnumber "centerchineseMostnumber"
#define PARA_CONFIG_ignorecase              "ignorecase"
#define PARA_CONFIG_serverport              "serverport"
#define PARA_CONFIG_querythreadnum          "querythreadnum"
#define PARA_CONFIG_useserverlog            "useserverlog"
#define PARA_CONFIG_useindexlog             "useindexlog"
#define PARA_CONFIG_withmixsearch           "withmixsearch"
#define PARA_CONFIG_indexport               "indexport"
#define PARA_CONFIG_tipserverlog            "tipserverlog"
#define PARA_CONFIG_tipindexlog             "tipindexlog"
#define PARA_CONFIG_splitwordpath           "splitwordpath"
#define PARA_CONFIG_selecttree              "selecttree"
#define PARA_CONFIG_suffix_extra            "suffix_extra"       //��չ�ĺ�׺
#define PARA_CONFIG_suffix_treenode         "suffix_treenode"    //���ڵ�ĺ�׺
#define PARA_CONFIG_suffix_nodedata         "suffix_nodedata"    //���ڵ���������ݵĺ�׺
#define PARA_CONFIG_suffix_mixsearch        "suffix_mixsearch"   //�������ʹ��
#define PARA_CONFIG_chinesestring           "chinesestring"      //���ֶ�Ӧ���ַ������к����ֵ����ɣ������������չ��Ϣ������
#define PARA_CONFIG_spellstring             "spellstring"        //ƴ����Ӧ���ַ����������������ĸ�ƴ�����ɼ�wordstringinfo
#define PARA_CONFIG_simplestring            "simplestring"       //��ƴƴ����Ӧ���ַ����������������ĸ�ƴ�����ɼ�wordstringinfo


#define PARA_CONFIG_IN_FILE_NAME            "chinese.dict"

#define CENTER_SEARCH_CLOASE   0 //��ʹ���м��ѯ
#define CENTER_SEARCH_OPEN     1 //ʹ���м��ѯ
#define CENTER_SEARCH_PERWORD  2 //�м��ѯʹ�õ���ģʽ

#define SPLIT_MULTI_TAKENAME_STR ";"

class CSysConfigSet {
public:
	CSysConfigSet() {
		InitDefault();
		InitConst();
	};
	int ReadConfig(const string & strCfgFile) {
		return Anly(strCfgFile);
	};

public:
	static CSysConfigSet & GetInstance(const string & strCfg = "") {
		static bool bFirst = true;
		static CSysConfigSet cfg;
		if(bFirst) {
			bFirst = false;
			cfg.ReadConfig(strCfg);
		}
		return cfg;
	};
	//iFlag = 0 >>dict
	//iFlag = 1 >>index
	string GetFileName(const int iSelectTree, const int iFlag = 0) {
		if(iFlag == 0) return filedict[iSelectTree];
		return fileindex[iSelectTree];
	};
	string GetString(const string & strKey) {
		map<string, string>::iterator iIter;
		iIter = m_mapStrKeyValue.find(strKey);
		if(iIter != m_mapStrKeyValue.end()) {
			return iIter->second;
		}
		return "";
	};

	int GetInt(const string & strKey) {
		map<string, int>::iterator iIter;
		iIter = m_mapIntKeyValue.find(strKey);
		if(iIter != m_mapIntKeyValue.end()) {
			return iIter->second;
		}
		return -1;
	};
	int Debug_Print() {
		printf("int---->>\n");
		map<string, int>::iterator iIter;
		for(iIter = m_mapIntKeyValue.begin(); iIter != m_mapIntKeyValue.end(); iIter++) {
			printf("key:%s; value:%d\n", iIter->first.c_str(), iIter->second);
		}
		printf("int---->>\n");
		map<string, string>::iterator iIterSS;
		for(iIterSS = m_mapStrKeyValue.begin(); iIterSS != m_mapStrKeyValue.end(); iIterSS++) {
			printf("key:%s; value:%s\n", iIterSS->first.c_str(), iIterSS->second.c_str());
		}
		printf("test debug over\n");
	};
private:
	//strKey->Ҫ������ַ�����iType->Ҫ���������[0:string; 1:int]
	//strRet->iType Ϊ0ʱ������������ﷵ��
	//iRet->iType Ϊ1ʱ������������ﷵ��
	//iFlag->0:�����map���Ҳ����ô�ֱ���˳�; 1:ֱ���滻
	//mapData->key,value���ݴ洢
	bool GetValue(string strKey, int iType, string& strRet, int & iRet, int iFlag, map<string, string> & mapData) {
		map<string, string>::iterator iIter;
		iIter = mapData.find(strKey);
		if(iIter == mapData.end()) {
			if(iFlag == 0) {
				printf("error: the configure not found(%s), must have\n", strKey.c_str());
				exit(-1);
			}
			//return false;
		} else {
			strRet = iIter->second;
			iRet = atoi(iIter->second.c_str());
		}
		if(iType == 0) {
			m_mapStrKeyValue[strKey] = strRet;
		} else {
			m_mapIntKeyValue[strKey] = iRet;
		}
		return true;
	};
	int Anly(const string & strCfgFile) {
		map<string, string> mapData;
		ConfigReader(strCfgFile, mapData);
		int iT;
		string strT;

		//�������
		GetValue(PARA_CONFIG_configurepath, 0, configurepath, iT, 0, mapData);
		GetValue(PARA_CONFIG_pathdata,      0, pathdata,      iT, 0, mapData);
		GetValue(PARA_CONFIG_tasklist,      0, tasklist,      iT, 0, mapData);

		//����
		GetValue(PARA_CONFIG_centersearchflag,        1, strT, centersearchflag,        1, mapData);
		GetValue(PARA_CONFIG_mincentersearchwordlen,  1, strT, mincentersearchwordlen,  1, mapData);
		GetValue(PARA_CONFIG_mincentersearchlen,      1, strT, mincentersearchlen,      1, mapData);
		GetValue(PARA_CONFIG_maxcentersearchlen,      1, strT, maxcentersearchlen,      1, mapData);
		GetValue(PARA_CONFIG_centerchineseMostnumber, 1, strT, centerchineseMostnumber, 1, mapData);
		GetValue(PARA_CONFIG_ignorecase,     1, strT, ignorecase,     1, mapData);
		GetValue(PARA_CONFIG_serverport,     1, strT, serverport,     1, mapData);
		GetValue(PARA_CONFIG_querythreadnum, 1, strT, querythreadnum, 1, mapData);
		GetValue(PARA_CONFIG_useserverlog,   1, strT, useserverlog,   1, mapData);
		GetValue(PARA_CONFIG_useindexlog,    1, strT, useindexlog,    1, mapData);
		GetValue(PARA_CONFIG_withmixsearch,  1, strT, withmixsearch,  1, mapData);
		GetValue(PARA_CONFIG_indexport,      1, strT, indexport,      1, mapData);

		//�ַ���
		GetValue(PARA_CONFIG_tipserverlog,      0, tipserverlog,      iT, 1, mapData);
		GetValue(PARA_CONFIG_tipindexlog,       0, tipindexlog,       iT, 1, mapData);
		GetValue(PARA_CONFIG_splitwordpath,     0, splitwordpath,     iT, 1, mapData);
		GetValue(PARA_CONFIG_selecttree,        0, selecttree,        iT, 1, mapData);
		GetValue(PARA_CONFIG_addtasksecondname, 0, addtasksecondname, iT, 1, mapData);
		return 0;
	};
	void InitDefault() { //����һЩϵͳ��Ĭ��ֵ
		centersearchflag = CENTER_SEARCH_OPEN;
		mincentersearchwordlen  = 5; //�ֽ�
		mincentersearchlen      = 8;
		maxcentersearchlen      = 30;
		centerchineseMostnumber = 5;
		ignorecase = 0;
		serverport = 31415;
		querythreadnum = 1;
		useserverlog   = 0;
		useindexlog    = 1;
		withmixsearch  = 1;
		indexport      = 31416;
                      
		tipserverlog  = "tipout.log";
		tipindexlog   = "indexout.log";
		splitwordpath = "dictdir";
		selecttree    = SELECT_TREE_ALL;  //#����ֵ��SysCommon.h
		addtasksecondname = "xuan";
	};
	void InitConst() {
		int iLoop;
		for(int iLoop = 0; iLoop < sizeof(filedict) / sizeof(filedict[0]); iLoop++) {
			filedict[iLoop] = "";
			fileindex[iLoop] = "";
		}
		vector<string> vecSplit;
		XHStrUtils::StrTokenize(vecSplit, SELECT_TREE_ALL, ";");
		for(iLoop = 0; iLoop < vecSplit.size(); iLoop++) {
			string strTmp = vecSplit[iLoop];
			if(strTmp.size() == 1) {
				filedict[atoi(strTmp.c_str())] = "dict.0" + strTmp;
				fileindex[atoi(strTmp.c_str())] = "index.0" + strTmp;
			} else {
				filedict[atoi(strTmp.c_str())] = "dict." + strTmp;
				fileindex[atoi(strTmp.c_str())] = "index." + strTmp;
			}
		}
		m_mapStrKeyValue[PARA_CONFIG_chinesestring]    = "chinesestring";
		m_mapStrKeyValue[PARA_CONFIG_spellstring]      = "spellstring";
		m_mapStrKeyValue[PARA_CONFIG_simplestring]     = "simplestring";
		m_mapStrKeyValue[PARA_CONFIG_suffix_extra]     = ".extra";
		m_mapStrKeyValue[PARA_CONFIG_suffix_treenode]  = ".treenode";
		m_mapStrKeyValue[PARA_CONFIG_suffix_nodedata]  = ".nodedata";
		m_mapStrKeyValue[PARA_CONFIG_suffix_mixsearch] = ".mixsearch";
	};


private: //����Ҫ���õĲ���
	string configurepath; //�����ļ�·������������ļ�:pinyin,mpinyin,muticorrectpy
	string pathdata;      //����·��pathdata+tasklist ; ��ʱ�ļ���·�� pathdata + "_tmp"
	string tasklist;      //Ҫ�����������б����֮��ʹ��SPLIT_MULTI_TAKENAME_STR�ָ�

private: //����Ĭ��ֵ�Ĳ��� --> ������
	int centersearchflag;        //�Ƿ�ʹ���м��ѯ/���ֲ�ѯ
	int mincentersearchwordlen;  //�м��ѯ���ַ�����̳���Ϊ����
	int mincentersearchlen;      //�м��ѯ������ַ����ĳ���
	int maxcentersearchlen;      //�м��ѯ������ַ�������
	int centerchineseMostnumber; //һ���������ʹ���м��ѯ�зֳ������ٸ���
	int ignorecase;              //���Դ�Сд;0 ������
	int serverport;              //��ѯ�������Ķ˿ں�
	int querythreadnum;          //��ѯ������߳���
	int useserverlog;            //ʹ�ò�ѯ��־
	int useindexlog;             //ʹ����������־
	int withmixsearch;           //�Ƿ�֧�ֻ�ϲ�ѯ
	int indexport;               //�����Ķ˿ں�
	
	string addtasksecondname;    //�洢һ��������������ݵ������ǰ׺
	string tipserverlog;         //��ѯ�������־λ��
	string tipindexlog;          //��������־λ��
	string splitwordpath;        //�ִʵ�λ�ã�Ĭ��Ϊ��config����
	string selecttree;           //Ҫʹ�õĲ�ѯ�������� -->> �ض������£����������Ҫ�ض�����
private: //������Ϊ�̶����������޸ĵ�
	string filedict[32];
	string fileindex[32];

private: //���ؽ��ʹ��
	map<string, string> m_mapStrKeyValue;
	map<string, int>    m_mapIntKeyValue;
};

#endif
