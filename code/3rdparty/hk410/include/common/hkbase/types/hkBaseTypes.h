/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKBASE_HKBASETYPES_H
#define HKBASE_HKBASETYPES_H

//
// compiler
//

#if defined(_MSC_VER)
#	define HK_COMPILER_MSVC
#elif defined(__SNC__)
#	define HK_COMPILER_SNC
#elif defined(__GNUC__)
#	define HK_COMPILER_GCC
#elif defined(__MWERKS__)
#	define HK_COMPILER_MWERKS
#elif defined(__INTEL_COMPILER)
#	define HK_COMPILER_INTEL
#else
#	error Could not detect compiler
#endif

//
// architecture
//

#if defined(__i386__) || defined(_M_IX86)
#	define HK_ARCH_IA32
#	define HK_ENDIAN_LITTLE 1
#	define HK_ENDIAN_BIG	0
#	define HK_POINTER_SIZE 4
#elif defined(_M_AMD64) || defined(_M_X64)
#	define HK_ARCH_X64
#	define HK_ENDIAN_LITTLE 1
#	define HK_ENDIAN_BIG	0
#	define HK_POINTER_SIZE 8
#elif defined( R3000 ) || defined( __R4000__ )
#	define HK_ARCH_PSP
#	define HK_ENDIAN_LITTLE 1
#	define HK_ENDIAN_BIG	0
#	define HK_POINTER_SIZE 4
#elif (!defined( R3000 )) && (!defined( __R4000__ )) && (defined( __mips__ ) || defined( __MIPS__ ))
#	define HK_ARCH_PS2
#	define HK_ENDIAN_LITTLE 1
#	define HK_ENDIAN_BIG	0
#	define HK_POINTER_SIZE 4
#elif defined(_PPC_) || defined(__POWERPC__) || defined(_M_PPC) || defined(_M_PPCBE) || defined(GEKKO)
#	define HK_ARCH_PPC
#	define HK_ENDIAN_LITTLE 0
#	define HK_ENDIAN_BIG	1
#	define HK_POINTER_SIZE 4
#elif defined(__PPU__) && defined(__CELLOS_LV2__)
#	define HK_ARCH_PS3
#	define HK_ENDIAN_LITTLE 0
#	define HK_ENDIAN_BIG	1
#elif defined(__SPU__) && defined(__CELLOS_LV2__)
#	define HK_ARCH_PS3SPU
#	define HK_ENDIAN_LITTLE 0
#	define HK_ENDIAN_BIG	1
#else
#	error Could not autodetect target architecture
#endif

#if defined(HK_ARCH_PS3) || defined(HK_ARCH_PS3SPU)
# include <sdk_version.h>
# define HK_CELL_SDK_VERSION CELL_SDK_VERSION
# if ( HK_CELL_SDK_VERSION < 0x080000 )
#	  define HK_POINTER_SIZE 8 // Caution: On SPU the pointer size is 4, but usually where this is used pointers will be "shadows" from the PPU
#	else
#	  define HK_POINTER_SIZE 4
#	endif
#endif
//
// platform
//

#if defined(_XBOX)
#   if defined(HK_ARCH_PPC)
#	  define HK_PLATFORM_XBOX360
#	else
#	  define HK_PLATFORM_XBOX
#   endif
#	define HK_PLATFORM_IS_CONSOLE 1
#elif defined(__APPLE_CC__)
#	define HK_PLATFORM_DARWIN
#	define HK_PLATFORM_IS_CONSOLE 0
#elif defined(_WIN32)
#	define HK_PLATFORM_WIN32
#	if defined(_WIN64)
#		define HK_PLATFORM_X64
#	endif
#	define HK_PLATFORM_IS_CONSOLE 0
#elif (!defined( R3000 )) && (!defined( __R4000__ )) && (defined( __mips__ ) || defined( __MIPS__ ))
#	define HK_PLATFORM_PS2
#	define HK_PS2
#	define HK_PLATFORM_IS_CONSOLE 1
#elif defined(__unix__)
#	define HK_PLATFORM_UNIX
#	define HK_PLATFORM_IS_CONSOLE 0
#elif defined(GEKKO) || defined(__PPCGEKKO__) //Also have custom added HK_REVOLUTION compiler switch
#	define HK_PLATFORM_GC
#if defined(RVL_OS)
#	define HK_PLATFORM_RVL
#endif
#	define HK_PLATFORM_IS_CONSOLE 1
#elif defined(__POWERPC__)
#	define HK_PLATFORM_MAC
#	define HK_PLATFORM_IS_CONSOLE 0
#elif defined( R3000 ) || defined( __R4000__ )
#	define HK_PLATFORM_PSP
#	define HK_PLATFORM_IS_CONSOLE 1
#elif defined(__PPU__) && defined(__CELLOS_LV2__)
#	define HK_PLATFORM_PS3
#	define HK_PLATFORM_IS_CONSOLE 1
#elif defined(__SPU__) && defined(__CELLOS_LV2__)
#	define HK_PLATFORM_PS3SPU
#	define HK_PLATFORM_IS_CONSOLE 1
#else
#	error Could not autodetect target platform.
#endif

