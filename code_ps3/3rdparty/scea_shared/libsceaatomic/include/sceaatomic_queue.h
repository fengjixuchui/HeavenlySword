/*
 *  sceaatomic_queue.h
 *  atomic_test
 *
 *  Created by Alex Rosenberg on 10/24/05.
 *  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
 *
 */

#ifndef __SCEAATOMIC_QUEUE_H__
#define __SCEAATOMIC_QUEUE_H__ 1

//#define SCEAATOMIC_CONFIG_QUEUE_DEBUG 0

#include "sceaatomic.h"
#include "sceaatomic_dma.h"
#include "sceaatomic_aba.h"
#include "sceaatomic_node.h"

#include <cstddef>
#include <memory>

#if SCEA_TARGET_CPU_CELL_SPU && SCEAATOMIC_CONFIG_QUEUE_DEBUG
	#include <spu_printf.h>
	#define SCEAATOMIC_QUEUE_PRINTF(...) spu_printf("SPU: " __VA_ARGS__)
#elif SCEAATOMIC_CONFIG_QUEUE_DEBUG
	#include <cstdio>
	#define SCEAATOMIC_QUEUE_PRINTF(...) std::printf("PPU: " __VA_ARGS__)
#else
	#if SCEA_HOST_COMPILER_MSVC == 1
		#define SCEAATOMIC_QUEUE_PRINTF __noop
	#else
		#define SCEAATOMIC_QUEUE_PRINTF(...)
	#endif
#endif

namespace SCEA { namespace Atomic {
	
template <class T>
class atomic_queue
{
public:	
	typedef atomic_node<T>			node_type;
	typedef T						value_type;
	typedef value_type*				pointer;
	typedef value_type&				reference;
	typedef const value_type&		const_reference;
	typedef std::size_t				size_type;
	typedef std::ptrdiff_t			difference_type;
	
	void preconditions(void) { char _ct_assert[(__alignof(T) < 16) ? 0 : 1]; (void)_ct_assert; }
	
#if SCEA_TARGET_CPU_CELL_SPU
	SCEAATOMIC_INLINE atomic_queue() : m_head(), m_tail()	{ preconditions(); }
#else
	// _concrete constructor
	SCEAATOMIC_INLINE atomic_queue(aba_wrapper<T>* head, aba_wrapper<T>* tail, T* dummy) :
		m_head(), m_tail(), m_dummy()
	{
	   m_head.set_ptr(head);
	   m_tail.set_ptr(tail);
	   m_dummy.set_ptr(dummy);
	   preconditions();
	}
#endif
	
	SCEAATOMIC_INLINE main_ptr<T>					front(void)				{ return get_head(); }
	SCEAATOMIC_INLINE main_ptr<T>					back(void)				{ return get_tail(); }
	
	SCEAATOMIC_INLINE bool							empty(void)		const;
	SCEAATOMIC_INLINE size_type						size(void)		const;	// O(N)
	SCEAATOMIC_INLINE size_type						max_size(void)	const	{ return size_type(-1); }
	
	void											push(main_ptr<value_type>& x);
	main_ptr<T>										pop(void);

	SCEAATOMIC_INLINE main_ptr<T>					sentinel(void)	const	{ main_ptr<T> s; s.set_ptr((typename main_ptr<T>::contained_type) -1); return s; }

protected:	
	SCEAATOMIC_INLINE main_ptr<T>					get_head(void)	const	{ return m_head.get().get_ptr(); }
	SCEAATOMIC_INLINE main_ptr<T>					get_tail(void)	const	{ return m_tail.get().get_ptr(); }
private:
		
