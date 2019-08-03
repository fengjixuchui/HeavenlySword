/*
 *  sceaatomic_slist.h
 *  libsceaatomic
 *
 *  Created by Alex Rosenberg on 9/30/05.
 *  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
 *
 */

#ifndef __SCEAATOMIC_SLIST_H__
#define __SCEAATOMIC_SLIST_H__ 1

//#define SCEAATOMIC_CONFIG_SLIST_DEBUG 1
//#define SCEAATOMIC_CONFIG_SLIST_DEBUG 0

#include "sceaatomic.h"
#include "sceaatomic_dma.h"
#include "sceaatomic_aba.h"
#include "sceaatomic_node.h"

#include <cstddef>
#include <memory>

#if SCEA_TARGET_CPU_CELL_SPU && SCEAATOMIC_CONFIG_SLIST_DEBUG
	#include <spu_printf.h>
	#define SCEAATOMIC_SLIST_PRINTF(...) spu_printf("SPU: " __VA_ARGS__)
#elif SCEAATOMIC_CONFIG_SLIST_DEBUG
	#include <cstdio>
	#define SCEAATOMIC_SLIST_PRINTF(...) std::printf("PPU: " __VA_ARGS__)
#else
	#if SCEA_HOST_COMPILER_MSVC == 1
		#define SCEAATOMIC_SLIST_PRINTF __noop
	#else
		#define SCEAATOMIC_SLIST_PRINTF(...)
	#endif
#endif

namespace SCEA { namespace Atomic {

namespace detail
{
	// predicates used internally with find_if
	
	template <class T>
	struct is_equal_pred
	{
		SCEAATOMIC_INLINE is_equal_pred(main_ptr<T> n) : node(n) {}
		SCEAATOMIC_INLINE bool operator()(main_ptr<T> lhs) { return lhs == node; }
		
		main_ptr<T> node;
	};

	template <class T>
	struct is_not_equal_pred
	{
		SCEAATOMIC_INLINE is_not_equal_pred(main_ptr<T> n) : node(n) {}
		SCEAATOMIC_INLINE bool operator()(main_ptr<T> lhs) { return lhs != node; }
		
		main_ptr<T> node;
	};

	template <class T>
	struct count_pred
	{
		SCEAATOMIC_INLINE count_pred(int & counter) : i(counter) {}
		SCEAATOMIC_INLINE bool operator()(main_ptr<T> /*lhs*/) { ++i; return false; }
		
		int & i;
	};
}

template <class T>
class atomic_slist
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
	SCEAATOMIC_INLINE atomic_slist() : m_head(), m_tail()	{ preconditions(); }
#else
	// _concrete constructor
	SCEAATOMIC_INLINE atomic_slist(pointer head, pointer tail) :
		m_head(), m_tail()
	{
		m_head.set_ptr(head);
		m_tail.set_ptr(tail);
		preconditions();
	}
#endif
	
	SCEAATOMIC_INLINE bool			empty(void);			// O(N)
	SCEAATOMIC_INLINE size_type		size(void);				// O(N) only returns approximate size
	SCEAATOMIC_INLINE size_type		max_size(void)	const	{ return size_type(-1); }

	SCEAATOMIC_INLINE main_ptr<T>	front(void)		const	{ return get_head(); }

	void							push_front(main_ptr<T> node);
	main_ptr<T>						pop_front(void);
	
	template <class _Predicate>
	main_ptr<T>						find_if(main_ptr<T>& left, _Predicate pred);
	
	bool							try_insert_after(main_ptr<T> after, main_ptr<T> node);
	bool							try_erase_after(main_ptr<T> left, main_ptr<T> right);
	
	// we use the bottom bit to mark that a node has been erased
#if SCEA_TARGET_RT_PTR_SIZE_64
	static bool			is_marked(main_ptr<T> p)	{ return ((U32)(U64)p.get_ptr()) & 1; }
	
	static main_ptr<T>	add_mark(main_ptr<T> p)
	{
		main_ptr<T> result;
		result.set_ptr((typename main_ptr<T>::contained_type)((U32)(U64)(p.get_ptr()) | 1));
		return result;
	}

	static main_ptr<T>	remove_mark(main_ptr<T> p)
	{
		main_ptr<T> result;
		result.set_ptr((typename main_ptr<T>::contained_type)(reinterpret_cast<U64>(p.get_ptr()) & ~1));
		return result;
	}
#else
	static bool			is_marked(main_ptr<T> p)	{ return (U32)(p.get_ptr()) & 1; }

	static main_ptr<T>	add_mark(main_ptr<T> p)
	{
		main_ptr<T> result;
		result.set_ptr((typename main_ptr<T>::contained_type)((U32)(p.get_ptr()) | 1));
		return result;
	}
	
	static main_ptr<T>	remove_mark(main_ptr<T> p)
	{
		main_ptr<T> result;
		result.set_ptr((typename main_ptr<T>::contained_type)((U32)(p.get_ptr()) & ~1));
		return result;
	}
#endif
	
