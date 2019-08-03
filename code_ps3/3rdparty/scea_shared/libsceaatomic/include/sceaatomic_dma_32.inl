/*
 *  sceaatomic_dma_32.inl
 *  libsceaatomic
 *
 *  Created by Alex Rosenberg on 10/10/05.
 *  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
 *
 */

template <class T> SCEAATOMIC_INLINE typename main_ptr<T>::pointer	main_ptr<T>::get_ptr(void)		const			{ return m_p; }
template <class T> SCEAATOMIC_INLINE typename main_ptr<T>::pointer	main_ptr<T>::get_ptr(void)		const volatile	{ return m_p; }

template <class T> SCEAATOMIC_INLINE main_ptr<T>&	main_ptr<T>::set_char_ptr(const main_ptr<char> & p)	{ m_p = reinterpret_cast<pointer>(p.m_p); return *this; }

// remote proxy support
template <class T> SCEAATOMIC_INLINE void								main_ptr<T>::put(const T & rhs) { *const_cast<T*>(m_p) = rhs; }
template <class T> SCEAATOMIC_INLINE typename main_ptr<T>::plain_value	main_ptr<T>::get(void) const
{
	// using named return value optimization allows us to skip the explicit copy constructor which
	// intentionally zaps the next pointer
	typename main_ptr<T>::plain_value nrv;
	nrv = *m_p;
	return nrv;
}

// CAS a 32-bit pointer
template <class T> 
template <class U, class V>
SCEAATOMIC_INLINE main_ptr<U> main_ptr<T>::CAS(main_ptr<U> const & old_ptr, main_ptr<V> const & new_ptr) volatile
{
	main_ptr<U> old;
	U32 expected = reinterpret_cast<U32>(old_ptr.get_ptr());
	old.m_p = reinterpret_cast<U*>(CompareAndSwap(cas_helper(m_p), expected, reinterpret_cast<U32>(new_ptr.get_ptr())));
	return old;
}

// CAS a 64-bit pointer (possibly legacy structure shape)
template <class T>
template <class U>
SCEAATOMIC_INLINE U64 main_ptr<T>::CAS2(U64 expected, main_ptr<U> const & new_ptr) volatile
{
	return CompareAndSwap(reinterpret_cast<U64*>(m_p), expected, reinterpret_cast<U64>(new_ptr.m_p));
}

// specialize for aba_wrapper's ABA count bumping behavior
template <class T> 
template <class U, class V>
SCEAATOMIC_INLINE main_ptr<U> main_ptr<T>::CAS2(aba_wrapper<U> & old, main_ptr<V> const & new_ptr) volatile
{
	U64	expected = old.bits.m_cas64_wrapper;
	U32 old_count = expected & 0xFFFFFFFF;
	U64	new_value = ((reinterpret_cast<U64>(new_ptr.m_p) << 32) | (old_count + 1));

	U64 read = CompareAndSwap(reinterpret_cast<U64*>(m_p), expected, new_value);
	old.bits.m_cas64_wrapper = read;				// return what we read
	
	main_ptr<U> result;
	if (read == expected)
	{
		return result.set_ptr(reinterpret_cast<U*>(read >> 32));
	}
	else
	{
		return result.set_ptr(reinterpret_cast<U*>(~(expected >> 32)));	// make sure they don't see a match
	}
}

template <class T>
SCEAATOMIC_INLINE U32 main_ptr<T>::CAS(U32 expected, U32 new_value) volatile
{
	return CompareAndSwap(reinterpret_cast<U32*>(m_p), expected, new_value);
}

template <class T>
SCEAATOMIC_INLINE U64 main_ptr<T>::CAS2(U64 expected, U64 new_value) volatile
{
	return CompareAndSwap(reinterpret_cast<U64*>(m_p), expected, new_value);
}

template <class T>
template <class U>
SCEAATOMIC_INLINE U32* main_ptr<T>::cas_helper( U m_p_ )
{
	return reinterpret_cast<U32*>(m_p_);
}

template <class T>
template <class U>
SCEAATOMIC_INLINE U32* main_ptr<T>::cas_helper( SCEA::Atomic::aba_wrapper<U>* volatile m_p_ )
{
	volatile U32 *result = &m_p_->bits.m_pointer_eal;
	return (U32*)result;
}