//
// types
//

/// hkReal is the default floating point type.
typedef float  hkReal;
/// hkFloat is provided if floats are explicitly required.
typedef float  hkFloat32;
/// hkDouble is provided if doubles are explicit required.
typedef double hkDouble64;


/// Signed 8 bit integer
typedef signed char		hkChar;
/// Signed 8 bit integer
typedef signed char		hkInt8;
/// Signed 16 bit integer
typedef signed short	hkInt16;
/// Signed 32 bit integer
typedef signed int		hkInt32;

/// Unsigned 8 bit integer
typedef unsigned char	hkUchar;
/// Unsigned  8 bit integer
typedef unsigned char	hkUint8;
/// Unsigned  16 bit integer
typedef unsigned short	hkUint16;
/// Unsigned  32 bit integer
typedef unsigned int	hkUint32;

/// An integer type guaranteed to be the same size as a pointer.
#if defined(HK_ARCH_PS2)
	typedef unsigned int hkUlong;
#elif defined(HK_ARCH_PSP)
	typedef unsigned int hkUlong;
#elif defined(HK_ARCH_X64)
	typedef unsigned __int64 hkUlong;
#elif defined(HK_COMPILER_MSVC) && (_MSC_VER >= 1300)
	typedef unsigned long __w64 hkUlong; // VC7.0 or higher, 64bit warnings
#else
	typedef unsigned long hkUlong;
#endif

#if defined(HK_PLATFORM_PS3SPU) && (HK_CELL_SDK_VERSION < 0x080000)
#	define HK_CPU_PTR( A ) hkUint64
#	define HK_INDEX_CPU_PTR( A, INDEX, TYPE ) ( (A) + (INDEX) * sizeof(TYPE) )
#else
#	define HK_CPU_PTR( A ) A
#	define HK_INDEX_CPU_PTR( A, INDEX, TYPE ) ( (A) + INDEX )
#endif

typedef void* hk_va_list;

/// a simple success/failure enum.
enum hkResult
{
	HK_SUCCESS = 0,
	HK_FAILURE = 1
};

#if defined( HK_PLATFORM_PS3SPU) 
#	include <spu_intrinsics.h>
#endif

//
// useful macros
//

#if  defined(DEBUG) || defined(_DEBUG)
#	undef HK_DEBUG
#	define HK_DEBUG
#	define hkDebug 1
#else
#	define hkDebug 0
#endif

#if defined(HK_ARCH_IA32)
#	if defined(HK_COMPILER_MSVC)
#		define HK_BREAKPOINT() __asm { int 3 }
#	elif defined(HK_COMPILER_GCC)
#		define HK_BREAKPOINT() asm("int $3");
#	else
#		error unknown asm syntax
#	endif
#elif defined(HK_PLATFORM_PS3)
#	define HK_BREAKPOINT() 	__asm__ volatile ( "tw 31,1,1" )
#else
#	define HK_BREAKPOINT() ((*((int*)0)) = 0)
#endif

#define HK_NULL 0

// use the compiler friendly but programmer ugly version for release only
#ifdef HK_DEBUG
#	define HK_MULTILINE_MACRO_BEGIN	do {
#	define HK_MULTILINE_MACRO_END		} while(0)
#else
#	define HK_MULTILINE_MACRO_BEGIN	if(1) {
#	define HK_MULTILINE_MACRO_END		} else
#endif

/// Note that ALIGNMENT must be a power of two for this to work.
/// Note: to use this macro you must cast your pointer to a byte pointer or to an integer value.
#define HK_NEXT_MULTIPLE_OF(ALIGNMENT, VALUE)  ( ((VALUE) + ((ALIGNMENT)-1)) & (~((ALIGNMENT)-1)) )

