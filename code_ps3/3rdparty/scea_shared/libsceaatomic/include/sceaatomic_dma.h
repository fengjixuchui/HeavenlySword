/*
 *  sceaatomic_dma.h
 *  libsceaatomic
 *
 *  Created by Alex Rosenberg on 10/10/05.
 *  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
 *
 */

#ifndef __SCEAATOMIC_DMA_H__
#define __SCEAATOMIC_DMA_H__ 1

#ifndef SCEAATOMIC_CONFIG_DMA_BASE_ALWAYS_ZERO
	#define SCEAATOMIC_CONFIG_DMA_BASE_ALWAYS_ZERO 1
#endif

#include <cstddef>
#include <cstring>		// for __Shuffles
#include "sceaatomic.h"

namespace SCEA { namespace Atomic
{

#if !SCEAATOMIC_CONFIG_DMA_BASE_ALWAYS_ZERO
	namespace detail
	{
		static U64	g_main_dma_base;	// base pointer for all main pointer accesses, initialized on entry
	}

	SCEAATOMIC_INLINE void	Initialize(U64 p)		{ detail::g_main_dma_base = p; }
	SCEAATOMIC_INLINE U64	U32ToMainAddr(U32 p)	{ return p + detail::g_main_dma_base; }
	SCEAATOMIC_INLINE U32	MainAddrToU32(U64 p)	{ return p - detail::g_main_dma_base; }
#else
	SCEAATOMIC_INLINE void	Initialize(U64)			{}
	SCEAATOMIC_INLINE U64	U32ToMainAddr(U32 p)	{ return p; }
	SCEAATOMIC_INLINE U32	MainAddrToU32(U64 p)	{ return static_cast<U32>(p); }
#endif

namespace detail
{
	// C++ TR1 type_traits (as prototyped in boost)
	template <class T>					struct remove_const						{ typedef T type; };
	template <class T>					struct remove_const<const T>			{ typedef T type; };
	template <class T>					struct remove_volatile					{ typedef T type; };
	template <class T>					struct remove_volatile<volatile T>		{ typedef T type; };
	template <class T>					struct remove_cv						{ typedef typename remove_const<typename remove_volatile<T>::type>::type type; };
	template <class T>					struct remove_cv<T&>					{ typedef T& type; };
	template <class T, std::size_t N>	struct remove_cv<T const[N]>			{ typedef T type[N]; };
	template <class T, std::size_t N>	struct remove_cv<T volatile[N]>			{ typedef T type[N]; };
	template <class T, std::size_t N>	struct remove_cv<T const volatile[N]>	{ typedef T type[N]; };
	
	void do_dma_get(U64 main_base, void* base, std::size_t size);			// libsceaatomic.a
	void do_dma_put(U64 main_base, volatile void* base, std::size_t size);	// libsceaatomic.a
}; // namespace detail

template <class T> struct aba_wrapper;
template <class T> class main_value;

template <class T>
class main_ptr
{
public:
	typedef main_ptr<T>										this_type;
	typedef T												value_type;
	typedef value_type*										pointer;
	typedef value_type&										reference;
	typedef typename detail::remove_cv<value_type>::type	plain_value;
#if SCEA_TARGET_CPU_CELL_SPU || SCEA_TARGET_RT_PTR_SIZE_64
	typedef U32												contained_type;
#else
	typedef pointer											contained_type;
#endif
	
	SCEAATOMIC_INLINE main_ptr() : m_p(0)	{}
	template <class U>
	SCEAATOMIC_INLINE main_ptr(main_ptr<U> const & rhs)	: m_p(static_cast<contained_type>(rhs.m_p)) {}
	SCEAATOMIC_INLINE main_ptr(this_type const & rhs)	: m_p(rhs.m_p) {}
	SCEAATOMIC_INLINE main_ptr(this_type const volatile & rhs)	: m_p(rhs.m_p) {}
	
	SCEAATOMIC_INLINE this_type&			operator=(this_type const & rhs)				{ m_p = rhs.m_p; return *this; }
	SCEAATOMIC_INLINE volatile this_type&	operator=(this_type const & rhs) volatile		{ m_p = rhs.m_p; return *this;}
	SCEAATOMIC_INLINE this_type&			operator=(pointer const & rhs)					{ return set_ptr(rhs); }
	