	SCEAATOMIC_INLINE main_ptr<T>	get_head(void)	const	{ return m_head; }
	SCEAATOMIC_INLINE main_ptr<T>	get_tail(void)	const	{ return m_tail; }
	
private:
	SCEA_ALIGN_BEG(16)
		main_ptr<T>		m_head
	SCEA_ALIGN_END(16);
	main_ptr<T>		m_tail;
};

#if !SCEA_TARGET_CPU_CELL_SPU
template <class T>
class atomic_slist_concrete : public atomic_slist<T>
{
public:
	SCEAATOMIC_INLINE atomic_slist_concrete() : atomic_slist<T>(&m_head_node, &m_tail_node)
	{
		m_head_node.next = &m_tail_node;
	}
	
private:
	T	m_head_node;	// dummy node; never popped
	T	m_tail_node;	// dummy node; never popped
};
#endif

template <class T>
template <class _Predicate>
main_ptr<T> atomic_slist<T>::find_if(main_ptr<T>& left, _Predicate pred)
{
	main_ptr<T>	head = m_head;
	main_ptr<T> tail = m_tail;
	
	main_ptr<T> left_next;
	main_ptr<T> right;
	
	while (1)
	{
		main_ptr<T> current = head;
		main_ptr<T> current_next = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(current, next, T, SCEA::Atomic::main_ptr<T>).get();
		
		// scan forward until we find an unmarked match
		do
		{
			if (!is_marked(current_next))
			{
				left = current;				// remember last unmarked node
				left_next = current_next;
			}
			
			current = remove_mark(current_next);
			if (current == tail)
			{
				break;
			}
			
			current_next = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(current, next, T, SCEA::Atomic::main_ptr<T>).get();
			
		} while (is_marked(current_next) || !pred(current));
		right = current;
		
		// confirm adjacency
		if (left_next == right)
		{
			if ((right != tail) && is_marked(SCEAATOMIC_MAIN_PTR_MEMBER_PTR(right, next, T, SCEA::Atomic::main_ptr<T>).get()))
			{
				continue;	// outer loop
			}
			else
			{
				SCEAATOMIC_SLIST_PRINTF("slist find_if found %08x\n", right.get_ptr());
				return right;
			}
		}
		
		// remove marked nodes
		main_ptr<main_ptr<T> >	left_next_ref = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(left, next, T, SCEA::Atomic::main_ptr<T>);
		if (left_next == left_next_ref.CAS(left_next, right))
		{
			if ((right != tail) && is_marked(SCEAATOMIC_MAIN_PTR_MEMBER_PTR(right, next, T, SCEA::Atomic::main_ptr<T>).get()))
			{
				continue;	// outer loop
			}
			else
			{
				SCEAATOMIC_SLIST_PRINTF("slist find_if found %08x\n", right.get_ptr());
				return right;
			}
		}
	};
}

template <class T>
void atomic_slist<T>::push_front(main_ptr<T> node)
{
	do {} while(!try_insert_after(m_head, node));
}

template <class T>
main_ptr<T> atomic_slist<T>::pop_front(void)
{
	main_ptr<T> left;
	main_ptr<T> right;
	
	do
	{
		right = find_if(left, detail::is_not_equal_pred<T>(m_head));
		if (right == m_tail)
		{
			main_ptr<T> result;
			return result;	// list is empty
		}
	} while(!try_erase_after(left, right));
	
	return right;
}

// try to insert node after the after node, return true if success
template <class T>
bool atomic_slist<T>::try_insert_after(main_ptr<T> after, main_ptr<T> node)
{
	if (after == m_tail)
	{
		return false;		//!!! should assert instead?
	}
	
	SCEAATOMIC_SLIST_PRINTF("slist try_insert_after(%08x,%08x)\n", after.get_ptr(), node.get_ptr());

	main_ptr<T>				right = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(after, next, T, SCEA::Atomic::main_ptr<T>).get();	// copy locally
	main_ptr<main_ptr<T> >	after_next_ref = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(after, next, T, SCEA::Atomic::main_ptr<T>);
	main_ptr<main_ptr<T> >	node_next_ref = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(node, next, T, SCEA::Atomic::main_ptr<T>);
	node_next_ref.put(right);

	return (right == after_next_ref.CAS(right, node));
}

// try to erase right, presumed to be immediately preceeded by left, return true if success
template <class T>
bool atomic_slist<T>::try_erase_after(main_ptr<T> left, main_ptr<T> right)
{
	main_ptr<main_ptr<T> >	left_next_ref = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(left, next, T, SCEA::Atomic::main_ptr<T>);
	main_ptr<main_ptr<T> >	right_next_ref = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(right, next, T, SCEA::Atomic::main_ptr<T>);
	main_ptr<T>				right_next = right_next_ref.get();	// copy locally
	
	// logically erase right by marking
	if (is_marked(right_next) || (right_next != right_next_ref.CAS(right_next, add_mark(right_next))))
	{
		return false;
	}
	
	if (right != left_next_ref.CAS(right, right_next))
	{
		// try to physically remove
		main_ptr<T>	dummy_left = left;
		(void) find_if(dummy_left, detail::is_equal_pred<T>(right));
	}
	
	SCEAATOMIC_SLIST_PRINTF("slist try_erase_after(%08x,%08x)\n", left.get_ptr(), right.get_ptr());
	
	return true;
}

template <class T>
bool atomic_slist<T>::empty(void)
{
	main_ptr<T> left;
	
	return (m_tail == find_if(left, detail::is_not_equal_pred<T>(m_head)));
}

template <class T>
typename atomic_slist<T>::size_type atomic_slist<T>::size(void)
{
	main_ptr<T> left;
	int i = 0;
	detail::count_pred<T> pred(i);	// stateful by reference to i

	(void) find_if(left, pred);
	return i;
}

}} // namespace SCEA::Atomic

#endif // __SCEAATOMIC_SLIST_H__