/// The offset of a member within a structure
#define HK_OFFSET_OF(CLASS,MEMBER) int(reinterpret_cast<hkUlong>(&(reinterpret_cast<CLASS*>(16)->MEMBER))-16)

/// A check for whether the offset of a member within a structure is as expected
#define HK_OFFSET_EQUALS(CLASS,MEMBER,OFFSET) (HK_OFFSET_OF(CLASS,MEMBER)==OFFSET)

/// Join two preprocessor tokens, even when a token is itself a macro.
#define HK_PREPROCESSOR_JOIN_TOKEN(A,B) HK_PREPROCESSOR_JOIN_TOKEN2(A,B)
#define HK_PREPROCESSOR_JOIN_TOKEN2(A,B) HK_PREPROCESSOR_JOIN_TOKEN3(A,B)
#define HK_PREPROCESSOR_JOIN_TOKEN3(A,B) A##B

//
// compiler specific settings
//


	// *************************************
	//			GCC and SN
	// *************************************
#if defined(HK_COMPILER_GCC) || defined(HK_COMPILER_SNC)
#   if defined(__GNUC_PATCHLEVEL__)
#   	define HK_COMPILER_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 +__GNUC_PATCHLEVEL__)
#	else
#		define HK_COMPILER_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100)
#   endif
#	if (HK_COMPILER_GCC_VERSION >= 40000)
#		undef HK_OFFSET_OF
#		define HK_OFFSET_OF(CLASS,MEMBER) __builtin_offsetof(CLASS,MEMBER)
#	elif ( HK_COMPILER_GCC_VERSION >= 30400 ) && !defined( HK_COMPILER_SNC )
#		undef HK_OFFSET_OF
#		define HK_OFFSET_OF(CLASS,MEMBER) (__offsetof__(reinterpret_cast<long>(&reinterpret_cast<CLASS*>(16)->MEMBER)-16))
#	endif
#	if ( HK_COMPILER_GCC_VERSION >= 40000 ) && !defined( HK_COMPILER_SNC )
#		undef HK_OFFSET_EQUALS
#		define HK_OFFSET_EQUALS(C,M,O) true
#	endif
#	define HK_ALIGN(DECL, ALIGNMENT) __attribute__((aligned(ALIGNMENT))) DECL
#	define HK_ALIGN16(DECL) __attribute__((aligned(16))) DECL
#   if defined(HK_PLATFORM_PS2)
	    typedef unsigned long hkUint64;	// long is 64 bits on PS2
	    typedef long hkInt64;
		typedef int hkInt128 __attribute__ ((mode (TI)));
		typedef unsigned int hkUint128 __attribute__ ((mode (TI)));
#   elif defined(HK_PLATFORM_PS3)
#     if ( HK_CELL_SDK_VERSION < 0x080000 )
		    typedef unsigned long hkUint64;
		    typedef long hkInt64;
#     else
		    typedef unsigned long long hkUint64;
		    typedef long long hkInt64;
#     endif
#   elif defined(HK_PLATFORM_PS3SPU)
		typedef unsigned long long hkUint64;
		typedef long long hkInt64;
#   else
	    typedef unsigned long long hkUint64;
	    typedef long long hkInt64;
#   endif
#	if defined(HK_PLATFORM_GC)
		typedef hkUint64 hkSystemTime; // GameCube Time ticks are 64 bit
#	else
		typedef long hkSystemTime;
#	endif
#	if defined(HK_PLATFORM_DARWIN) || ( ( defined(HK_PLATFORM_PS3) && ( HK_CELL_SDK_VERSION < 0x080000 ) ) || defined (HK_PLATFORM_PS3SPU) )
		typedef unsigned long hk_size_t;
#	else
		typedef unsigned hk_size_t;
#	endif
#	if defined(HK_ARCH_IA32) && HK_COMPILER_GCC_VERSION >= 30100
#		define HK_COMPILER_HAS_INTRINSICS_IA32
#	endif
#	if defined(HK_ARCH_PS2) && HK_COMPILER_GCC_VERSION >= 30003 && !defined(HK_COMPILER_SNC)
#		define HK_COMPILER_HAS_INTRINSICS_PS2
#	endif
#	define HK_FORCE_INLINE inline
// calling convention
#	define HK_CALL
#	define HK_FAST_CALL


	// *************************************
	//			MICROSOFT and INTEL
	// *************************************
