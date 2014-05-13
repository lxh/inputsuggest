
#include "util/BaseMemoryManager.h"

CBaseMemoryManager::CBaseMemoryManager():nmax_block_num_(0),
										nblock_offset_(0),
										nblock_size_(0),
										noffset_(0),
										ppblock_list_(NULL),
										bsingle_(false)
{
#ifndef WIN32
	pthread_mutex_init(&mutex_, NULL);
#else
	InitializeCriticalSection(&cri_);
#endif
}

CBaseMemoryManager::~CBaseMemoryManager()
{
	int i;

	if(ppblock_list_)
	{
		for(i = 0; i < nmax_block_num_; i++)
		{
			if(ppblock_list_[i])
				delete ppblock_list_[i];
			else
				break;
		}
		delete ppblock_list_;
	}
	//
#ifndef WIN32
	pthread_mutex_destroy(&mutex_);
#else
	DeleteCriticalSection(&cri_);
#endif
}

int CBaseMemoryManager::init(int nmax_block_num, int nblock_size, bool bsingle)
{
	if(nmax_block_num <= 0 || nblock_size <= 0)
		return -1;

	int i;
	//已经有值
	if(ppblock_list_)
	{
		for(i = 0; i < nmax_block_num_; i++)
		{
			if(ppblock_list_[i])
				delete ppblock_list_[i];
			else
				break;
		}
		delete ppblock_list_;
	}
	//初始化
	ppblock_list_ = new char*[nmax_block_num];
	memset(ppblock_list_, 0, sizeof(char *)*nmax_block_num);
	ppblock_list_[0] = new char[nblock_size];
	nmax_block_num_ = nmax_block_num;
	nblock_offset_ = 0;
	nblock_size_ = nblock_size;
	noffset_ = 0;
	bsingle_ = bsingle;

	return 0;
}

void *CBaseMemoryManager::alloc(int nsize)
{
	if(nblock_offset_ >= nmax_block_num_)
		return NULL;

	char *p = NULL;

#ifndef WIN32
	pthread_mutex_lock(&mutex_);
#else
	EnterCriticalSection(&cri_);
#endif

	if(noffset_ + nsize > nblock_size_)
	{
		noffset_ = 0;
		nblock_offset_++;
		if(nblock_offset_ >= nmax_block_num_)
		{
#ifndef WIN32
			pthread_mutex_unlock(&mutex_);
#else
			LeaveCriticalSection(&cri_);
#endif

			return NULL;
		}
		if(!ppblock_list_[nblock_offset_])
			ppblock_list_[nblock_offset_] = new char[nblock_size_];
	}
	p = ppblock_list_[nblock_offset_] + noffset_;
	noffset_ += nsize;

#ifndef WIN32
	pthread_mutex_unlock(&mutex_);
#else
	LeaveCriticalSection(&cri_);
#endif

	return (void *)p;
}

int CBaseMemoryManager::back(int nsize)
{
	if(bsingle_)
	{
		if(nblock_offset_ >= nmax_block_num_)
			return -1;

		noffset_ -= nsize;

		return 0;
	}

	return -1;
}

void CBaseMemoryManager::clear()
{
#ifndef WIN32
	pthread_mutex_lock(&mutex_);
#else
	EnterCriticalSection(&cri_);
#endif

	nblock_offset_ = 0;
	noffset_ = 0;

#ifndef WIN32
	pthread_mutex_unlock(&mutex_);
#else
	LeaveCriticalSection(&cri_);
#endif
}