	SCEAATOMIC_INLINE this_type				operator+(std::ptrdiff_t d)		const			{ this_type n; n.set_ptr(m_p + (d * sizeof(T))); return n; }
	SCEAATOMIC_INLINE this_type				operator+(std::ptrdiff_t d)		const volatile	{ this_type n; n.set_ptr(m_p + (d * sizeof(T))); return n; }
	SCEAATOMIC_INLINE this_type				operator-(std::ptrdiff_t d)		const			{ this_type n; n.set_ptr(m_p - (d * sizeof(T))); return n; }
	SCEAATOMIC_INLINE this_type				operator-(std::ptrdiff_t d)		const volatile	{ this_type n; n.set_ptr(m_p - (d * sizeof(T))); return n; }
	
	// deref operator. main_value has value-like semantics, but internally stores a main_ptr. AtomicInt has an SPU specialization of it.
					  main_value<T>			operator*(void)					const			{ return main_value<T>(*this); }
					  main_value<T>			operator*(void)					const volatile	{ return main_value<T>(*this); }
	
#if !SCEA_TARGET_CPU_CELL_SPU
	SCEAATOMIC_INLINE pointer				get_ptr(void)					const;
	SCEAATOMIC_INLINE pointer				get_ptr(void)					const volatile;
#else
	SCEAATOMIC_INLINE contained_type		get_ptr(void)					const			{ return m_p; }
	SCEAATOMIC_INLINE contained_type		get_ptr(void)					const volatile	{ return m_p; }
#endif

#if SCEA_TARGET_RT_PTR_SIZE_64
	SCEAATOMIC_INLINE this_type&			set_ptr(pointer p)								{ m_p = MainAddrToU32(reinterpret_cast<U64>(p)); return *this; }
#endif
	SCEAATOMIC_INLINE this_type&			set_ptr(contained_type p)						{ m_p = p; return *this; }
	SCEAATOMIC_INLINE this_type&			set_ptr(this_type const p)						{ m_p = p.m_p; return *this; }

	SCEAATOMIC_INLINE const main_ptr<char>	get_char_ptr(void)				const			{ return *reinterpret_cast<main_ptr<char>* >(const_cast<this_type*>(this)); }
	SCEAATOMIC_INLINE this_type&			set_char_ptr(const main_ptr<char> & p);
	
	// remote proxy support
	SCEAATOMIC_INLINE plain_value			get(void)						const;
	SCEAATOMIC_INLINE void					put(const value_type & p);

	// limited bool context support
	typedef contained_type this_type::*unspecified_bool_type;
	SCEAATOMIC_INLINE operator unspecified_bool_type()						const			{ return m_p == 0 ? 0 : &this_type::m_p; }

	template<class U> friend SCEAATOMIC_INLINE bool operator==(main_ptr<U> const & a, main_ptr<U> const & b);
	template<class U> friend SCEAATOMIC_INLINE bool operator==(main_ptr<U> const & a, U* b);
	template<class U> friend SCEAATOMIC_INLINE bool operator==(U* a, main_ptr<U> const & b);

	template<class U> friend SCEAATOMIC_INLINE bool operator!=(main_ptr<U> const & a, main_ptr<U> const & b);
	template<class U> friend SCEAATOMIC_INLINE bool operator!=(main_ptr<U> const & a, U* b);
	template<class U> friend SCEAATOMIC_INLINE bool operator!=(U* a, main_ptr<U> const & b);

	template<class U> friend SCEAATOMIC_INLINE bool operator>(main_ptr<U> const & a, main_ptr<U> const & b);
	template<class U> friend SCEAATOMIC_INLINE bool operator>(main_ptr<U> const & a, U* b);
	template<class U> friend SCEAATOMIC_INLINE bool operator>(U* a, main_ptr<U> const & b);
	
	template<class U> friend SCEAATOMIC_INLINE bool operator<(main_ptr<U> const & a, main_ptr<U> const & b);
	template<class U> friend SCEAATOMIC_INLINE bool operator<(main_ptr<U> const & a, U* b);
	template<class U> friend SCEAATOMIC_INLINE bool operator<(U* a, main_ptr<U> const & b);
	
	template<class U> friend SCEAATOMIC_INLINE bool operator<=(main_ptr<U> const & a, main_ptr<U> const & b);
	template<class U> friend SCEAATOMIC_INLINE bool operator<=(main_ptr<U> const & a, U* b);
	template<class U> friend SCEAATOMIC_INLINE bool operator<=(U* a, main_ptr<U> const & b);
	
