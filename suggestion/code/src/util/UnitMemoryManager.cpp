
#define  NO_NEED_LOCK 1
#include "util/UnitMemoryManager.h"

CUnitMemoryManager::CUnitMemoryManager():nunit_size_(0),
										ncost_unit_(0),
										pidle_(NULL)
{
#if NEED_LOCK
#ifndef WIN32
	pthread_mutex_init(&mutex_, NULL);
#else
	InitializeCriticalSection(&cri_);
#endif
#endif
}

CUnitMemoryManager::~CUnitMemoryManager()
{
#if NEED_LOCK
#ifndef WIN32
	pthread_mutex_destroy(&mutex_);
#else
	DeleteCriticalSection(&cri_);
#endif
#endif
}

int CUnitMemoryManager::init(int nmax_block_num, int nblock_unit_num, int nunit_size)
{
	if(CBaseMemoryManager::init(nmax_block_num, nblock_unit_num*nunit_size))
		return -1;

	nunit_size_ = nunit_size;
	ncost_unit_ = 0;
	pidle_ = NULL;

	return 0;
}

void *CUnitMemoryManager::alloc()
{
	void *p = NULL;

#if NEED_LOCK
#ifndef WIN32
	pthread_mutex_lock(&mutex_);
#else
	EnterCriticalSection(&cri_);
#endif
#endif
	//
	if(pidle_)
	{
		p = (void *)pidle_;
		pidle_ = *(char **)pidle_;
		ncost_unit_++;
		//
#if NEED_LOCK
#ifndef WIN32
		pthread_mutex_unlock(&mutex_);
#else
		LeaveCriticalSection(&cri_);
#endif
#endif
		//
		return p;
	}
	if(p = CBaseMemoryManager::alloc(nunit_size_))
		ncost_unit_++;
	//
#if NEED_LOCK
#ifndef WIN32
	pthread_mutex_unlock(&mutex_);
#else
	LeaveCriticalSection(&cri_);
#endif
#endif
	//
	return p;
}

void CUnitMemoryManager::clear()
{
#if NEED_LOCK
#ifndef WIN32
	pthread_mutex_lock(&mutex_);
#else
	EnterCriticalSection(&cri_);
#endif
#endif

	pidle_ = NULL;
	ncost_unit_ = 0;
	CBaseMemoryManager::clear();

#if NEED_LOCK
#ifndef WIN32
	pthread_mutex_unlock(&mutex_);
#else
	LeaveCriticalSection(&cri_);
#endif
#endif
}

void CUnitMemoryManager::free(void *pbuf)
{
#if NEED_LOCK
#ifndef WIN32
	pthread_mutex_lock(&mutex_);
#else
	EnterCriticalSection(&cri_);
#endif
#endif

	*(char **)pbuf = pidle_;
	pidle_ = (char *)pbuf;
	ncost_unit_--;

#if NEED_LOCK
#ifndef WIN32
	pthread_mutex_unlock(&mutex_);
#else
	LeaveCriticalSection(&cri_);
#endif
#endif
}

int CUnitMemoryManager::used_unit_num()
{
	return ncost_unit_;
}
