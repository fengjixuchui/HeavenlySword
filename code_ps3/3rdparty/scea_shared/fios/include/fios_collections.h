/**
	\file fios_collections.h

	Definitions for the collection classes in the fios::collections namespace.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_collections
#define _H_fios_collections

#include "fios_types.h"
#include "fios_base.h"
#include "fios_platform.h"

/**
* \internal
* @{
*/

namespace fios {
	namespace collections {

/** \brief Simple singly-linked list class. */
template <class T> class FIOS_EXPORT list
{
public:
	list<T>() : m_pListHead(NULL) {}
	list<T>(T pHead) : m_pListHead(pHead) {}
	void push(T pNewHead) {
		if (pNewHead == NULL) return;
		pNewHead->m_pNext = m_pListHead;
		m_pListHead = pNewHead;
	}
	T pop() {
		T pOut = m_pListHead;
		if (pOut != NULL) {
			m_pListHead = pOut->m_pNext;
			pOut->m_pNext = NULL;
		}
		return pOut;
	}
	T steal() {
		T pOut = m_pListHead;
		m_pListHead = NULL;
		return pOut;
	}
	size_t count() {
		size_t c = 0;
		T pWalk = m_pListHead;
		while (pWalk != NULL) {
			c++;
			pWalk = pWalk->m_pNext;
		}
		return c;
	}
protected:
	T & find(T pItem) {
		T *pOut = &m_pListHead;
		while (((*pOut) != pItem) && ((*pOut) != NULL)) {
			pOut = &(*pOut)->m_pNext;
		}
		return (*pOut);
	}
public:
	bool remove(T pItem) {
		if (pItem == NULL) return false;
		T &pFound = find(pItem);
		if (pFound) {
			pFound = pItem->m_pNext;
			pItem->m_pNext = NULL;
			return true;
		} else {
			return false;
		}
	}
	bool contains(T pItem) {
		if (pItem == NULL) return false;
		return find(pItem) == pItem;
	}
	void pushLast(T pItem) { // expensive, requires walking to end of list
		if (pItem == NULL) return;
		(find(NULL)) = pItem;
	}
	void unsteal(T pRestore) {
		pushLast(pRestore);
	}
	bool isEmpty() const {
		T pHead = m_pListHead;
		return (pHead == NULL);
	}
	const T & head() const {
		return m_pListHead;
	}
	T & head() {
		return m_pListHead;
	}
	
protected:
	T m_pListHead;
};

/** \brief Atomic singly-linked list class. */
template <class T> class FIOS_EXPORT atomicList
{
public:
	inline atomicList<T>() {}
	void push(T pNewHead) {
		if (pNewHead) do {
			pNewHead->m_pNext = m_pListHead;
		} while (!m_pListHead.compareAndSwap(pNewHead->m_pNext,pNewHead));
	}
	T pop() {
		T pOut;
		do {
			pOut = m_pListHead;
		} while ((pOut != NULL) && !m_pListHead.compareAndSwap(pOut,pOut->m_pNext));
		if (pOut != NULL) pOut->m_pNext = NULL;
		return pOut;
	}
	T steal() {
		T pOut;
		do {
			pOut = m_pListHead;
		} while (!m_pListHead.compareAndSwap(pOut,NULL));
		return pOut;
	}
	void pushLast(T pItem) {
		if (pItem == NULL) return;
		// pRestore:     D C B A
		// m_pListHead:  F E
		// end result:   F E D C B A
		while (!m_pListHead.compareAndSwap(NULL,pItem))
		{
			T pHead = steal();
			T &pTail = pHead;
			while (pTail != NULL)
				pTail = pTail->m_pNext;
			pTail = pItem;
			pItem = pHead;
		}
	}
	void unsteal(T pRestore) {
		pushLast(pRestore);
	}

	bool isEmpty() const {
		T pHead = m_pListHead;
		return (pHead == NULL);
	}

protected:
	fios::platform::atomicPtr<T> m_pListHead;
};

/** \brief Specialized hashtable used by the catalog cache. */
class FIOS_EXPORT hashtable
{
public:
	explicit hashtable(FIOS_ALLOCATOR *allocator_, U32 memPurposeTable_, U32 memPurposeKey_, size_t reserve_ = 0);
	~hashtable();
	
	bool put(const char *key_, const void *value_);
	const void* get(const char *key_) const;
	const char* getKeyPtr(const char *key_) const;
	bool remove(const char *key_);
	void reset();
	
	size_t count() const;
private:
	struct entry {
		const char	*m_key;
		const void	*m_value;
	};
	
	FIOS_ALLOCATOR	*m_allocator;
	U32             m_memPurposeTable;
	U32             m_memPurposeKey;
	size_t			m_used_count, m_filled_count, m_capacity, m_table_size;
	entry			*m_table;
	
