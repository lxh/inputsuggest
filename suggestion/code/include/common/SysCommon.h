#ifndef _SYS_COMMON_H_
#define _SYS_COMMON_H_
/*
	定义一些系统的全局要使用的宏
*/

//定义总共有多少种树的结构
#define SELECT_TREE_CHINESE              0 // (1 <<  0)
#define SELECT_TREE_SPELL                1 // (1 <<  1)
#define SELECT_TREE_SIMPLE               2 // (1 <<  2)
#define SELECT_TREE_CENTER_CHINESE       6 // (1 <<  6)
#define SELECT_TREE_CENTER_SPELL         7 // (1 <<  7)
#define SELECT_TREE_CENTER_SIMPLE        8 // (1 <<  8)
#define SELECT_TREE_EX_CHINESE          20 // (1 << 20)
#define SELECT_TREE_EX_SPELL            21 // (1 << 21)
#define SELECT_TREE_EX_SIMPLE           22 // (1 << 22)
#define SELECT_TREE_EX_CENTER_CHINESE   26 // (1 << 26)
#define SELECT_TREE_EX_CENTER_SPELL     27 // (1 << 27)
#define SELECT_TREE_EX_CENTER_SIMPLE    28 // (1 << 28)

#define ALL_SELECT_SIMPLE_TREE(tt) (tt == SELECT_TREE_SIMPLE || tt == SELECT_TREE_CENTER_SIMPLE || tt == SELECT_TREE_EX_SIMPLE || tt == SELECT_TREE_EX_CENTER_SIMPLE)
#define ALL_SELECT_SPELL_TREE (SELECT_TREE_SPELL | SELECT_TREE_CENTER_SPELL | SELECT_TREE_EX_SPELL | SELECT_TREE_EX_CENTER_SPELL)
#define WITH_PINYIN_TREE(tt) (tt==SELECT_TREE_SPELL||tt==SELECT_TREE_SIMPLE||tt==SELECT_TREE_CENTER_SPELL||tt==SELECT_TREE_CENTER_SIMPLE||tt==SELECT_TREE_EX_SPELL||tt==SELECT_TREE_EX_SIMPLE||tt==SELECT_TREE_EX_CENTER_SPELL||tt==SELECT_TREE_EX_CENTER_SIMPLE)

#define SELECT_TREE_ALL "0;1;2;6;7;8;20;21;22;26;27;28"

#define DICT_MAX_SIZE 32

//混合搜索最多查询多少次
#define MAX_MIX_SEARCH_TIME 50

//并且还限制了对于扩展时(找上级节点)的上级个数[因为上级个数太多,会影响到速度]
#define NUMBER_MAX_RETURN_LIMIT  1000 //返回结果的个数上限
#define NUMBER_RETURN_DEFAULT    10  //默认返回多少条结果

#define MAX_PRI_QUEUE_NUMBER  100000 //优先级队列的最大容量

#define MAX_TRAVERSING_NUMBER_WITH_NULL_PRE_NUMBER  500 //遍历查找结果,在前缀有部分模糊匹配情况下,最多收集多少个节点(直接影响速度)

//CompressPinyin使用
//ALL_OVER_CHAR 最终结束符号
//SPLIT_MUL_STR 同一个节点压缩了多个字母时,多个字母之间的分隔符号
#define ALL_OVER_CHAR 0xff
#define SPLIT_MUL_STR 0xfe

#define SAFE_DELETE_MUL(p) do { \
	if(p) { \
		delete []p; \
		p = NULL; \
	} \
}while(0);

#define SAFE_DELETE_SIG(p) do { \
	if(p) { \
		delete p; \
		p = NULL; \
	} \
} while(0);

#define PRINT_T(str) 
#define PRINT_T2(str) { \
	char szTime[128]; \
	timeval startt; \
	gettimeofday(&startt,0); \
	long lTime = (startt.tv_sec * 1000000 + startt.tv_usec); \
	snprintf(szTime, sizeof(szTime), "%s", str); \
	memset(szTime + strlen(str), '-', 50 - strlen(szTime)); \
	snprintf(szTime + 40, sizeof(szTime) - 40, ">>>>%ld", lTime); \
	printf("%s\n", szTime); \
}

#endif
