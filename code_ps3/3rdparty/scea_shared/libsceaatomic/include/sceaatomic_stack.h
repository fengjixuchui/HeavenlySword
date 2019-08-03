/*
 *  sceaatomic_stack.h
 *  libsceaatomic
 *
 *  Created by Alex Rosenberg on 10/4/05.
 *  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
 *
 */

#ifndef __SCEAATOMIC_STACK_H__
#define __SCEAATOMIC_STACK_H__ 1

//#define SCEAATOMIC_CONFIG_STACK_DEBUG 0

#include "sceaatomic.h"
#include "sceaatomic_dma.h"
#include "sceaatomic_aba.h"
#include "sceaatomic_node.h"

#include <cstddef>
#include <memory>

#if SCEA_TARGET_CPU_CELL_SPU && SCEAATOMIC_CONFIG_STACK_DEBUG
	#include <spu_printf.h>
	#define SCEAATOMIC_PRINTF(...) spu_printf("SPU: " __VA_ARGS__)
#elif SCEAATOMIC_CONFIG_STACK_DEBUG
	#include <cstdio>
	#define SCEAATOMIC_PRINTF(...) std::printf("PPU: " __VA_ARGS__)
#else
	#if SCEA_HOST_COMPILER_MSVC == 1
		#define SCEAATOMIC_PRINTF __noop
	#else
		#define SCEAATOMIC_PRINTF(...)
	#endif
#endif

namespace SCEA { namespace Atomic {

template <class T>
class atomic_stack
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
	SCEAATOMIC_INLINE atomic_stack()					: m_top()	{ preconditions(); }
#else
	// _concrete constructor
	SCEAATOMIC_INLINE atomic_stack(aba_wrapper<T>* aba)	: m_top()	{ m_top.set_ptr(aba); preconditions(); }
#endif
	
	SCEAATOMIC_INLINE main_ptr<T>					top(void)				{ return get_top(); }
	
	SCEAATOMIC_INLINE bool							empty(void)		const	{ return get_top() == NULL; }
	SCEAATOMIC_INLINE size_type						size(void)		const;	// O(N)
	SCEAATOMIC_INLINE size_type						max_size(void)	const	{ return size_type(-1); }
	
	SCEAATOMIC_INLINE void							push(main_ptr<value_type>& x);
	SCEAATOMIC_INLINE main_ptr<T>					pop(void);

#if !SCEA_TARGET_CPU_CELL_SPU
	SCEAATOMIC_INLINE main_ptr<aba_wrapper<T> >		get_aba(void)	const	{ return m_top; }
#endif
protected:	
	SCEAATOMIC_INLINE main_ptr<T>					get_top(void)	const	{ return m_top.get().get_ptr(); }
private:

	SCEA_ALIGN_BEG(16)
		main_ptr<aba_wrapper<T> >	m_top
	SCEA_ALIGN_END(16);
};

#if !SCEA_TARGET_CPU_CELL_SPU
template <class T>
class atomic_stack_concrete : public atomic_stack<T>
{
public:
	SCEAATOMIC_INLINE atomic_stack_concrete() : atomic_stack<T>(&m_aba) {}
private:
	aba_wrapper<T>				m_aba;
};
#endif

// caller must DMA put the new node prior to calling
template <class T>
void atomic_stack<T>::push(main_ptr<T>& x_ref)
{
	SCEAATOMIC_PRINTF("pushing %08x\n", x_ref.get_ptr());

	aba_wrapper<T>			old_top = m_top.get();			// copy locally
	main_ptr<T>				last_seen = old_top.get_ptr();

	main_ptr<main_ptr<T> >	x_next = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(x_ref, next, T, SCEA::Atomic::main_ptr<T>);
	
	for (;;)
	{
		main_ptr<T> expected = last_seen;
		x_next.put(expected);								// point new node at stack top
		
		last_seen = m_top.CAS(expected, x_ref);
		if (last_seen == expected)
		{
			break;
		}
	}
}

template <class T>
main_ptr<T> atomic_stack<T>::pop(void)
{
	aba_wrapper<T>	old_top = m_top.get();				// copy locally
	main_ptr<T>		last_seen = old_top.get_ptr();
	main_ptr<T>		head;

	for (;;)
	{
		head = last_seen;
		if (!head)
		{
			break;
		}
		
		main_ptr<main_ptr<T> >	head_next = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(head, next, T, SCEA::Atomic::main_ptr<T>);
		
		last_seen = m_top.CAS2(old_top, head_next.get());	// ABA solution
		if (last_seen == head)
		{
			break;
		}
	}

	return head;
}

template <class T>
typename atomic_stack<T>::size_type atomic_stack<T>::size(void) const
{
	// O(n) NOT THREAD-SAFE (requires that nodes not be immediately deleted; use a delete list!)
	main_ptr<T>	current = get_top();
	size_type	i = 0;
	
	while (current)
	{
		++i;
		current = SCEAATOMIC_MAIN_PTR_MEMBER_PTR(current, next, T, SCEA::Atomic::main_ptr<T>).get();
	}
	
	return i;
}

}} // namespace SCEA::Atomic

#endif // __SCEAATOMIC_STACK_H__