	template<class U> friend SCEAATOMIC_INLINE bool operator>=(main_ptr<U> const & a, main_ptr<U> const & b);
	template<class U> friend SCEAATOMIC_INLINE bool operator>=(main_ptr<U> const & a, U* b);
	template<class U> friend SCEAATOMIC_INLINE bool operator>=(U* a, main_ptr<U> const & b);
	
	template<class U> friend struct aba_wrapper;
	template<class U> friend class main_ptr;
	
	// CAS a 32-bit pointer
	template <class U, class V>
	SCEAATOMIC_INLINE main_ptr<U>	CAS(main_ptr<U> const & old_ptr, main_ptr<V> const & new_ptr)	volatile;
	
	// CAS a 64-bit pointer
	template <class U>
	SCEAATOMIC_INLINE U64			CAS2(U64 expected, main_ptr<U> const & new_ptr)					volatile;
	
	// CAS a 64-bit pointer
	SCEAATOMIC_INLINE main_ptr<T>	CAS2(main_ptr<T> old_ptr, U64 new_value)						volatile;
	
	// specialize for aba_wrapper's ABA count bumping behavior (alters old!)
	template <class U, class V>
	SCEAATOMIC_INLINE main_ptr<U>	CAS2(aba_wrapper<U> & old, main_ptr<V> const & new_ptr)			volatile;
	
	// conventional CAS operations
	SCEAATOMIC_INLINE U32			CAS(U32 expected, U32 new_value)								volatile;
	SCEAATOMIC_INLINE U64			CAS2(U64 expected, U64 new_value)								volatile;
	
private:
	template <class U>
	static SCEAATOMIC_INLINE U32* cas_helper( U m_p_ );
	template <class U>
	static SCEAATOMIC_INLINE U32* cas_helper( SCEA::Atomic::aba_wrapper<U>* volatile m_p_ );

	contained_type	m_p;
};

#if SCEA_TARGET_CPU_CELL_SPU
	#include "sceaatomic_dma_spu.inl"
#elif SCEA_TARGET_RT_PTR_SIZE_64
	#include "sceaatomic_dma_64.inl"
#elif SCEA_TARGET_RT_PTR_SIZE_32
	#include "sceaatomic_dma_32.inl"
#else
	#error main_ptr not defined for this combination
#endif

template<class T> SCEAATOMIC_INLINE bool operator==(main_ptr<T> const & a,	main_ptr<T> const & b)	{ return a.m_p			==	b.m_p;		 }
template<class T> SCEAATOMIC_INLINE bool operator==(main_ptr<T> const & a,	T* b)					{ return a.get_ptr()	==	b;			 }
template<class T> SCEAATOMIC_INLINE bool operator==(T* a,					main_ptr<T> const & b)	{ return a				==	b.get_ptr(); }

template<class T> SCEAATOMIC_INLINE bool operator!=(main_ptr<T> const & a,	main_ptr<T> const & b)	{ return a.m_p			!=	b.m_p;		 }
template<class T> SCEAATOMIC_INLINE bool operator!=(main_ptr<T> const & a,	T* b)					{ return a.get_ptr()	!=	b;			 }
template<class T> SCEAATOMIC_INLINE bool operator!=(T* a,					main_ptr<T> const & b)	{ return a				!=	b.get_ptr(); }

template<class T> SCEAATOMIC_INLINE bool operator<(main_ptr<T> const & a,	main_ptr<T> const & b)	{ return a.m_p			<	b.m_p;		 }
template<class T> SCEAATOMIC_INLINE bool operator<(main_ptr<T> const & a,	T* b)					{ return a.get_ptr()	<	b;			 }
template<class T> SCEAATOMIC_INLINE bool operator<(T* a,					main_ptr<T> const & b)	{ return a				<	b.get_ptr(); }

template<class T> SCEAATOMIC_INLINE bool operator>(main_ptr<T> const & a,	main_ptr<T> const & b)	{ return a.m_p			>	b.m_p;		 }
template<class T> SCEAATOMIC_INLINE bool operator>(main_ptr<T> const & a,	T* b)					{ return a.get_ptr()	>	b;			 }
template<class T> SCEAATOMIC_INLINE bool operator>(T* a,					main_ptr<T> const & b)	{ return a				>	b.get_ptr(); }

template<class T> SCEAATOMIC_INLINE bool operator<=(main_ptr<T> const & a,	main_ptr<T> const & b)	{ return a.m_p			<=	b.m_p;		 }
template<class T> SCEAATOMIC_INLINE bool operator<=(main_ptr<T> const & a,	T* b)					{ return a.get_ptr()	<=	b;			 }
template<class T> SCEAATOMIC_INLINE bool operator<=(T* a,					main_ptr<T> const & b)	{ return a				<=	b.get_ptr(); }

template<class T> SCEAATOMIC_INLINE bool operator>=(main_ptr<T> const & a,	main_ptr<T> const & b)	{ return a.m_p			>=	b.m_p;		 }
template<class T> SCEAATOMIC_INLINE bool operator>=(main_ptr<T> const & a,	T* b)					{ return a.get_ptr()	>=	b;			 }
template<class T> SCEAATOMIC_INLINE bool operator>=(T* a,					main_ptr<T> const & b)	{ return a				>=	b.get_ptr(); }

#if 1
	//	CURRENT GCC & MSVC++ HAPPY VERSION:
	template <class memberType, class ptrType>
typename SCEA::Atomic::main_ptr<typename SCEA::Atomic::detail::remove_cv<memberType>::type> sceaatomic_main_ptr_member_ptr_helper( ptrType ptr, std::size_t member_offset ) {
		SCEA::Atomic::main_ptr<typename SCEA::Atomic::detail::remove_cv<memberType>::type> new_ptr;
		new_ptr.set_char_ptr(ptr.get_char_ptr() + member_offset);
		return new_ptr;
	}

