
#ifndef _BASE_MEMORY_MANAGER_H
#define _BASE_MEMORY_MANAGER_H

#ifndef WIN32
#include <pthread.h>
#else
#include <Windows.h>
#endif
#include <string.h>

/*
	���ڴ���Ƶ��new��delete�����ڴ���Ƭ���⣬ͨ��ʹ�ù̶���С�ڴ���з���ͻ��գ�
	����ʹ��ϵͳ��new��delete���У�����������Դ���һ�����һЩ���⣻
	ע�ⲻҪ������ʼ����С���ƣ�ʹ�ø���ǰ��Ҫ����ʵ��ʹ�ô�С�������򲻽��з��أ�

	ϵͳ�����𲽷���ķ�ʽ������һ�ξͷ�����ɣ�����Ҫ��ʱ��Ž��з��䣻
	ֻ�ʺ����ڷ����ڴ棬����û����ȫʵ��;
	�����ڴ����С���ڴ���С;(�ʺ���Ƶ�����з���ͻ����ڴ�)
*/
class CBaseMemoryManager
{
public:
	CBaseMemoryManager();
	virtual ~CBaseMemoryManager();

	int init(int nmax_block_num, int nblock_size, bool bsingle = false);
	//����ʱ���г�ʼ���ڴ棬û���㹻�ڴ淵��NULL�����򷵻��ڴ��ַ
	void *alloc(int nsize);
	void clear();
	//���߳�ʱ��Ч���ұ������һ��alloc()���ö�Ӧ; 
	int back(int nsize);

protected:
	bool bsingle_;         //
	int nmax_block_num_;   //�����������
	int nblock_offset_;    //��ǰʹ�ÿ�λ��
	int nblock_size_;      //ÿ���ڴ��С
	int noffset_;		   //ʹ�ÿ���ڴ�λ��	
	char **ppblock_list_;  //
	//
#ifndef WIN32
	pthread_mutex_t mutex_;
#else
	CRITICAL_SECTION cri_;
#endif

};

#endif 
