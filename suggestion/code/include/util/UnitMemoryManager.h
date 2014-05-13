
#ifndef _UNIT_MEMORY_MANAGER_H
#define _UNIT_MEMORY_MANAGER_H

#ifndef WIN32
#include <pthread.h>
#else
#include <Windows.h>
#endif

#include "util/BaseMemoryManager.h"

/*
	固定大小内存分配方案：输入参数 block数目，每个block中unit数目，unit大小；
	需要注意alloc和free必须成对使用，当前分配后不置空，所以没法判断是否该块内存无用，需要成对使用；

	分配和回收时按照unit大小进行； 相对于BaseMemoryManager是更小的内存分配单元,每次分配固定大小内存;

*/
class CUnitMemoryManager : public CBaseMemoryManager
{
public:
	CUnitMemoryManager();
	virtual ~CUnitMemoryManager();

	int init(int nmax_block_num, int nblock_unit_num, int nunit_size);
	void *alloc();
	void clear();
	//内存返回
	void free(void *pbuf);
	int used_unit_num();

protected:
	int nunit_size_;
	int ncost_unit_;  //使用的内存unit数目
	char *pidle_;     //
	//
#ifndef WIN32
	pthread_mutex_t mutex_;
#else
	CRITICAL_SECTION cri_;
#endif

};

#endif 