	hashtable(const hashtable&){}; // Don't allow copying.
	hashtable& operator=(const hashtable&){ return *this; }; // Don't allow assignment.
	static void initialize_table(entry *table_, size_t table_size_);
	static size_t find_available_row(const entry *table_, size_t table_count_, const char *key_, size_t key_length_);
	static size_t find_row(const entry *table_, size_t table_count_, const char *key_);
public:
	class FIOS_EXPORT entry_iterator {
	public:
		bool		hasNext();
		bool		advance();
		const char*	key();
		const void*	value();
	private:
		friend class hashtable;
		entry_iterator( entry *it_, entry *end_ );
		entry		*m_it, *m_end;
	};
	entry_iterator get_entry_iterator();
};

/** \brief Dynamic array. */
template <class T> class FIOS_EXPORT array {
public:
    explicit array(FIOS_ALLOCATOR *allocator_, U32 memPurpose_)
	: m_allocator(allocator_), m_memPurpose(memPurpose_), m_internalElements(reinterpret_cast<T*>(m_internalElementStorage)),
	  m_elements(m_internalElements), m_capacity(kInternalCapacityCount), m_used(0) {}
    
	~array() {
		reset();
		if (m_elements != m_internalElements)
			FIOS_DEALLOCATE_LONGTERM(m_allocator, m_memPurpose, m_elements);
	}
    
    size_t length() const { return m_used; }
    
    bool append(T value_) {
        //FIOS_ASSERT(invariants());
        
        if (m_used == m_capacity) {
            //  No space left, attempt to reallocate.
            size_t newCapacity = m_capacity * 2;
            T *newElements;
            FIOS_ALLOCATE_LONGTERM(m_allocator, T*, m_memPurpose, newElements, sizeof(T) * newCapacity);
            if (newElements) {
                for (size_t i = 0; i < m_used; ++i) {
                    newElements[i] = m_elements[i];
                    m_elements[i].~T();
                }
                if (m_elements != m_internalElements)
                    FIOS_DEALLOCATE_LONGTERM(m_allocator, m_memPurpose, m_elements);
                m_elements = newElements;
                m_capacity = newCapacity;
            } else {
                //FIOS_ASSERT(invariants());
                return false;
            }
        }
        
        m_elements[m_used++] = value_;
        //FIOS_ASSERT(invariants());
        return true;
    }
    
    T& operator[](size_t index_) const {
        //FIOS_ASSERT(invariants());
        //FIOS_ASSERT(index_ < m_used);
		// "Since we can't throw exceptions, it should assert against bad
		// accesses and if one occurs just return the first slot."
		if (index_ >= m_used)
			index_ = 0;
        return m_elements[index_];
    }
    
    void reset() {
        //FIOS_ASSERT(invariants());
        remove(0, m_used);
        //FIOS_ASSERT(invariants());
    }
    
    void remove(size_t offset_, size_t count_) {
        //FIOS_ASSERT(invariants());
        //FIOS_ASSERT(count_ <= m_used);
        //FIOS_ASSERT(offset_ <= m_used);
        //FIOS_ASSERT(count_+offset_ <= m_used);
        
		T *elementsEnd = m_elements + m_used;
		
		// Call destructors.
        T *removingIt = m_elements + offset_;
        T *removingEnd = removingIt + count_;
        //FIOS_ASSERT((removingEnd - removingIt) == count_);
        for (; removingIt != removingEnd; ++removingIt) {
            removingIt->~T();
        }
		
		//	Cinch up the array (overwrite the old elements with the elements that proceed them).
		removingIt = m_elements + offset_;
		T *replacingIt = removingEnd;
        T *replacingEnd = elementsEnd;
        for (; replacingIt != replacingEnd; ++replacingIt, ++removingIt) {
            *removingIt = *replacingIt;
        }
        m_used -= count_;
        
        //FIOS_ASSERT(invariants());
    }
private:
	array(const array&){}; // Don't allow copying.
	array& operator=(const array&){ return *this; }; // Don't allow assignment.
	
    enum { kInternalCapacityCount = 4 };
    
    FIOS_ALLOCATOR  *m_allocator;
    U32             m_memPurpose;
    T               *m_internalElements;
	char			m_internalElementStorage[sizeof(T)*kInternalCapacityCount];
    T               *m_elements;
    size_t          m_capacity, m_used;
    
    bool invariants() const {
        //FIOS_ASSERT(m_allocator);
        //FIOS_ASSERT(m_internalElements);
        //FIOS_ASSERT(m_elements);
        //FIOS_ASSERT(m_capacity);
        //FIOS_ASSERT(m_capacity % 2 == 0);
        //FIOS_ASSERT(m_used <= m_capacity);
        return true;
    }
};

	}; /* namespace collections */
}; /* namespace fios */

/*@}*/

#endif	//	_H_fios_collections_
