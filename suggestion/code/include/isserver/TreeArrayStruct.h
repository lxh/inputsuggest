//���»�  
//2013-12-27 20:00
//   ��һ���XHMixSearch�����. ������϶���������,�ٶ�Ҳ��Ҫ��һ���Ż�.��������Ԫ��ǰ��ɻ��ƥ�䣨��ƴȫƴ���ĵ�һ��,���ǱȽ�������
//   ��������������ʾ��֮ǰ�İ汾�ı����ĵط�.Ҳ���߼��Ƚϸ��ӵĵط�.
#ifndef _TREE_ARRAY_STRUCT_H_
#define _TREE_ARRAY_STRUCT_H_
#include <queue>
#include<functional>
#include <string>
#include <vector>

using namespace std;
//ֻҪ��������ֹͣ�����ڻ��
#define ADD_NEW_FRANCH2_NEW(p) if(p && !topResult.IsFullQueue()) { \
	if(iPQNums < MAX_PRI_QUEUE_NUMBER) { \
		listNode.push((const void*)p); \
		iPQNums++; \
	} \
}
#define ADD_NEW_FRANCH_NEW(p) if(p && (!topResult.IsFullQueue() ||  topResult.IsFullQueue() && p->usWeight > topResult.GetMinScore())) { \
	if(iPQNums < MAX_PRI_QUEUE_NUMBER) { \
		listNode.push((const void*)p); \
		iPQNums++; \
	} \
}

#define ADD_NEW_FRANCH2(p) if(p && !topResult.IsFullQueue()) { \
	if(iPQNums < MAX_PRI_QUEUE_NUMBER) { \
		stTmp.iWeight = p->usWeight; \
		stTmp.pNode = p; \
		pqRes.push(stTmp); \
		iPQNums++; \
	} \
}


	//if(iPQNums < MAX_PRI_QUEUE_NUMBER) { \
		//iPQNums++; \
	//} \
//>=���ռ���ȫһЩ�������ٶȻ���
		//printf("node weight:%d; son weight:%d; min weight of topresult:%d\n", p->usWeight, pson->usWeight, topResult.GetMinScore()); \
//#define ADD_NEW_FRANCH(p) if(p && (!topResult.IsFullQueue() ||  topResult.IsFullQueue() && p->usWeight >= topResult.GetMinScore())) { 
#define ADD_NEW_FRANCH(pson) if(pson && (!topResult.IsFullQueue() ||  pson->usWeight > topResult.GetMinScore())) { \
		stTmp.iWeight = pson->usWeight; \
		stTmp.pNode = pson; \
		pqRes.push(stTmp); \
}
typedef struct _StForCollect {
	int iWeight;
	void *pTreeArray;
	const XHRbNode * pNode;
	bool operator < (const struct _StForCollect & t) const {
		return iWeight < t.iWeight;
	};
}StForCollect;






#endif