	SCEA_ALIGN_BEG(16)
		main_ptr<aba_wrapper<T> >	m_head
	SCEA_ALIGN_END(16);
	main_ptr<aba_wrapper<T> >		m_tail;
	main_ptr<T>						m_dummy;
};

#if !SCEA_TARGET_CPU_CELL_SPU
template <class T>
class atomic_queue_concrete : public atomic_queue<T>
{
public:
	SCEAATOMIC_INLINE atomic_queue_concrete() :
#if SCEA_TARGET_RT_PTR_SIZE_64
		m_aba_head((U32)(U64) &m_dummy_node), m_aba_tail((U32)(U64) &m_dummy_node),
#else
		m_aba_head(&m_dummy_node), m_aba_tail(&m_dummy_node),
#endif
		atomic_queue<T>(&m_aba_head, &m_aba_tail, &m_dummy_node)
	{
		m_dummy_node.next = this->sentinel();
	}
	
private:
	aba_wrapper<T>				m_aba_head;
	aba_wrapper<T>				m_aba_tail;
	T							m_dummy_node;	// dummy node; never popped
};
#endif

template <class T>
void atomic_queue<T>::push(main_ptr<value_type>& x_ref)
{
	SCEAATOMIC_QUEUE_PRINTF("pushing queue %08x\n", x_ref.get_ptr());
	
	aba_wrapper<T>				old_tail;

	main_ptr<main_ptr<T> >	x_next = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(x_ref, next, T, SCEA::Atomic::main_ptr<T>);
	x_next.put(sentinel());

	while (1)
	{
		old_tail = m_tail.get();			// copy locally
		main_ptr<T>				tail_node = old_tail.get_ptr();
		main_ptr<main_ptr<T> >	tail_next = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(tail_node, next, T, SCEA::Atomic::main_ptr<T>);

		// try to set current tail's next to new node
		if (sentinel() == tail_next.CAS(sentinel(), x_ref))
		{
			break;
		}
		else
		{
			// tail did not point to last node so try to correct
			(void) m_tail.CAS2(old_tail, tail_next.get());
		}
	}
	// try to point tail to new node
	(void) m_tail.CAS2(old_tail, x_ref);
}

template <class T>
main_ptr<T> atomic_queue<T>::pop(void)
{
try_again:
	aba_wrapper<T>				old_head = m_head.get();							// copy locally
	aba_wrapper<T>				old_tail = m_tail.get();							// copy locally
	main_ptr<T>					head;

	while (1)
	{
		head = old_head.get_ptr();
		main_ptr<main_ptr<T> >	head_next = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(head, next, T, SCEA::Atomic::main_ptr<T>);
		main_ptr<T>				next = head_next.get();		// copy locally
		aba_wrapper<T>			later_head = m_head.get();	// copy locally
		
		// recheck to ensure that next is valid
		if (old_head.bits.m_aba_counter == later_head.bits.m_aba_counter)
		{
			// is empty or tail is behind
			if (head == m_tail.get().get_ptr())
			{
				if (next == sentinel())
				{
					main_ptr<T> result;
					return result;	// queue is empty
				}
				
				// tail doesn't point at last node, try to correct
				aba_wrapper<T> new_tail;
				new_tail.bits.m_pointer_eal = (U32) head.get_ptr();
				new_tail.bits.m_aba_counter = old_tail.bits.m_aba_counter;
				(void) m_tail.CAS2(new_tail, next);
			}
			else if (next != sentinel())
			{
				if (m_head.CAS2(old_head, next))
				{
					break;
				}
			}
		}
		
		old_head = m_head.get();							// copy locally
		old_tail = m_tail.get();							// copy locally
	}

	if (head == m_dummy)	// if popping the dummy
	{
		push(head);			// put it back
		
		//head = pop();		// and try popping again
		goto try_again;		// gcc can't eliminate this tail recursion
	}
	
	SCEAATOMIC_QUEUE_PRINTF("popping queue node %08x\n", head.get_ptr());
	
	return head;
}

template <class T>
bool atomic_queue<T>::empty(void) const
{
	// O(1) NOT THREAD-SAFE (requires that nodes not be immediately deleted; use a delete list!)
	main_ptr<T>	current = get_head();
	
	if (current)
	{
		current = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(current, next, T, SCEA::Atomic::main_ptr<T>).get();
		if (current != sentinel())
		{
			return false;
		}
	}
	
	return true;
}

template <class T>
typename atomic_queue<T>::size_type atomic_queue<T>::size(void) const
{
	// O(n) NOT THREAD-SAFE (requires that nodes not be immediately deleted; use a delete list!)
	main_ptr<T>	current = get_head();
	size_type	i = 0;

	while (current)
	{
		current = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(current, next, T, SCEA::Atomic::main_ptr<T>).get();
		if (current == sentinel())
		{
			break;
		}
		
		++i;
	}
	
	return i;
}

}} // namespace SCEA::Atomic

#endif // __SCEAATOMIC_QUEUE_H__