#elif defined(HK_COMPILER_MSVC) || defined(HK_COMPILER_INTEL)
#	define HK_COMPILER_SUPPORTS_PCH
#	define HK_COMPILER_MSVC_VERSION _MSC_VER
#	define HK_COMPILER_INTEL_VERSION _MSC_VER
#	pragma warning( disable : 4786 ) // Debug tuncated to 255:
#	pragma warning( disable : 4530 ) // C++ Exception handler used but not enabled:(used in <xstring>)
#	if defined(HK_PLATFORM_XBOX) || defined(HK_PLATFORM_XBOX360) || defined(HK_PLATFORM_WIN32)
#		define HK_ALIGN(DECL, ALIGNMENT) __declspec(align(ALIGNMENT)) DECL
#		define HK_ALIGN16(DECL) __declspec(align(16)) DECL
#		define HK_FORCE_INLINE __forceinline
#	elif defined(HK_PLATFORM_UNIX)
#		define HK_ALIGN(DECL, ALIGNMENT) __attribute__((aligned(ALIGNMENT))) DECL
#		define HK_ALIGN16(DECL) __attribute__((aligned(16))) DECL
#		define HK_FORCE_INLINE inline
#	else
#		error "fix the alignment on this platform"
#	endif
#	if defined(HK_ARCH_IA32)
		typedef unsigned __int64 hkUint64;
		typedef __int64 hkInt64;
		typedef long hkSystemTime;
#		if defined(HK_COMPILER_MSVC) && (_MSC_VER >= 1300)
			typedef unsigned __w64 hk_size_t; // VC7.0 or higher, 64bit warnings
#		else
			typedef unsigned hk_size_t;
#		endif
#		define HK_COMPILER_HAS_INTRINSICS_IA32
#	elif defined( HK_ARCH_PPC )
		typedef unsigned __int64 hkUint64;
		typedef __int64 hkInt64;
		typedef unsigned hk_size_t;
		typedef __int64 hkSystemTime; // 64bit time
#		define HK_COMPILER_HAS_INTRINSICS_PPC
#	elif defined(HK_ARCH_X64)
		typedef unsigned __int64 hkUint64;
		typedef __int64 hkInt64;
		typedef __int64 hkSystemTime;
		typedef unsigned __int64 hk_size_t;
#		define HK_COMPILER_HAS_INTRINSICS_IA32
#	else
#		error No defs for this architecture
#	endif
// calling convention
#	define HK_CALL __cdecl
#	define HK_FAST_CALL __fastcall

	// *************************************
	//			METROWERKS
	// *************************************
#elif defined(HK_COMPILER_MWERKS)
#	define HK_ALIGN(DECL, ALIGNMENT) DECL __attribute__((aligned(ALIGNMENT)))
#	define HK_ALIGN16(DECL) DECL __attribute__((aligned(16)))
#   if defined(HK_PLATFORM_PS2)
		typedef unsigned long hkUint64;	// long is 64 bits on PS2
		typedef long hkInt64;
#   endif
#	if defined(HK_PLATFORM_PSP)
		typedef unsigned long long hkUint64;	// long long is 64 bits on PSP
		typedef long long hkInt64;
#	endif
#   if defined(HK_PLATFORM_PS2) || defined(HK_PLATFORM_PSP)
		typedef long hkSystemTime;
		typedef unsigned hk_size_t;
		typedef __int128 hkInt128;
		typedef unsigned __int128 hkUint128;
#   else
		typedef unsigned long long hkUint64;
		typedef long long hkInt64;
#		if defined(HK_PLATFORM_GC)
			typedef hkUint64 hkSystemTime; // GameCube Time ticks are 64 bit
#		else
			typedef unsigned long hkSystemTime;
#		endif
		typedef unsigned long hk_size_t;
#   endif
#	define HK_FORCE_INLINE inline
// calling convention
#	define HK_CALL
#	define HK_FAST_CALL

#else
#	error Unknown compiler
#endif // compilers

#if defined(HK_PLATFORM_PS3) || defined(HK_PLATFORM_PS3SPU)
#	define HK_ALWAYS_INLINE __attribute__((always_inline)) inline
#	if !defined (HK_DEBUG)
#		define HK_LOCAL_INLINE inline
#	else
#		define HK_LOCAL_INLINE
#	endif
#else
#	define HK_ALWAYS_INLINE HK_FORCE_INLINE
#	define HK_LOCAL_INLINE HK_FORCE_INLINE
#endif


