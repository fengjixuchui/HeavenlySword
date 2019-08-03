/*
*  sceaatomic_dma_spu.inl
*  libsceaatomic
*
*  Created by Alex Rosenberg on 10/10/05.
*  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
*
*/

template <class T> SCEAATOMIC_INLINE main_ptr<T>&	main_ptr<T>::set_char_ptr(const main_ptr<char> & p)	{ m_p = p.m_p; return *this; }

// CAS a 32-bit pointer
template <class T> 
template <class U, class V>
SCEAATOMIC_INLINE main_ptr<U> main_ptr<T>::CAS(main_ptr<U> const & old_ptr, main_ptr<V> const & new_ptr) volatile
{
	main_ptr<U> old;
	return old.set_ptr(CompareAndSwap(U32ToMainAddr(m_p), old_ptr.m_p, new_ptr.m_p));
}

// CAS a 64-bit pointer
template <class T> 
template <class U>
SCEAATOMIC_INLINE U64 main_ptr<T>::CAS2(U64 expected, main_ptr<U> const & new_ptr) volatile
{
	return CompareAndSwap(U32ToMainAddr(m_p), expected, U32ToMainAddr(new_ptr.m_p));
}

// CAS a 64-bit pointer
template <class T> 
SCEAATOMIC_INLINE main_ptr<T> main_ptr<T>::CAS2(main_ptr<T> old_ptr, U64 new_value) volatile
{
	main_ptr<T> result;
	return result.set_ptr(MainAddrToU32(CompareAndSwap(U32ToMainAddr(m_p), U32ToMainAddr(old_ptr.m_p), new_value)));
}

// specialize for aba_wrapper's ABA count bumping behavior
template <class T> 
template <class U, class V>
SCEAATOMIC_INLINE main_ptr<U> main_ptr<T>::CAS2(aba_wrapper<U> & old, main_ptr<V> const & new_ptr) volatile
{
	U64	expected = old.bits.m_cas64_wrapper;
	U32 old_count = expected & 0xFFFFFFFF;
	U64	new_value = ((static_cast<U64>(new_ptr.m_p) << 32) | (old_count + 1));

	U64 read = CompareAndSwap(U32ToMainAddr(m_p), expected, new_value);
	old.bits.m_cas64_wrapper = read;				// return what we read
	
	main_ptr<U> result;
	if (read == expected)
	{
		return result.set_ptr(read >> 32);
	}
	else
	{
		return result.set_ptr(~(expected >> 32));	// make sure they don't see a match
	}
}

template <class T> 
SCEAATOMIC_INLINE U32 main_ptr<T>::CAS(U32 expected, U32 new_value) volatile
{
	return CompareAndSwap(U32ToMainAddr(m_p), expected, new_value);
}

template <class T> 
SCEAATOMIC_INLINE U64 main_ptr<T>::CAS2(U64 expected, U64 new_value) volatile
{
	return CompareAndSwap(U32ToMainAddr(m_p), expected, new_value);
}

namespace detail {

// private to get()
template <class T>
SCEAATOMIC_INLINE typename main_ptr<T>::plain_value main_ptr_T__get_small_aligned(main_ptr<T> p)
{
	typedef typename main_ptr<T>::value_type value_type;
	
	U64	main_addr = U32ToMainAddr(p.get_ptr());
	
	// alignment can't really be guaranteed to line up, so we'll do it manually
	vector unsigned char buf[2];
	
	detail::do_dma_get(main_addr, (void*)(((U32)(char*)buf) | (((U32)main_addr) & 0xF)), sizeof(T));

	if ((__alignof__(T) == sizeof(T)) &&
		(sizeof(T) == 8) || (sizeof(T) == 4) || (sizeof(T) == 2) || (sizeof(T) == 1))
	{
		vector unsigned char data = spu_slqwbyte(buf[0], main_addr & 0xF);
		return *(value_type*)&data;
	}
	else
	{
		// data may straddle two vectors
		vector unsigned char data = spu_shuffle(buf[0], buf[1], std::__Shuffles[main_addr & 0xF]);
		return *(value_type*)&data;
	}
}

} // namespace detail

template <class T>
SCEAATOMIC_INLINE typename main_ptr<T>::plain_value main_ptr<T>::get(void) const
{
	value_type		localcopy;
	U64				main_addr = U32ToMainAddr(m_p);
	
	if (sizeof(T) < 16)
	{
		localcopy = detail::main_ptr_T__get_small_aligned<T>(*this);	// call utility so that we always return via NRV
	}
	else
	{
		// objects >= 16 bytes are presumed to be quad aligned
		char _ct_assert[((sizeof(T) >= 16) && (__alignof__(T) < 16)) ? 0 : 1]; (void)_ct_assert;

		detail::do_dma_get(main_addr, &localcopy, sizeof(T));
	}

	return localcopy;
}

template <class T>
SCEAATOMIC_INLINE void main_ptr<T>::put(const main_ptr<T>::value_type & v)
{
	U64	main_addr = U32ToMainAddr(m_p);
	
	if (sizeof(T) < 16)
	{
		// alignment can't really be guaranteed to line up, so we'll do it manually
		vector unsigned char buf[2];
		
		vector unsigned char data = *(vector unsigned char*)&v;
		
		data = spu_rlqwbyte(data, -main_addr);	// rotate right
		buf[0] = data;
		
		if (!((__alignof__(T) == sizeof(T)) &&
			  (sizeof(T) == 8) || (sizeof(T) == 4) || (sizeof(T) == 2) || (sizeof(T) == 1)))
		{
			buf[1] = data;						// data may straddle two vectors
		}
		
		detail::do_dma_put(main_addr, (volatile void*)(((U32)(char*)buf) | (((U32)main_addr) & 0xF)), sizeof(T));
	}
	else
	{
		// objects > 16 bytes are presumed to be quad aligned
		char _ct_assert[((sizeof(T) >= 16) && (__alignof__(T) < 16)) ? 0 : 1]; (void)_ct_assert;
		
		detail::do_dma_put(main_addr, (volatile void*) &v, sizeof(T));
	}
}
