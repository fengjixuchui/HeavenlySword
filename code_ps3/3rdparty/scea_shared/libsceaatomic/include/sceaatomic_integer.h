/*
 *  sceaatomic_integer.h
 *  libsceaatomic
 *
 *  Created by Alex Rosenberg on 1/19/06.
 *  Copyright 2006 Sony Computer Entertainment America, Inc.. All rights reserved.
 *
 */

#ifndef __SCEAATOMIC_INTEGER_H__
#define __SCEAATOMIC_INTEGER_H__

#if defined(__CELLOS_LV2__)
	#include <sdk_version.h>
#endif

#include <wchar.h>
#include <limits.h>
#include "sceaatomic.h"
#include "sceaatomic_dma.h"

#if SCEA_HOST_COMPILER_MSVC || SCEA_TARGET_OS_PSP
	#define SCEAATOMIC_INTEGER_WCHAR 0
#else
	#define SCEAATOMIC_INTEGER_WCHAR 1
#endif

namespace SCEA { namespace Atomic {

namespace detail {
	
	// simplified from boost/static_assert.hpp
	template <bool b>	struct STATIC_ASSERTION_FAILURE;
	template <>			struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
	template <int x>	struct static_assert_test{};
	
	// This C++ solution produces slightly better error messages.
#define SCEAATOMIC_STATIC_ASSERT(x)												\
	typedef SCEA::Atomic::detail::static_assert_test<							\
		sizeof(SCEA::Atomic::detail::STATIC_ASSERTION_FAILURE< (bool)(x) >)>	\
		SCEAATOMIC_CAT(static_assert_typedef_, __LINE__)

// simplified from Boost. Paul Mensonides is a genius.
#define SCEAATOMIC_CAT(a,b)		SCEAATOMIC_CAT_I(a,b)
#define SCEAATOMIC_CAT_I(a, b)	SCEAATOMIC_CAT_II(a ## b)
#define SCEAATOMIC_CAT_II(res)	res
		
	// simplified from TR1 <type_traits>
	struct false_	{ enum { value = false }; };
	struct true_	{ enum { value = true }; };

#define SCEAATOMIC_TT_VALID(name,type)	template<> struct name<type> : true_ {}
#define SCEAATOMIC_TT_CV(name,type)		SCEAATOMIC_TT_VALID(name, type);			\
										SCEAATOMIC_TT_VALID(name, type const);		\
										SCEAATOMIC_TT_VALID(name, type volatile);	\
										SCEAATOMIC_TT_VALID(name, type const volatile)

	template <class T> struct is_integral : false_ {};

	SCEAATOMIC_TT_CV(is_integral, unsigned char);
	SCEAATOMIC_TT_CV(is_integral, unsigned short);
	SCEAATOMIC_TT_CV(is_integral, unsigned int);
	SCEAATOMIC_TT_CV(is_integral, unsigned long);
	SCEAATOMIC_TT_CV(is_integral, unsigned long long);

	SCEAATOMIC_TT_CV(is_integral, signed char);
	SCEAATOMIC_TT_CV(is_integral, signed short);
	SCEAATOMIC_TT_CV(is_integral, signed int);
	SCEAATOMIC_TT_CV(is_integral, signed long);
	SCEAATOMIC_TT_CV(is_integral, signed long long);

	SCEAATOMIC_TT_CV(is_integral, bool);
	SCEAATOMIC_TT_CV(is_integral, char);
#if SCEAATOMIC_INTEGER_WCHAR
	SCEAATOMIC_TT_CV(is_integral, wchar_t);
#endif
	
	// Determine what type we should promote an integral type to.
	//
	// The API is like the one in boost::numeric::ublas, but the technique used is
	// from "C++ Templates" by Vendevoorde and Josuttis (although their results for
	// two of the same type are "wrong").
	template <typename T, typename U>
	struct promote_traits;
	
