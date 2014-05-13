#ifndef _SYS_CONFIG_SET_H_
#define _SYS_CONFIG_SET_H_
#include "util/XHStrUtils.h"
#include "util/ConfigReader.h"
#include "common/SysCommon.h"

#define G_INT(p) CSysConfigSet::GetInstance().GetInt(PARA_CONFIG_##p)
#define G_STR(p) CSysConfigSet::GetInstance().GetString(PARA_CONFIG_##p)

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
#define PARA_CONFIG_suffix_extra            "suffix_extra"       //扩展的后缀
#define PARA_CONFIG_suffix_treenode         "suffix_treenode"    //树节点的后缀
#define PARA_CONFIG_suffix_nodedata         "suffix_nodedata"    //树节点包含的数据的后缀
#define PARA_CONFIG_suffix_mixsearch        "suffix_mixsearch"   //混合搜索使用
#define PARA_CONFIG_chinesestring           "chinesestring"      //汉字对应的字符串，有汉字字典生成，因此里面有扩展信息的内容
#define PARA_CONFIG_spellstring             "spellstring"        //拼音对应的字符串索引，具体有哪个拼音生成见wordstringinfo
#define PARA_CONFIG_simplestring            "simplestring"       //简拼拼音对应的字符串索引，具体有哪个拼音生成见wordstringinfo


#define PARA_CONFIG_IN_FILE_NAME            "chinese.dict"

#define CENTER_SEARCH_CLOASE   0 //不使用中间查询
#define CENTER_SEARCH_OPEN     1 //使用中间查询
#define CENTER_SEARCH_PERWORD  2 //中间查询使用单字模式

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
	//strKey->要处理的字符串，iType->要输出的类型[0:string; 1:int]
	//strRet->iType 为0时，结果放在这里返回
	//iRet->iType 为1时，结果放在这里返回
	//iFlag->0:如果在map中找不到该词直接退出; 1:直接替换
	//mapData->key,value内容存储
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

		//必须存在
		GetValue(PARA_CONFIG_configurepath, 0, configurepath, iT, 0, mapData);
		GetValue(PARA_CONFIG_pathdata,      0, pathdata,      iT, 0, mapData);
		GetValue(PARA_CONFIG_tasklist,      0, tasklist,      iT, 0, mapData);

		//数字
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

		//字符串
		GetValue(PARA_CONFIG_tipserverlog,  0, tipserverlog,  iT, 1, mapData);
		GetValue(PARA_CONFIG_tipindexlog,   0, tipindexlog,   iT, 1, mapData);
		GetValue(PARA_CONFIG_splitwordpath, 0, splitwordpath, iT, 1, mapData);
		GetValue(PARA_CONFIG_selecttree,    0, selecttree,    iT, 1, mapData);
		return 0;
	};
	void InitDefault() { //设置一些系统的默认值
		centersearchflag = CENTER_SEARCH_OPEN;
		mincentersearchwordlen  = 5; //字节
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
		selecttree    = SELECT_TREE_ALL;  //#具体值见SysCommon.h
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


private: //必须要设置的参数
	string configurepath; //配置文件路径下面必须有文件:pinyin,mpinyin,muticorrectpy
	string pathdata;      //工程路径pathdata+tasklist ; 临时文件的路径 pathdata + "_tmp"
	string tasklist;      //要处理的任务的列表，多个之间使用SPLIT_MULTI_TAKENAME_STR分割

private: //含有默认值的参数 --> 配置类
	int centersearchflag;        //是否使用中间查询/单字查询
	int mincentersearchwordlen;  //中间查询的字符串最短长度为多少
	int mincentersearchlen;      //中间查询的最短字符串的长度
	int maxcentersearchlen;      //中间查询的最长的字符串长度
	int centerchineseMostnumber; //一个词语可以使用中间查询切分成最多多少个词
	int ignorecase;              //忽略大小写;0 不忽略
	int serverport;              //查询服务器的端口号
	int querythreadnum;          //查询服务的线程数
	int useserverlog;            //使用查询日志
	int useindexlog;             //使用索引的日志
	int withmixsearch;           //是否支持混合查询
	int indexport;               //索引的端口号
	
	string tipserverlog;         //查询服务的日志位置
	string tipindexlog;          //索引的日志位置
	string splitwordpath;        //分词的位置，默认为在config下面
	string selecttree;           //要使用的查询树的种类 -->> 特定条件下，这个参数需要特定配置
private: //可以认为固定死，不能修改的
	string filedict[32];
	string fileindex[32];

private: //返回结果使用
	map<string, string> m_mapStrKeyValue;
	map<string, int>    m_mapIntKeyValue;
};

#endif
