//----------------------------------------------------------------------------------------
//! 
//! \filename core/threadsafequeue.h
//! 
//----------------------------------------------------------------------------------------
#ifndef THREADSAFEQUEUE_H_
#define THREADSAFEQUEUE_H_

#include "core/syncprims.h"

template
<
	typename	Type,
	uint32_t	MaxNumElements
>
class ThreadSafeQueue
{
	public:
		//
		//	Queue operations.
		//
		void			PushBack	( const Type &obj );		// Push an object onto the back of the queue.
		Type			PopFront	();							// Remove the object at the front of the queue and return a copy.
		const Type &	PeekFront	()				const;		// Look at the front of the queue without removing it.

		bool			IsEmpty		()				const	{ ScopedCritical sc( m_Mutex ); return m_NumElements == 0; }
		bool			IsFull		()				const	{ ScopedCritical sc( m_Mutex ); return m_NumElements == MaxNumElements; }
		uint32_t		NumElements	()				const	{ ScopedCritical sc( m_Mutex ); return m_NumElements; }

	public:
		//
		//	Ctor, dtor.
		//
		ThreadSafeQueue()
		:	m_NumElements	( 0 )
		,	m_CurrentFront	( 0 )
		{}

		~ThreadSafeQueue()
		{}

	private:
		//
		//	Aggregated members.
		//
		mutable CriticalSection	m_Mutex;
		Type					m_Queue[ MaxNumElements ];
		uint32_t				m_NumElements;
		uint32_t				m_CurrentFront;
};

template < typename Type, uint32_t MaxNumElements >
void ThreadSafeQueue< Type, MaxNumElements >::PushBack( const Type &obj )
{
	ScopedCritical sc( m_Mutex );
	ntError_p( m_NumElements < MaxNumElements, ("Out of space on the queue.") );
	uint32_t idx = ( m_NumElements + m_CurrentFront ) % MaxNumElements;
	m_Queue[ idx ] = obj;
	m_NumElements++;
}

template < typename Type, uint32_t MaxNumElements >
Type ThreadSafeQueue< Type, MaxNumElements >::PopFront()
{
	ntError_p( m_NumElements > 0, ("There is nothing to pop - the queue is empty.") );
	ScopedCritical sc( m_Mutex );
	Type ret = m_Queue[ m_CurrentFront ];
	m_Queue[ m_CurrentFront ] = Type();		// Important if we're holding ref-counted objects.
	m_NumElements--;
	m_CurrentFront++;
	if ( m_CurrentFront >= MaxNumElements )
		m_CurrentFront = 0;

	return ret;
}

template < typename Type, uint32_t MaxNumElements >
const Type &ThreadSafeQueue< Type, MaxNumElements >::PeekFront() const
{
	ntError_p( m_NumElements > 0, ("There is nothing to peek - the queue is empty.") );
	ScopedCritical sc( m_Mutex );
	return m_Queue[ m_CurrentFront ];
}


#endif // !THREADSAFEQUEUE_H_