	#define SCEAATOMIC_TT_PROMOTION_SAME(A,Result)	\
		template <>							\
		struct promote_traits<A,A>			\
		{									\
			typedef Result promote_type;	\
		};									\
			
	#define SCEAATOMIC_TT_PROMOTION(A,B,Result)		\
		template <>							\
		struct promote_traits<A,B>			\
		{									\
			typedef Result promote_type;	\
		};									\
											\
		template <>							\
		struct promote_traits<B,A>			\
		{									\
			typedef Result promote_type;	\
		};
	
	SCEAATOMIC_TT_PROMOTION_SAME(bool,int);
	SCEAATOMIC_TT_PROMOTION(bool,char,int);
	SCEAATOMIC_TT_PROMOTION(bool,signed char,int);
	SCEAATOMIC_TT_PROMOTION(bool,unsigned char,int);
	SCEAATOMIC_TT_PROMOTION(bool,short,int);
	SCEAATOMIC_TT_PROMOTION(bool,unsigned short,int);
	SCEAATOMIC_TT_PROMOTION(bool,int,int);
	SCEAATOMIC_TT_PROMOTION(bool,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(bool,long,long);
	SCEAATOMIC_TT_PROMOTION(bool,unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(bool,long long,long long);
	SCEAATOMIC_TT_PROMOTION(bool,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(bool,float,float);
	SCEAATOMIC_TT_PROMOTION(bool,double,double);
	SCEAATOMIC_TT_PROMOTION(bool,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(char,int);
	SCEAATOMIC_TT_PROMOTION(char,signed char,int);
	SCEAATOMIC_TT_PROMOTION(char,unsigned char,int);
	SCEAATOMIC_TT_PROMOTION(char,short,int);
	SCEAATOMIC_TT_PROMOTION(char,unsigned short,int);
	SCEAATOMIC_TT_PROMOTION(char,int,int);
	SCEAATOMIC_TT_PROMOTION(char,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(char,long,long);
	SCEAATOMIC_TT_PROMOTION(char,unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(char,long long,long long);
	SCEAATOMIC_TT_PROMOTION(char,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(char,float,float);
	SCEAATOMIC_TT_PROMOTION(char,double,double);
	SCEAATOMIC_TT_PROMOTION(char,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(signed char,int);
	SCEAATOMIC_TT_PROMOTION(signed char,unsigned char,int);
	SCEAATOMIC_TT_PROMOTION(signed char,short,int);
	SCEAATOMIC_TT_PROMOTION(signed char,unsigned short,int);
	SCEAATOMIC_TT_PROMOTION(signed char,int,int);
	SCEAATOMIC_TT_PROMOTION(signed char,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(signed char,long,long);
	SCEAATOMIC_TT_PROMOTION(signed char,unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(signed char,long long,long long);
	SCEAATOMIC_TT_PROMOTION(signed char,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(signed char,float,float);
	SCEAATOMIC_TT_PROMOTION(signed char,double,double);
	SCEAATOMIC_TT_PROMOTION(signed char,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(unsigned char,int);
	SCEAATOMIC_TT_PROMOTION(unsigned char,short,int);
	SCEAATOMIC_TT_PROMOTION(unsigned char,unsigned short,int);
	SCEAATOMIC_TT_PROMOTION(unsigned char,int,int);
	SCEAATOMIC_TT_PROMOTION(unsigned char,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(unsigned char,long,long);
	SCEAATOMIC_TT_PROMOTION(unsigned char,unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(unsigned char,long long,long long);
	SCEAATOMIC_TT_PROMOTION(unsigned char,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(unsigned char,float,float);
	SCEAATOMIC_TT_PROMOTION(unsigned char,double,double);
	SCEAATOMIC_TT_PROMOTION(unsigned char,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(short,int);
	SCEAATOMIC_TT_PROMOTION(short,unsigned short,int);
	SCEAATOMIC_TT_PROMOTION(short,int,int);
	SCEAATOMIC_TT_PROMOTION(short,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(short,long,long);
	SCEAATOMIC_TT_PROMOTION(short,unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(short,long long,long long);
	SCEAATOMIC_TT_PROMOTION(short,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(short,float,float);
	SCEAATOMIC_TT_PROMOTION(short,double,double);
	SCEAATOMIC_TT_PROMOTION(short,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(unsigned short,int);
	SCEAATOMIC_TT_PROMOTION(unsigned short,int,int);
	SCEAATOMIC_TT_PROMOTION(unsigned short,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(unsigned short,long,long);
	SCEAATOMIC_TT_PROMOTION(unsigned short,unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(unsigned short,long long,long long);
	SCEAATOMIC_TT_PROMOTION(unsigned short,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(unsigned short,float,float);
	SCEAATOMIC_TT_PROMOTION(unsigned short,double,double);
	SCEAATOMIC_TT_PROMOTION(unsigned short,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(int,int);
	SCEAATOMIC_TT_PROMOTION(int,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(int,long,long);
	SCEAATOMIC_TT_PROMOTION(int,unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(int,long long,long long);
	SCEAATOMIC_TT_PROMOTION(int,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(int,float,float);
	SCEAATOMIC_TT_PROMOTION(int,double,double);
	SCEAATOMIC_TT_PROMOTION(int,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(unsigned int,long,long);
	SCEAATOMIC_TT_PROMOTION(unsigned int,unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(unsigned int,long long,long long);
	SCEAATOMIC_TT_PROMOTION(unsigned int,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(unsigned int,float,float);
	SCEAATOMIC_TT_PROMOTION(unsigned int,double,double);
	SCEAATOMIC_TT_PROMOTION(unsigned int,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(long,long);
	SCEAATOMIC_TT_PROMOTION(long,unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(long,long long,long long);
	SCEAATOMIC_TT_PROMOTION(long,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(long,float,float);
	SCEAATOMIC_TT_PROMOTION(long,double,double);
	SCEAATOMIC_TT_PROMOTION(long,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(unsigned long,long long,long long);
	SCEAATOMIC_TT_PROMOTION(unsigned long,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(unsigned long,float,float);
	SCEAATOMIC_TT_PROMOTION(unsigned long,double,double);
	SCEAATOMIC_TT_PROMOTION(unsigned long,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(long long,long long);
	SCEAATOMIC_TT_PROMOTION(long long,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(long long,float,float);
	SCEAATOMIC_TT_PROMOTION(long long,double,double);
	SCEAATOMIC_TT_PROMOTION(long long,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(unsigned long long,float,float);
	SCEAATOMIC_TT_PROMOTION(unsigned long long,double,double);
	SCEAATOMIC_TT_PROMOTION(unsigned long long,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(float,float);
	SCEAATOMIC_TT_PROMOTION(float,double,double);
	SCEAATOMIC_TT_PROMOTION(float,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(double,double);
	SCEAATOMIC_TT_PROMOTION(double,long double,long double);
	
	SCEAATOMIC_TT_PROMOTION_SAME(long double,long double);
	
	// wchar_t is implementation-dependent, so we need to promote it accordingly
#if WCHAR_MAX == SHRT_MAX
	#define WCHAR_PROMOTED_TYPE int
	SCEAATOMIC_TT_PROMOTION(wchar_t,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(wchar_t,long,long);
#elif WCHAR_MAX == USHRT_MAX
	#define WCHAR_PROMOTED_TYPE int
	SCEAATOMIC_TT_PROMOTION(wchar_t,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(wchar_t,long,long);
#elif WCHAR_MAX == INT_MAX
	#define WCHAR_PROMOTED_TYPE int
	SCEAATOMIC_TT_PROMOTION(wchar_t,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(wchar_t,long,long);
#elif WCHAR_MAX == UINT_MAX
	#define WCHAR_PROMOTED_TYPE unsigned int
	SCEAATOMIC_TT_PROMOTION(wchar_t,unsigned int,unsigned int);
	SCEAATOMIC_TT_PROMOTION(wchar_t,long,long);
#elif WCHAR_MAX == LONG_MAX
	#define WCHAR_PROMOTED_TYPE long
	SCEAATOMIC_TT_PROMOTION(wchar_t,unsigned int,long);
	SCEAATOMIC_TT_PROMOTION(wchar_t,long,long);
#elif WCHAR_MAX == ULONG_MAX
	#define WCHAR_PROMOTED_TYPE unsigned long
	SCEAATOMIC_TT_PROMOTION(wchar_t,unsigned int,unsigned long);
	SCEAATOMIC_TT_PROMOTION(wchar_t,long,unsigned long);
#elif !SCEAATOMIC_INTEGER_WCHAR
#else
	#error what size is wchar_t?
#endif
	
#if SCEAATOMIC_INTEGER_WCHAR
	SCEAATOMIC_TT_PROMOTION_SAME(wchar_t,WCHAR_PROMOTED_TYPE);
	SCEAATOMIC_TT_PROMOTION(wchar_t,bool,WCHAR_PROMOTED_TYPE);
	SCEAATOMIC_TT_PROMOTION(wchar_t,char,WCHAR_PROMOTED_TYPE);
	SCEAATOMIC_TT_PROMOTION(wchar_t,signed char,WCHAR_PROMOTED_TYPE);
	SCEAATOMIC_TT_PROMOTION(wchar_t,unsigned char,WCHAR_PROMOTED_TYPE);
	SCEAATOMIC_TT_PROMOTION(wchar_t,short,WCHAR_PROMOTED_TYPE);
	SCEAATOMIC_TT_PROMOTION(wchar_t,unsigned short,WCHAR_PROMOTED_TYPE);
	SCEAATOMIC_TT_PROMOTION(wchar_t,int,WCHAR_PROMOTED_TYPE);
	// see above
	SCEAATOMIC_TT_PROMOTION(wchar_t,unsigned long,unsigned long);
	SCEAATOMIC_TT_PROMOTION(wchar_t,long long,long long);
	SCEAATOMIC_TT_PROMOTION(wchar_t,unsigned long long,unsigned long long);
	SCEAATOMIC_TT_PROMOTION(wchar_t,float,float);
	SCEAATOMIC_TT_PROMOTION(wchar_t,double,double);
	SCEAATOMIC_TT_PROMOTION(wchar_t,long double,long double);
#endif
	
	// Convert a type to the unsigned equivalent with the same number of bits.
	template <typename T>
	struct make_unsigned { typedef T type; };
	
	template <> struct make_unsigned<char>			{ typedef unsigned char type; };
	template <> struct make_unsigned<signed char>	{ typedef unsigned char type; };
	template <> struct make_unsigned<short>			{ typedef unsigned short type; };
	template <> struct make_unsigned<int>			{ typedef unsigned int type; };
	template <> struct make_unsigned<long>			{ typedef unsigned long type; };
	template <> struct make_unsigned<long long>		{ typedef unsigned long long type; };
} // namespace detail

#if SCEA_TARGET_CPU_PPC
	#define LOAD_RESERVED(type, base)			 (sizeof(type) == 8 ? __ldarx(base) : __lwarx(base))
// The operand order for the stores was changed in SDK 0.9.0.
#if defined(__CELLOS_LV2__) && (CELL_SDK_VERSION >= 0x090000)
	#define STORE_CONDITIONAL(type, value, base) (sizeof(type) == 8 ? __stdcx(base, value) : __stwcx(base, value))
#else
	#define STORE_CONDITIONAL(type, value, base) (sizeof(type) == 8 ? __stdcx(value, base) : __stwcx(value, base))
#endif

	#define ASSIGN_OP(type, op)	ASSIGN_OP_LLSC(type, op)
	#define PREFIX_OP(type, op, half_op) PREFIX_OP_LLSC(type, op, half_op)
	#define POSTFIX_OP(type, op, half_op) POSTFIX_OP_LLSC(type, op, half_op)
#elif SCEA_TARGET_CPU_CELL_SPU
	// atomic_integer is mostly empty and all work is done in the main_value specialization
	#define ASSIGN_OP(type, op)
	#define PREFIX_OP(type, op, half_op)
	#define POSTFIX_OP(type, op, half_op)
#elif SCEA_TARGET_CPU_X86 || SCEA_TARGET_CPU_MIPS
	#define ASSIGN_OP(type, op)	ASSIGN_OP_CAS(type, op)
	#define PREFIX_OP(type, op, half_op) PREFIX_OP_CAS(type, op, half_op)
	#define POSTFIX_OP(type, op, half_op) POSTFIX_OP_CAS(type, op, half_op)
#else
	#error CPU not supported
#endif

#define ASSIGN_OP_LLSC(type, op)									\
	template <typename U> atomic_integer<type>& operator op ## = (U rhs)	\
	{																\
		type new_value;												\
		do {														\
			new_value = LOAD_RESERVED(type, &mValue) op rhs;		\
		} while (!STORE_CONDITIONAL(type, new_value, &mValue));		\
		return *this;												\
	}

#define PREFIX_OP_LLSC(type, op, half_op)							\
	atomic_integer<type>& operator op (void)						\
	{																\
		type new_value;												\
		do {														\
			new_value = LOAD_RESERVED(type, &mValue) half_op 1;		\
		} while (!STORE_CONDITIONAL(type, new_value, &mValue));		\
		return *this;												\
	}

#define POSTFIX_OP_LLSC(type, op, half_op)								\
	type operator op (int)												\
	{																	\
		type new_value;													\
		do {															\
			new_value = LOAD_RESERVED(type, &mValue);					\
		} while (!STORE_CONDITIONAL(type, new_value half_op 1, &mValue));\
		return new_value;												\
	}

#define ASSIGN_OP_CAS(type, op)															\
	template <typename U> atomic_integer<type>& operator op ## = (U rhs)						\
	{																							\
		type old_value, previous_value;															\
		do {																					\
			old_value = mValue;																	\
			previous_value = SCEA::Atomic::CompareAndSwap( &mValue, old_value, mValue op rhs ); \
		} while( old_value != previous_value );													\
		return *this;																			\
	}

#define PREFIX_OP_CAS(type, op, half_op)														\
	atomic_integer<type>& operator op (void)														\
	{																								\
		type old_value, previous_value;																\
		do {																						\
			old_value = mValue;																		\
			previous_value = SCEA::Atomic::CompareAndSwap( &mValue, old_value, mValue half_op 1 );	\
		} while( old_value != previous_value );														\
		return *this;																				\
	}

#define POSTFIX_OP_CAS(type, op, half_op)														\
	type operator op (int)																			\
	{																								\
		type old_value, previous_value;																\
		do {																						\
			old_value = mValue;																		\
			previous_value = SCEA::Atomic::CompareAndSwap( &mValue, old_value, mValue half_op 1 );	\
		} while( old_value != previous_value );														\
		return previous_value;																		\
	}

template <typename T>
struct atomic_integer
{
	// fail to instantiate for non-integral types like float/pointer
	SCEAATOMIC_STATIC_ASSERT(detail::is_integral<T>::value && (sizeof(T) == 8 || sizeof(T) == 4));
	
	atomic_integer()					: mValue(0)		{ SCEAATOMIC_WRITE_BARRIER(); }
	atomic_integer(const T& rhs)		: mValue(rhs)	{ SCEAATOMIC_WRITE_BARRIER(); }
	atomic_integer<T>& operator=(const T& rhs)	{ mValue = rhs; SCEAATOMIC_WRITE_BARRIER(); return *this; }

	operator T (void) { T result = mValue; SCEAATOMIC_READ_BARRIER(); return result; }
	
	ASSIGN_OP(T, +)
	ASSIGN_OP(T, -)
	ASSIGN_OP(T, *)
	ASSIGN_OP(T, /)
	ASSIGN_OP(T, %)
	ASSIGN_OP(T, ^)
	ASSIGN_OP(T, &)
	ASSIGN_OP(T, |)
	ASSIGN_OP(T, <<)
	ASSIGN_OP(T, >>)

	PREFIX_OP(T, ++, +)
	PREFIX_OP(T, --, -)
	POSTFIX_OP(T, ++, +)
	POSTFIX_OP(T, --, -)
	
	volatile T	mValue;
};

#undef ASSIGN_OP
#undef ASSIGN_OP_LLSC
#undef PREFIX_OP_LLSC
#undef POSTFIX_OP_LLSC
#undef LOAD_RESERVED
#undef STORE_CONDITIONAL

#if SCEA_TARGET_CPU_CELL_SPU

namespace detail {
	// unsigned variants only
	template <typename T> T add_function(T lhs, T rhs);
	template <typename T> T sub_function(T lhs, T rhs);
	template <typename T> T xor_function(T lhs, T rhs);
	template <typename T> T and_function(T lhs, T rhs);
	template <typename T> T or_function(T lhs, T rhs);
	template <typename T> T lshift_function(T lhs, T rhs);

	// use both signed and unsigned variants
	template <typename T> T mul_function(T lhs, T rhs);
	template <typename T> T div_function(T lhs, T rhs);
	template <typename T> T mod_function(T lhs, T rhs);
	template <typename T> T rshift_function(T lhs, T rhs);

	typedef unsigned int (binary_func_uint)(unsigned int, unsigned int);
	typedef unsigned long (binary_func_ulong)(unsigned long, unsigned long);
	typedef unsigned long long (binary_func_ulonglong)(unsigned long long, unsigned long long);
	
	// the unsigned long variants aren't strictly necessary if the sceabasetypes are used, but they'll be dead-stripped if not used
	unsigned int		do_atomic_binary_op(unsigned int rhs, U64 lhs_main_addr, binary_func_uint binary_func);
	unsigned long		do_atomic_binary_op(unsigned long rhs, U64 lhs_main_addr, binary_func_ulong binary_func);
	unsigned long long	do_atomic_binary_op(unsigned long long rhs, U64 lhs_main_addr, binary_func_ulonglong binary_func);
	unsigned int		do_atomic_binary_postfix_op(unsigned int rhs, U64 lhs_main_addr, binary_func_uint binary_func);
	unsigned long		do_atomic_binary_postfix_op(unsigned long rhs, U64 lhs_main_addr, binary_func_ulong binary_func);
	unsigned long long	do_atomic_binary_postfix_op(unsigned long long rhs, U64 lhs_main_addr, binary_func_ulonglong binary_func);
}

// On SPU, we use a main_ptr<atomic_integer<T> > to refer to an atomic_integer in main memory.
// main_ptr's dereference operator combined with the value-like semantics of main_value
// allow us to form expressions like *pCounter += 5 and use a minimum of DMA traffic.
template <typename T>
struct main_value<atomic_integer<T> >
{
	typedef main_value<atomic_integer<T> >	this_type;
	typedef atomic_integer<T>				value_type;
	
	main_value(const main_ptr<value_type> p) : m_p(p) {}		// constructor used by main_ptr's deref operator
	
	// These DMA helpers should be used sparingly!
	operator T (void) { return m_p.get(); }
	this_type& operator = (const T& rhs) { m_p.put(rhs); return *this; }

	// These macro assists help us break down the messy type algebra we need to do.
	//
	// BARE:		means a type without const or volatile on it
	// PROMOTED:	applies standard C type promotion rules, as if it were giving us the type
	//				that would result from adding two differently typed values
	// UNSIGNED:	gives us the unsigned version of a type, which we use because many operations
	//				are the same at the instruction level for both signed and unsigned types.
	#define BARE_T				typename detail::remove_cv<T>::type
	#define BARE_U				typename detail::remove_cv<U>::type
	#define PROMOTED			typename detail::promote_traits<BARE_T, BARE_U>::promote_type	
	#define UNSIGNED_PROMOTED	typename detail::make_unsigned<PROMOTED>::type
	
	// The operator overloads call into a set of common atomic functions, passing in a function that
	// performs the actual operation. All this type algebra helps us reduce the number of atomic
	// functions we have because they're quite large compared to the equivalent functionality on the
	// PPU.
	#define UNSIGNED_ASSIGN_OP(op, func)											\
		template <typename U> PROMOTED operator op ## = (U rhs)						\
		{																			\
			return detail::do_atomic_binary_op(static_cast<UNSIGNED_PROMOTED>(rhs),	\
											   U32ToMainAddr(m_p.get_ptr()),		\
											   &detail::func<UNSIGNED_PROMOTED>);	\
		}

	// For operators with both signed and unsigned variants, we call through the unsigned atomic knowing
	// that it doesn't munge the bytes on the way to the function we're passing in. The extra static_cast
	// in there is to help the compiler know exactly which function we're asking for.
	#define SIGNED_ASSIGN_OP(op, func)															\
		template <typename U> PROMOTED operator op ## = (U rhs)									\
		{																						\
			typedef PROMOTED (binary_func)(PROMOTED, PROMOTED);									\
			typedef UNSIGNED_PROMOTED (un_binary_func)(UNSIGNED_PROMOTED, UNSIGNED_PROMOTED);	\
			return detail::do_atomic_binary_op(static_cast<UNSIGNED_PROMOTED>(rhs),				\
											   U32ToMainAddr(m_p.get_ptr()),					\
											   reinterpret_cast<un_binary_func*>(static_cast<binary_func*>(&detail::func<PROMOTED>))); \
		}
	
	UNSIGNED_ASSIGN_OP(+, add_function)
	UNSIGNED_ASSIGN_OP(-, sub_function)
	UNSIGNED_ASSIGN_OP(^, xor_function)
	UNSIGNED_ASSIGN_OP(&, and_function)
	UNSIGNED_ASSIGN_OP(|, or_function)
	UNSIGNED_ASSIGN_OP(<<, lshift_function)
	SIGNED_ASSIGN_OP(*, mul_function)
	SIGNED_ASSIGN_OP(/, div_function)
	SIGNED_ASSIGN_OP(%, mod_function)
	SIGNED_ASSIGN_OP(>>, rshift_function)
		
	// Prefix operators are defined in terms of the binary operators using 1 as the rhs.
	// Since the type of the constant rhs is int, we need to fill in the U template
	// parameter as int by inserting it into our macro expansions.
#define U int
	PROMOTED operator ++ (void)
	{
		return detail::do_atomic_binary_op(static_cast<UNSIGNED_PROMOTED>(1),
										   U32ToMainAddr(m_p.get_ptr()),
										   &detail::add_function<UNSIGNED_PROMOTED>);
	}

	PROMOTED operator -- (void)
	{
		return detail::do_atomic_binary_op(static_cast<UNSIGNED_PROMOTED>(1),
										   U32ToMainAddr(m_p.get_ptr()),
										   &detail::sub_function<UNSIGNED_PROMOTED>);
	}
	
	// Postfix operators are similar to the prefix ones, but need to use a different atomic.
	PROMOTED operator ++ (int)
	{
		return detail::do_atomic_binary_postfix_op(static_cast<UNSIGNED_PROMOTED>(1),
												   U32ToMainAddr(m_p.get_ptr()),
												   &detail::add_function<UNSIGNED_PROMOTED>);
	}

	PROMOTED operator -- (int)
	{
		return detail::do_atomic_binary_postfix_op(static_cast<UNSIGNED_PROMOTED>(1),
												   U32ToMainAddr(m_p.get_ptr()),
												   &detail::sub_function<UNSIGNED_PROMOTED>);
	}
#undef U
	
	main_ptr<value_type>	m_p;
};
#endif

}} // namespace SCEA::Atomic

#endif // __SCEAATOMIC_INTEGER_H__
