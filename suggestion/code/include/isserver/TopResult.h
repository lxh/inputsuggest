#ifndef _COMPRO_TOPRESULT_H
#define _COMPRO_TOPRESULT_H

#include <string>
#include <vector>
#include <set>

#include "util/PriorityQueue.h"
//�������ﲻ��,��ʱ��
#include "isserver/XHParaCategory.h"

using namespace std;

/*
	ID: resultNum : {elem}  ��multi-result struct; get data accord ID��
	elem: ID, queryNum, data;  (spell etc process)

	����ID��ȡ��ͬ���ݽṹ;����ǰ�õ�ԭʼ����(һ��������ȡ�����ͬʱ���ؽ���������ڳ���ʱ��ֹ);
*/

class ResultElement
{
public:
	ResultElement(int id, int num, int len, void * pParaIn = NULL)
	{
		eleId = id;
		queryNum = num;
		length = len;
		pPara = pParaIn;
	}
	~ResultElement()
	{
	}
	int queryNum;
	int length;
	int eleId;
	const void *pPara;
};

class TopTipQueue : public CPriorityQueue<ResultElement*>
{
public:
	TopTipQueue()
	{
	}
	TopTipQueue(size_t maxSize)
  {   
     initialize(maxSize,true);
  }   
  virtual~TopTipQueue() {}

	virtual void initial(size_t maxSize)
    {   
        initialize(maxSize,true);
    } 

protected:
	
	virtual bool lessThan(ResultElement* pDoc1,ResultElement* pDoc2)
    {   
      if (pDoc1->queryNum == pDoc2->queryNum)
      {   
        if ( pDoc1->length == pDoc2->length)
          return pDoc1->eleId < pDoc2->eleId;
              else
          return pDoc1->length > pDoc2->length;
      }   
      else
        return pDoc1->queryNum < pDoc2->queryNum;
    }  

};

/*
*/
class TopResult
{
public:
	TopResult(int maxSize)
	{
		m_maxsize = maxSize;
		tipqueue.initial(maxSize);
		totalHit = 0;
		minScore = 0;
	}
	~TopResult()
	{
	}

  inline bool isInQueue(int elemId)
  {
    if(queueSet.count(elemId))
    {
      return true;
    }
    else
    {
      queueSet.insert(elemId);
      return false;
    }
  }

	//���Խ�������
	inline int collect(int elemId, int queryNum, int elemLen, void * pTree = NULL)
	{
		if ((int)tipqueue.size() < m_maxsize || queryNum > minScore)
		{
			if(isInQueue(elemId)) {
				return 0;
			}
			ResultElement *elem = new ResultElement(elemId,queryNum,elemLen, pTree);
			if(!tipqueue.insert(elem))
			{
				delete elem;
			}
		}
		else
		{
		}
		totalHit++;
		return 0;
	}

	void setTotal(int total)
	{
		totalHit = total;
	}
	int getSavedSize() {
		return tipqueue.size();
	};
	int getTotal()
	{
		return totalHit;
	}
	//add by lxh
	int IsFullQueue()
	{
		return tipqueue.IsFull();
	}
	int GetMinScore()
	{
		ResultElement * p = tipqueue.top();
		if(!p) {
			return 0;
		} else {
			return p->queryNum;
		}
	}
	void SetExtraP(const void *pPara) {
		int iLoop;
		int iNum;
		iNum = getSavedSize();
		for(iLoop = 1; iLoop <= iNum; iLoop++) {
			ResultElement *pElem = tipqueue.m_heap[iLoop];
			if(pElem) {
				if(!pElem->pPara) {
					pElem->pPara = pPara;
				}
			}
		}
	}
	void AddWeight(int iAdd) {
		int iLoop;
		int iNum;
		iNum = getSavedSize();
		for(iLoop = 1; iLoop <= iNum; iLoop++) {
			ResultElement *pElem = tipqueue.m_heap[iLoop];
			if(pElem) {
				pElem->queryNum += iAdd;
			}
		}
	};
	int GetMaxSize() {
		return m_maxsize;
	};

	TopTipQueue tipqueue;
	FlagSet *pFlagSet;
private:
	int m_maxsize;
	int totalHit;
	int minScore;
	set<int> queueSet;
};

#endif
