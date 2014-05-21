#ifndef _SYS_COMMON_H_
#define _SYS_COMMON_H_
/*
	����һЩϵͳ��ȫ��Ҫʹ�õĺ�
*/

//�����ܹ��ж��������Ľṹ
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

//�����������ѯ���ٴ�
#define MAX_MIX_SEARCH_TIME 50

//���һ������˶�����չʱ(���ϼ��ڵ�)���ϼ�����[��Ϊ�ϼ�����̫��,��Ӱ�쵽�ٶ�]
#define NUMBER_MAX_RETURN_LIMIT  1000 //���ؽ���ĸ�������
#define NUMBER_RETURN_DEFAULT    10  //Ĭ�Ϸ��ض��������

#define MAX_PRI_QUEUE_NUMBER  100000 //���ȼ����е��������

#define MAX_TRAVERSING_NUMBER_WITH_NULL_PRE_NUMBER  500 //�������ҽ��,��ǰ׺�в���ģ��ƥ�������,����ռ����ٸ��ڵ�(ֱ��Ӱ���ٶ�)

//CompressPinyinʹ��
//ALL_OVER_CHAR ���ս�������
//SPLIT_MUL_STR ͬһ���ڵ�ѹ���˶����ĸʱ,�����ĸ֮��ķָ�����
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
