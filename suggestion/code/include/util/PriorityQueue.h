#ifndef _PRIORITYQUEUE_H
#define _PRIORITYQUEUE_H

#ifdef WIN32
typedef long        int32_t;
typedef unsigned long   uint32_t;
#else
typedef int         int32_t;
typedef unsigned int  uint32_t;
#endif

/**
 * A CPriorityQueue maintains a partial ordering of its elements such that the
 * least element can always be found in constant time.  Put()'s and pop()'s
 *  require log(size) time.
 */
template <class _type>
class CPriorityQueue
{
public:
    _type*	m_heap;
private:
    size_t	m_size;
    bool	m_bDelete;        //used to indicate if delete the element by PriorityQueue
    size_t	m_maxSize;

    void upHeap()
    {
        size_t i = m_size;
        _type node = m_heap[i];			  // save bottom node (WAS object)
        int32_t j = ((uint32_t)i) >> 1;
        while (j > 0 && lessThan(node,m_heap[j]))
        {
            m_heap[i] = m_heap[j];			  // shift parents down
            i = j;
            j = ((uint32_t)j) >> 1;
        }
        m_heap[i] = node;				  // install saved node
    }

    void downHeap()
    {
        size_t i = 1;
        _type node = m_heap[i];			  // save top node
        size_t j = i << 1;				  // find smaller child
        size_t k = j + 1;
        if (k <= m_size && lessThan(m_heap[k], m_heap[j]))
        {
            j = k;
        }
        while (j <= m_size && lessThan(m_heap[j],node))
        {
            m_heap[i] = m_heap[j];			  // shift up child
            i = j;
            j = i << 1;
            k = j + 1;
            if (k <= m_size && lessThan(m_heap[k], m_heap[j]))
            {
                j = k;
            }
        }
        m_heap[i] = node;				  // install saved node
    }

protected:
    CPriorityQueue()
    {
        m_size = 0;
        m_bDelete = false;
        m_heap = NULL;
        m_maxSize = 0;
    }

    /**
     * Determines the ordering of objects in this priority queue.
     * Subclassesmust define this one method.
     */
    virtual bool lessThan(_type a, _type b)=0;

    /**
     * Subclass constructors must call this.
     */
    void initialize(const size_t maxSize, bool deleteOnClear)
    {
        m_size = 0;
        m_bDelete = deleteOnClear;
        m_maxSize = maxSize;
        size_t heapSize = m_maxSize + 1;
        m_heap = new _type[heapSize];
    }

public:
    virtual ~CPriorityQueue()
    {
        clear();
        delete[] m_heap;
    }

    /**
     * Adds an Object to a CPriorityQueue in log(size) time.
     * If one tries to add more objects than m_maxSize from initialize
     * a RuntimeException (ArrayIndexOutOfBound) is thrown.
     */
    void put(_type element)
    {
        if (m_size >= m_maxSize)
            exit(1);
            //LogApp::getSelf().writeLog(LOG_LEVEL_INFO, "PriorityQueue.h put, %s. ", "add is out of bounds");

        m_size++;
        m_heap[m_size] = element;
        upHeap();
    }

    /**
     * Adds element to the CPriorityQueue in log(size) time if either
     * the CPriorityQueue is not full, or not lessThan(element, top()).
     * @param element
     * @return true if element is added, false otherwise.
     */
    bool insert(_type element)
    {
        if (m_size < m_maxSize)
        {
            put(element);
            return true;
        }
        else if (m_size > 0 && !lessThan(element, top()))
        {
            if ( m_bDelete )
            {
                delete m_heap[1];
            }
            m_heap[1] = element;
            adjustTop();
            return true;
        }
        else
            return false;
    }

    /**
     * Returns the least element of the CPriorityQueue in constant time.
     */
    _type top()
    {
        if (m_size > 0)
            return m_heap[1];
        else
            return NULL;
    }

    /**
     * Removes and returns the least element of the CPriorityQueue in log(size) time.
     */
    _type pop()
    {
        if (m_size > 0)
        {
            _type result = m_heap[1];			  // save first value
            m_heap[1] = m_heap[m_size];			  // move last to first

            m_heap[m_size] = (_type)0;			  // permit GC of objects
            m_size--;
            downHeap();				  // adjust m_heap
            return result;
        }
        else
            return (_type)NULL;
    }

    /**
     * Should be called when the object at top changes values.  Still log(n)
     * worst case, but it's at least twice as fast to <pre>
     * { pq.top().change(); pq.adjustTop(); }
     * </pre> instead of <pre>
     * { o = pq.pop(); o.change(); pq.push(o); }
     * </pre>
     */
    void adjustTop()
    {
        downHeap();
    }


    /**
     * Returns the number of elements currently stored in the CPriorityQueue.
     */
    size_t size()
    {
        return m_size;
    }

    /**
     * Removes all entries from the CPriorityQueue.
     */
    void clear()
    {
        for (size_t i = 1; i <= m_size; i++)
        {
            if ( m_bDelete )
            {
                delete m_heap[i];
            }
        }
        m_size = 0;
    }
    //add by lxh
    bool IsFull()
    {
        return m_maxSize == m_size;
    }
};


#endif