typedef hkUint16 hkObjectIndex;
typedef hkReal hkTime;

#define HK_INVALID_OBJECT_INDEX 0xffff


HK_FORCE_INLINE hkInt32 HK_CALL hkPointerToInt32( const void* ptr )
{
	return static_cast<int>( hkUlong(ptr) );
}

/// get the byte offset of B - A, as a full ulong.
HK_FORCE_INLINE hkUlong HK_CALL hkGetByteOffset( const void* base, const void* pntr)
{
	return hkUlong(pntr) - hkUlong(base);
}

/// get the byte offset of B - A, as an int (64bit issues, so here for easy code checks)
HK_FORCE_INLINE int HK_CALL hkGetByteOffsetInt( const void* base, const void* pntr)
{
	return static_cast<int>( hkGetByteOffset( base, pntr ) );
}

/// get the byte offset of B - A, as a full 64bit hkUint64.
HK_FORCE_INLINE hkUint32 HK_CALL hkGetByteOffsetCpuPtr( const HK_CPU_PTR(void*) base, const HK_CPU_PTR(void*) pntr)
{
	return hkUint32(hkUlong((HK_CPU_PTR(const char*))(pntr) - (HK_CPU_PTR(const char*))(base)));
}

template <typename TYPE>
HK_FORCE_INLINE TYPE* HK_CALL hkAddByteOffset( TYPE* base, hkUlong offset )
{
	return reinterpret_cast<TYPE*>( reinterpret_cast<char*>(base) + offset );
}

template <typename TYPE>
HK_FORCE_INLINE TYPE HK_CALL hkAddByteOffsetCpuPtr( TYPE base, hkUlong offset )
{
	return reinterpret_cast<TYPE>( reinterpret_cast<char*>(base) + offset );
}

template <typename TYPE>
HK_FORCE_INLINE const TYPE* HK_CALL hkAddByteOffsetConst( const TYPE* base, hkUlong offset )
{
	return reinterpret_cast<const TYPE*>( reinterpret_cast<const char*>(base) + offset );
}

	/// If you have a pair of pointers and you have one pointer, than this function allows you to quickly get the other pointer of the pair.
template <typename TYPE>
HK_FORCE_INLINE TYPE* HK_CALL hkSelectOther( TYPE* a, TYPE* pairA, TYPE* pairB )
{
	return reinterpret_cast<TYPE*>( hkUlong(a) ^ hkUlong(pairA) ^ hkUlong(pairB) );
}


#ifdef HK_PLATFORM_XBOX360
#	define HK_RESTRICT __restrict
#else
#	define HK_RESTRICT
#endif




#define hkSizeOf(A) int(sizeof(A))

#define HK_DECLARE_REFLECTION() \
	static const struct hkInternalClassMember Members[]; \
	struct DefaultStruct

class hkClass;

/// A generic object with metadata.
struct hkVariant
{
	void* m_object;
	const hkClass* m_class;
};

/// A wrapper to store a hkBool in one byte, regardless of compiler options.
class hkBool
{
	public:

		inline hkBool()
		{
		}

		inline hkBool(bool b)
		{
			m_bool = static_cast<char>(b);
		}

		inline operator bool() const
		{
			return m_bool != 0;
		}

		inline hkBool& operator=(bool e)
		{
			m_bool = static_cast<char>(e);
			return *this;
		}

		inline hkBool operator==(bool e) const
		{
			return static_cast<int>(m_bool) == static_cast<int>(e);
		}

		inline hkBool operator!=(bool e) const
		{
			return static_cast<int>(m_bool) != static_cast<int>(e);
		}

	private:
		char m_bool;
};

    /// A wrapper to store a float in 16 bit. This is a non ieee representation.
    /// Basically we simply chop of the last 16 bit. That means the whole floating point range
    /// will be supported, but only with 7 bit precision
class hkHalf
{
    public:
    
	    inline hkHalf()
	    {
	    }
    
	    inline hkHalf(const float& f)
	    {
		    int t = ((int*)&f)[0];
		    m_value = hkInt16(t>>16);
	    }
    
	    inline hkHalf& operator=(const float& f)
	    {
		    int t = ((int*)&f)[0];
		    m_value = hkInt16(t>>16);
		    return *this;
	    }
    
	    inline operator float() const
	    {
		    union
		    {
			    int i;
			    float f;
		    } u;
		    u.i = (m_value <<16);
		    return u.f;
	    }
    
