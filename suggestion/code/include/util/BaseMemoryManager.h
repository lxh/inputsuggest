
#ifndef _BASE_MEMORY_MANAGER_H
#define _BASE_MEMORY_MANAGER_H

#ifndef WIN32
#include <pthread.h>
#else
#include <Windows.h>
#endif
#include <string.h>

/*
	用于处理频繁new和delete引起内存碎片问题，通过使用固定大小内存进行分配和回收；
	避免使用系统的new和delete进行，不仅消耗资源而且会引起一些问题；
	注意不要超过初始化大小限制，使用该类前需要估计实际使用大小；（否则不进行返回）

	系统采用逐步分配的方式，不是一次就分配完成，当需要的时候才进行分配；
	只适合用于分配内存，回收没有完全实现;
	分配内存必须小于内存块大小;(适合于频繁进行分配和回收内存)
*/
class CBaseMemoryManager
{
public:
	CBaseMemoryManager();
	virtual ~CBaseMemoryManager();

	int init(int nmax_block_num, int nblock_size, bool bsingle = false);
	//分配时进行初始化内存，没有足够内存返回NULL，否则返回内存地址
	void *alloc(int nsize);
	void clear();
	//单线程时有效，且必须和上一次alloc()调用对应; 
	int back(int nsize);

protected:
	bool bsingle_;         //
	int nmax_block_num_;   //分配的最大块数
	int nblock_offset_;    //当前使用块位置
	int nblock_size_;      //每块内存大小
	int noffset_;		   //使用块的内存位置	
	char **ppblock_list_;  //
	//
#ifndef WIN32
	pthread_mutex_t mutex_;
#else
	CRITICAL_SECTION cri_;
#endif

};

#endif 
