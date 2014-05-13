//李新华  
//2013-12-27 20:00
//   第一版的XHMixSearch完成了. 这里面肯定还有问题,速度也需要进一步优化.但是能在元旦前完成混合匹配（简拼全拼）的第一般,算是比较满意了
//   这是整个输入提示与之前的版本改变最大的地方.也是逻辑比较复杂的地方.
#ifndef _TREE_ARRAY_STRUCT_H_
#define _TREE_ARRAY_STRUCT_H_
#include <queue>
#include<functional>
#include <string>
#include <vector>

using namespace std;
//只要个数够就停止，用于混合
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
//>=会收集的全一些，但是速度会慢
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