	#define SCEAATOMIC_OFFSET_OF_PTR_MEMBER( ptrValueType, member )	\
		((std::size_t)(&((ptrValueType*)1)->member)-1)

	// equivalent to new_ptr = &ptr->member, required because main_ptr and anything containing one is non-POD
	// and offsetof doesn't work for non-POD.
	#define SCEAATOMIC_MAIN_PTR_MEMBER_PTR(ptr,member,ptrValueType,memberType) \
		SCEA::Atomic::sceaatomic_main_ptr_member_ptr_helper<memberType >( ptr, SCEAATOMIC_OFFSET_OF_PTR_MEMBER( ptrValueType, member ))
#elif 0
	//	TRANSITIONAL HINTED-ERROR-GENERATING VERSION:
	class ptrValueType{}; class memberType{};
	#define SCEAATOMIC_MAIN_PTR_MEMBER_PTR_IMP(ptr,member)															\
	({																											\
		typeof(ptr.get()) * base;																				\
		SCEA::Atomic::ptrValueType a = ptr.get(); &a; SCEA::Atomic::memberType b = base->member; &b;			\
		U32 offset = (char*)(&base->member) - (char*)(base);													\
		SCEA::Atomic::main_ptr<typename SCEA::Atomic::detail::remove_cv<typeof(base->member)>::type> new_ptr;	\
		new_ptr.set_char_ptr(ptr.get_char_ptr() + offset);														\
		new_ptr;																								\
	})
	#define SCEAATOMIC_MAIN_PTR_MEMBER_PTR(ptr,member,ptrValueType,memberType) SCEAATOMIC_MAIN_PTR_MEMBER_PTR_IMP(ptr,member)
#else
	//	ORIGINAL STATEMENT-EXPRESSION VERSION (doesn't work on MSVC++):
	
	// equivalent to new_ptr = &ptr->member, required because main_ptr and anything containing one is non-POD
	// and offsetof doesn't work for non-POD.
	#define SCEAATOMIC_MAIN_PTR_MEMBER_PTR_IMP(ptr,member)															\
	({																											\
		typeof(ptr.get()) * base;																				\
		U32 offset = (char*)(&base->member) - (char*)(base);													\
		SCEA::Atomic::main_ptr<typename SCEA::Atomic::detail::remove_cv<typeof(base->member)>::type> new_ptr;	\
		new_ptr.set_char_ptr(ptr.get_char_ptr() + offset);														\
		new_ptr;																								\
	})
	#define SCEAATOMIC_MAIN_PTR_MEMBER_PTR(ptr,member,ptrValueType,memberType) SCEAATOMIC_MAIN_PTR_MEMBER_PTR_IMP(ptr,member) 
#endif

}} // namespace SCEA::Atomic

#endif // __SCEAATOMIC_STACK_H__