    private:
	    hkInt16 m_value;
};



template<typename ENUM, typename STORAGE>
class hkEnum
{
	public:

		hkEnum()
		{
		}

		hkEnum(ENUM e)
		{
			m_storage = static_cast<STORAGE>(e);
		}

		operator ENUM() const
		{
			return static_cast<ENUM>(m_storage);
		}
		void operator=(ENUM e)
		{
			m_storage = static_cast<STORAGE>(e);
		}
		hkBool operator==(ENUM e) const
		{
			return m_storage == static_cast<STORAGE>(e);
		}
		hkBool operator!=(ENUM e) const
		{
			return m_storage != static_cast<STORAGE>(e);
		}

	private:

		STORAGE m_storage;
};

template<typename TYPE>
class hkPadSpu
{
public:

	HK_ALWAYS_INLINE hkPadSpu()
	{
	}

	HK_FORCE_INLINE hkPadSpu(TYPE e)
	{
#if defined(HK_PLATFORM_PS3SPU) 
		m_storage = spu_promote( int(e), 0 );
#else
		m_storage = e;
#endif
	}

	HK_FORCE_INLINE void operator=(TYPE e)
	{
#if defined(HK_PLATFORM_PS3SPU) 
		m_storage = spu_promote( int(e), 0 );
#else
		m_storage = e;
#endif
	}

	HK_FORCE_INLINE TYPE val() const
	{
#if defined(HK_PLATFORM_PS3SPU) 
		return (TYPE)(spu_extract(m_storage,0));
#else
		return m_storage;
#endif
	}

	HK_FORCE_INLINE TYPE operator->() const
	{
#if defined(HK_PLATFORM_PS3SPU) 
		return (TYPE)(spu_extract(m_storage,0));
#else
		return m_storage;
#endif
	}

	HK_FORCE_INLINE operator TYPE() const
	{
		return val();
	}


private:
#if defined(HK_PLATFORM_PS3SPU) 
	HK_ALIGN16(vec_int4 m_storage);
#elif defined(HK_PLATFORM_PS3)
	HK_ALIGN16(TYPE m_storage);
	hkUchar m_padding[ 16 - sizeof(TYPE) ];
#else
	TYPE m_storage;
#endif
};


template<typename TYPE>
class hkPadSpuLong: public hkPadSpu<TYPE>
{
	public:
		HK_FORCE_INLINE hkPadSpuLong()	{}
		HK_FORCE_INLINE hkPadSpuLong(TYPE e): hkPadSpu<TYPE>(e) {}
};


template<typename TYPE>
class hkPadSpuf
{
	public:

		HK_FORCE_INLINE hkPadSpuf()
		{
		}

		HK_FORCE_INLINE hkPadSpuf(TYPE e)
		{
#if defined(HK_PLATFORM_PS3SPU) 
		m_storage = spu_promote( e, 0 );
#else
		m_storage = e;
#endif
		}

		HK_FORCE_INLINE void operator=(TYPE e)
		{
#if defined(HK_PLATFORM_PS3SPU) 
		m_storage = spu_promote( e, 0 );
#else
		m_storage = e;
#endif
		}

		HK_FORCE_INLINE TYPE val() const
		{
#if defined(HK_PLATFORM_PS3SPU) 
			return (TYPE)(spu_extract(m_storage,0));
#else
			return m_storage;
#endif
		}

		HK_FORCE_INLINE TYPE operator->() const
		{
#if defined(HK_PLATFORM_PS3SPU) 
			return (TYPE)(spu_extract(m_storage,0));
#else
			return m_storage;
#endif
		}

		HK_FORCE_INLINE operator TYPE() const
		{
			return val();
		}


	private:

#if defined(HK_PLATFORM_PS3SPU) 
		HK_ALIGN16(vec_float4 m_storage);
#elif defined(HK_PLATFORM_PS3)
		HK_ALIGN16(TYPE m_storage);
		hkUchar m_padding[ 16 - sizeof(TYPE) ];
#else
		TYPE m_storage;
#endif
};

#define HK_HINT_SIZE16(A) hkInt16(A)

#if defined(HK_PLATFORM_PS3) || defined(HK_PLATFORM_WIN32) || defined(HK_PLATFORM_XBOX360) 
#	define HK_PLATFORM_MULTI_THREAD
#endif



#endif // HKBASE_HKBASETYPES_H

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
