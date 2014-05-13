
#ifndef _UNIT_MEMORY_MANAGER_H
#define _UNIT_MEMORY_MANAGER_H

#ifndef WIN32
#include <pthread.h>
#else
#include <Windows.h>
#endif

#include "util/BaseMemoryManager.h"

/*
	�̶���С�ڴ���䷽����������� block��Ŀ��ÿ��block��unit��Ŀ��unit��С��
	��Ҫע��alloc��free����ɶ�ʹ�ã���ǰ������ÿգ�����û���ж��Ƿ�ÿ��ڴ����ã���Ҫ�ɶ�ʹ�ã�

	����ͻ���ʱ����unit��С���У� �����BaseMemoryManager�Ǹ�С���ڴ���䵥Ԫ,ÿ�η���̶���С�ڴ�;

*/
class CUnitMemoryManager : public CBaseMemoryManager
{
public:
	CUnitMemoryManager();
	virtual ~CUnitMemoryManager();

	int init(int nmax_block_num, int nblock_unit_num, int nunit_size);
	void *alloc();
	void clear();
	//�ڴ淵��
	void free(void *pbuf);
	int used_unit_num();

protected:
	int nunit_size_;
	int ncost_unit_;  //ʹ�õ��ڴ�unit��Ŀ
	char *pidle_;     //
	//
#ifndef WIN32
	pthread_mutex_t mutex_;
#else
	CRITICAL_SECTION cri_;
#endif

};

#endif 
