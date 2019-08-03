/**
    \file fios_common.h

    File I/O runtime library common definitions.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_common
#define _H_fios_common


#if !defined(FIOS_STATIC_LIB) && !defined(FIOS_DLL) && !defined(FIOS_MIXIN)
# define FIOS_STATIC_LIB     1
#endif


/**
	\internal
	\def FIOS_EXPORT
	\brief Generates compiler export/import directives like __declspec(dllimport) on some platforms.
*/
#if SCEA_HOST_COMPILER_MSVC && SCEA_TARGET_OS_WIN32
# if FIOS_DLL          /* ----- DLL ----- */
#  if FIOS_EXPORTS     /* library */
#   define FIOS_EXPORT __declspec(dllexport)
#  elif !FIOS_EXPORTS  /* client */
#   define FIOS_EXPORT __declspec(dllimport)
#   pragma comment(lib, "fios.lib")
#   pragma comment(lib, "pthreadVC2.lib")
#  endif
# elif FIOS_STATIC_LIB /* ----- static lib ----- */
#  define FIOS_EXPORT
#  if !FIOS_EXPORTS    /* client */
#   pragma comment(lib, "fios_static.lib")
#   pragma comment(lib, "pthreadVC2.lib")
#  endif
# elif FIOS_MIXIN      /* ----- code added directly to your project ----- */
#  define FIOS_EXPORT
# else
#  error "One of these must be defined to 1: FIOS_DLL, FIOS_STATIC_LIB, or FIOS_MIXIN"
# endif
#else /* !SCEA_HOST_COMPILER_MSVC */
# define FIOS_EXPORT
#endif /* SCEA_HOST_COMPILER_MSVC */

/**
	\internal
	\def FIOS_ATTRIBUTE
	\param[in]  attr   Attribute definition.
	\brief Declares attributes that are compatible with both GCC and SNC.
*/
#if SCEA_HOST_COMPILER_GCC
# define FIOS_ATTRIBUTE(attr)       __attribute__(attr)
#else
# define FIOS_ATTRIBUTE(attr)
#endif

/**
	\internal
	\def FIOS_ATTRIBUTE_GCC
	\brief Declares attributes that are compatible with GCC but not SNC.
*/
#if SCEA_HOST_COMPILER_GCC && !SCEA_HOST_COMPILER_GCC_SN
# define FIOS_ATTRIBUTE_GCC(attr)       __attribute__(attr)
#else
# define FIOS_ATTRIBUTE_GCC(attr)
#endif

/**
	\internal
	\def FIOS_EXPECT
	\brief Tells the compiler that expr will almost always evaluate to val, and to optimize for that.
	\param[in]  expr   Expression
	\param[in]  val    Value
	\return Evaluates to the value of expr.
*/
#if SCEA_HOST_COMPILER_GCC && !SCEA_HOST_COMPILER_GCC_SN
# define FIOS_EXPECT(expr, val)          __builtin_expect((expr), (val))
#else
# define FIOS_EXPECT(expr, val)          (expr)
#endif

/** \internal
	\def FIOS_LIKELY
	\brief Tells the compiler that the boolean expr will typically evaluate to 1, and to optimize for that.
	\param[in]  expr
	\return Evaluates to the value of expr.
*/
#define FIOS_LIKELY(expr)            FIOS_EXPECT(expr, 1)

/** \internal
	\def FIOS_UNLIKELY
	\brief Tells the compiler that the boolean expr will typically evaluate to 0, and to optimize for that.
	\param[in]  expr
	\return Evaluates to the value of expr.
*/
#define FIOS_UNLIKELY(expr)           FIOS_EXPECT(expr, 0)

/** \internal
	\brief Declares a parameter to be unused without generating a warning. */
#define FIOS_UNUSED(param)      (void)&param


// Enable WinXP defs on Win32.
#if SCEA_HOST_COMPILER_MSVC
# if _MSC_VER > VC60
#  ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x0501
#  endif
#  ifndef _WIN32_WINDOWS
#   define _WIN32_WINDOWS 0x0410
#  endif
# endif
#endif 


#endif // _H_fios_common
