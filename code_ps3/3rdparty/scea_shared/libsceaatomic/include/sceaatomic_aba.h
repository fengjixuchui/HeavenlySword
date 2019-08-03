/*
 *  sceaatomic_aba.h
 *  atomic_test
 *
 *  Created by Alex Rosenberg on 10/10/05.
 *  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
 *
 */

#ifndef __SCEAATOMIC_ABA_H__
#define __SCEAATOMIC_ABA_H__ 1

#include "sceaatomic.h"

namespace SCEA { namespace Atomic
{

template <class T> class main_ptr;
	
union pointer_and_aba
{
	SCEAATOMIC_INLINE pointer_and_aba()								: m_cas64_wrapper(0)					{}
	SCEAATOMIC_INLINE pointer_and_aba(const pointer_and_aba & a)	: m_cas64_wrapper(a.m_cas64_wrapper)	{}

	SCEAATOMIC_INLINE pointer_and_aba& operator=(const pointer_and_aba& rhs) { m_cas64_wrapper = rhs.m_cas64_wrapper; return *this; }
	
	struct // anonymous
	{
#if SCEA_TARGET_RT_BIG_ENDIAN
		volatile U32 m_pointer_eal;		// either bottom 32bits of pointer or all of pointer
		volatile U32 m_aba_counter;		// monotonically increasing counter to avoid ABA problem (see literature)
#else
		volatile U32 m_aba_counter;		// monotonically increasing counter to avoid ABA problem (see literature)
		volatile U32 m_pointer_eal;		// either bottom 32bits of pointer or all of pointer
#endif
	};
	
	SCEA_ALIGN_BEG(16)
		volatile U64	m_cas64_wrapper	// for double-word CompareAndSwap encompassing both pointer and counter
	SCEA_ALIGN_END(16);
};
		
// this wrapper struct is both a node pointer and a counter to avoid the ABA problem
template <class T>
struct aba_wrapper
{
	void preconditions(void) { char _ct_assert[(__alignof(aba_wrapper<T>) < 16) ? 0 : 1]; (void)_ct_assert; }

	SCEAATOMIC_INLINE aba_wrapper() : bits() { preconditions(); }
#if SCEA_TARGET_CPU_CELL_SPU || SCEA_TARGET_RT_PTR_SIZE_64
	SCEAATOMIC_INLINE aba_wrapper(U32 a) { bits.m_pointer_eal = a; bits.m_aba_counter = 0; preconditions(); }
#else
	SCEAATOMIC_INLINE aba_wrapper(T* a) { bits.m_pointer_eal = reinterpret_cast<U32>(a); bits.m_aba_counter = 0; preconditions(); }
#endif	
	SCEAATOMIC_INLINE aba_wrapper(const aba_wrapper<T>& a) : bits(a.bits) { preconditions(); }

	SCEAATOMIC_INLINE aba_wrapper<T>& operator=(aba_wrapper<T>& a) { bits = a.bits; return *this; }
	SCEAATOMIC_INLINE aba_wrapper<T>& operator=(const aba_wrapper<T>& a) { bits = a.bits; return *this; }
				
	SCEAATOMIC_INLINE main_ptr<T> get_ptr(void) const
	{
#if SCEA_TARGET_CPU_CELL_SPU
		if (bits.m_pointer_eal == 0)
		{
			return main_ptr<T>();
		}
		else
		{
			return main_ptr<T>().set_ptr(bits.m_pointer_eal);
		}
#elif SCEA_TARGET_RT_PTR_SIZE_64
		if (bits.m_pointer_eal == 0)
		{
			return main_ptr<T>();
		}
		else
		{
			return main_ptr<T>().set_ptr(reinterpret_cast<T*>(U32ToMainAddr(bits.m_pointer_eal)));
		}
#else
		return main_ptr<T>().set_ptr(reinterpret_cast<T*>(bits.m_pointer_eal));
#endif
	}
	
	pointer_and_aba	bits;
};

}} // namespace SCEA::Atomic

#endif // __SCEAATOMIC_ABA_H__
