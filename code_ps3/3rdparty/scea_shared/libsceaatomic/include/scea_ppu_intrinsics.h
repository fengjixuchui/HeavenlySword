/*
 *  ppu_intrinsics.h
 *
 *  Created by Alex Rosenberg on 5/13/05.
 *  Copyright 2005 Sony Computer Entertainment America, Inc. All rights reserved.
 *
 *	This file is serious GCC voodoo. Even the GCC engineers themselves seem to barely
 *	understand this stuff. Requires a chicken, obsidian knife, and #gcc on oftc.net.
 *
 *  TODO:
 *		misc ops (traps)
 *		supervisor/hypervisor mode ops
 *		port to SNC when it becomes available
 *		port to MWERKS when it becomes available
 */

#if defined(__CELLOS_LV2__)
	#include <sdk_version.h>
#endif

/* 0.8.4 includes this file in the SDK directly, so refer to that version */
#if defined(__CELLOS_LV2__) && (CELL_SDK_VERSION >= 0x084000)
	#include <ppu_intrinsics.h>
#else

#ifndef _PPU_INTRINSICS_H
#define _PPU_INTRINSICS_H

#if !defined(__PPU__) && !defined(__ppc__) && !defined(__ppc64__)
    && !defined(__GNUC__)
  #error ppu_intrinsics.h included on wrong platform/compiler
#endif

#ifdef __cplusplus
extern "C" {
#endif 

/*
 * unsigned int __cntlzw(unsigned int)
 * unsigned int __cntlzd(unsigned long long)
 * unsigned int __popcntb(unsigned long long)
 * int __mulhw(int, int)
 * unsigned int __mulhwu(unsigned int, unsigned int)
 * long long __mulhd(long long, long long)
 * unsigned long long __mulhdu(unsigned long long, unsigned long long)
 *
 * void __sync(void)
 * void __isync(void)
 * void __lwsync(void)
 * void __eieio(void)
 *
 * void __nop(void)
 * void __cctpl(void)
 * void __cctpm(void)
 * void __cctph(void)
 * void __db8cyc(void)
 * void __db10cyc(void)
 * void __db12cyc(void)
 * void __db16cyc(void)
 *
 * void __mtspr(unsigned int spr, unsigned long long value)
 * unsigned long long __mfspr(unsigned int spr)
 * unsigned long long __mftb(void)
 *
 * void __icbi(void *base)
 * void __dcbi(void *base)
 *
 * void __dcbf(void *base)
 * void __dcbz(void *base)
 * void __dcbst(void *base)
 * void __dcbtst(void *base)
 * void __dcbt(void *base)
 * void __dcbt_TH1000(void *EATRUNC, bool D, bool UG, int ID)
 * void __dcbt_TH1010(bool GO, int S, int UNITCNT, bool T, bool U, int ID)
 *
 * unsigned __lwarx(void *base)
 * unsigned long long __ldarx(void *base)
 * bool __stwcx(void *base, unsigned value)
 * bool __stdcx(void *base, unsigned long long value)
 *
 * unsigned short __lhbrx(unsigned short *base)
 * signed short __lhbrx(signed short *base)
 * unsigned int __lwbrx(unsigned int *base)
 * unsigned long long __ldbrx(unsigned long long *base)
 * signed int __lwbrx(signed int *base)
 * void __sthbrx(void *base, unsigned short value)
 * void __stwbrx(void *base, unsigned int value)
 * void __stdbrx(void *base, unsigned long long value)
 *
 * double __fabs(double x)
 * float __fabsf(double x)
 * double __fnabs(double x)
 * float __fnabsf(double x)
 * double __fmadd(double x, double y, double z)
 * double __fmsub(double x, double y, double z)
 * double __fnmadd(double x, double y, double z)
 * double __fnmsub(double x, double y, double z)
 * float __fmadds(double x, double y, double z)
 * float __fmsubs(double x, double y, double z)
 * float __fnmadds(double x, double y, double z)
 * float __fnmsubs(double x, double y, double z)
 * double __fsel(double x, double y, double z)
 * float __fsels(double x, double y, double z)
 * double __frsqrte(double x)
 * float __fres(double x)
 * double __fre(double x)
 * double __fsqrt(double x)
 * float __fsqrts(double x)
 * long long __fctid(double x)
 * long long __fctiw(double x)
 * double __fcfid(long long x)
 * double __mffs(void)
 * void __mtfsf(int mask, double value)
 * void __mtfsfi(int bits, int field)
 * void __mtfsb0(int)
 * void __mtfsb1(int)
 * double __setflm(double)
 */

typedef int __V4SI __attribute__((vector_size(16)));

#define __cntlzw(v) __builtin_clz(v)
#define __cntlzd(v) __builtin_clzll(v)
#if 0
/* commented out due to lack of assembler support */
#define __popcntb(v) __extension__ \
  ({unsigned long long result;	   \
  __asm__ ("popcntb %0,%1"	   \
	   : "=r" (result)	   \
	   : "r" (v));		   \
  result; })
#endif

#define __mulhw(a,b) __extension__ \
  ({int result;			   \
  __asm__ ("mulhw %0,%1,%2"	   \
	   : "=r" (result)	   \
	   : "r" ((int) (a)),	   \
	     "r" ((int) (b)));	   \
  result; })

#define __mulhwu(a,b) __extension__	\
  ({unsigned int result;		\
  __asm__ ("mulhwu %0,%1,%2"		\
	   : "=r" (result)		\
	   : "r" ((unsigned int) (a)),	\
	     "r" ((unsigned int) (b))); \
  result; })

#define __mulhd(a,b) __extension__   \
  ({ long long result;		     \
  __asm__ ("mulhd %0,%1,%2"	     \
	   : "=r" (result)	     \
	   : "r" ((long long) (a)),  \
	     "r" ((long long) (b))); \
  result; })

#define __mulhdu(a,b) __extension__	      \
  ({unsigned long long result;		      \
  __asm__ ("mulhdu %0,%1,%2"		      \
	   : "=r" (result)		      \
	   : "r" ((unsigned long long) (a)),  \
	     "r" ((unsigned long long) (b))); \
  result; })

#define __sync() __asm__ volatile ("sync" : : : "memory")
#define __isync() __asm__ volatile ("isync" : : : "memory")
#define __lwsync() __asm__ volatile ("lwsync" : : : "memory")
#define __eieio() __asm__ volatile ("eieio" : : : "memory")

#define __nop() __asm__ volatile ("ori 0,0,0" : : : "memory")
#define __cctpl() __asm__ volatile ("or 1,1,1" : : : "memory")
#define __cctpm() __asm__ volatile ("or 2,2,2" : : : "memory")
#define __cctph() __asm__ volatile ("or 3,3,3" : : : "memory")
#define __db8cyc() __asm__ volatile ("or 28,28,28" : : : "memory")
#define __db10cyc() __asm__ volatile ("or 29,29,29" : : : "memory")
#define __db12cyc() __asm__ volatile ("or 30,30,30" : : : "memory")
#define __db16cyc() __asm__ volatile ("or 31,31,31" : : : "memory")

#define __mtspr(spr, value) \
  __asm__ volatile ("mtspr %0,%1" : : "n" (spr), "r" (value))
  
#define __mfspr(spr) __extension__				\
  ({ unsigned long long result;					\
  __asm__ volatile ("mfspr %0,%1" : "=r" (result) : "n" (spr)); \
  result; })
  
#define __mftb() __extension__			\
  ({ unsigned long long result;			\
  __asm__ volatile ("mftb %0" : "=r" (result));	\
  result; })

#define __dcbf(base) \
  __asm__ volatile ("dcbf %y0" : "=m" (*(__V4SI*) (base)) : : "memory")
  
#define __dcbz(base) \
  __asm__ volatile ("dcbz %y0" : "=m" (*(__V4SI*) (base)) : : "memory")

#define __dcbst(base) \
  __asm__ volatile ("dcbst %y0" : "=m" (*(__V4SI*) (base)) : : "memory")

#define __dcbtst(base) \
  __asm__ volatile ("dcbtst %y0" : "=m" (*(__V4SI*) (base)) : : "memory")

#define __dcbt(base) \
  __asm__ volatile ("dcbt %y0" : "=m" (*(__V4SI*) (base)) : : "memory")

#define __icbi(base) \
  __asm__ volatile ("icbi %y0" : "=m" (*(__V4SI*) (base)) : : "memory")
  
#define __dcbi(base) \
  __asm__ volatile ("dcbi %y0" : "=m" (*(__V4SI*) (base)) : : "memory")

#define __dcbt_TH1000(EATRUNC, D, UG, ID)				\
  __asm__ volatile ("dcbt %y0,8"					\
	   : "=m" (*(__V4SI*) ((((unsigned long long) EATRUNC) & ~0x7F)	\
	   		       | (((D & 1) << 6)			\
	   		       | ((UG & 1) << 5)			\
	   		       | (ID & 0xF)))) : : "memory")

#define __dcbt_TH1010(GO, S, UNITCNT, T, U, ID)			     \
  __asm__ volatile ("dcbt %y0,10"				     \
	   : "=m" (*(__V4SI*) ((((unsigned long long) GO & 1) << 31) \
	   		       | ((S & 0x3) << 29)		     \
	   		       | ((UNITCNT & 0x3FF) << 7)	     \
	   		       | ((T & 1) << 6)			     \
	   		       | ((U & 1) << 5)			     \
	   		       | (ID & 0xF))) : : "memory")

#define __lhbrx(base) __extension__		\
  ({unsigned short result;	       		\
    typedef  struct {char a[2];} halfwordsize;	\
    halfwordsize *ptrp = (halfwordsize*)(base);	\
  __asm__ ("lhbrx %0,%y1"			\
	   : "=r" (result)			\
	   : "m" (*ptrp));			\
  result; })

#define __lwbrx(base) __extension__		\
  ({unsigned int result;	       		\
    typedef  struct {char a[4];} wordsize;	\
    wordsize *ptrp = (wordsize*)(base);		\
  __asm__ ("lwbrx %0,%y1"			\
	   : "=r" (result)			\
	   : "m" (*ptrp));			\
  result; })


#define __ldbrx(base) __extension__			\
  ({unsigned long long result;	       			\
    typedef  struct {char a[8];} doublewordsize;	\
    doublewordsize *ptrp = (doublewordsize*)(base);	\
  __asm__ ("ldbrx %0,%y1"				\
	   : "=r" (result)				\
	   : "m" (*ptrp));				\
  result; })


#define __sthbrx(base, value) do {			\
    typedef  struct {char a[2];} halfwordsize;		\
    halfwordsize *ptrp = (halfwordsize*)(base);		\
    __asm__ ("sthbrx %1,%y0"				\
	   : "=m" (*ptrp)				\
	   : "r" (value));				\
   } while (0)

#define __stwbrx(base, value) do {		\
    typedef  struct {char a[4];} wordsize;	\
    wordsize *ptrp = (wordsize*)(base);		\
    __asm__ ("stwbrx %1,%y0"			\
	   : "=m" (*ptrp)			\
	   : "r" (value));			\
   } while (0)


#define __stdbrx(base, value) do {			\
    typedef  struct {char a[8];} doublewordsize;	\
    doublewordsize *ptrp = (doublewordsize*)(base);	\
    __asm__ ("stdbrx %1,%y0"				\
	   : "=m" (*ptrp)				\
	   : "r" (value));				\
   } while (0)



#define __lwarx(base) __extension__		\
  ({unsigned int result;	       		\
    typedef  struct {char a[4];} wordsize;	\
    wordsize *ptrp = (wordsize*)(base);		\
  __asm__ ("lwarx %0,%y1"			\
	   : "=r" (result)			\
	   : "m" (*ptrp));			\
  result; })


#define __ldarx(base) __extension__			\
  ({unsigned long long result;	       			\
    typedef  struct {char a[8];} doublewordsize;	\
    doublewordsize *ptrp = (doublewordsize*)(base);	\
  __asm__ ("ldarx %0,%y1"				\
	   : "=r" (result)				\
	   : "m" (*ptrp));				\
  result; })


#define __stwcx(base, value) __extension__	\
  ({unsigned int result;			\
    typedef  struct {char a[4];} wordsize;	\
    wordsize *ptrp = (wordsize*)(base);		\
  __asm__ ("stwcx. %2,%y1\n"			\
	   "\tmfocrf %0,0x80"			\
	   : "=r" (result),			\
	     "=m" (*ptrp)			\
	   : "r" (value) : "cr0");		\
  (result & 0x20000000); })


#define __stdcx(base, value) __extension__		\
  ({unsigned long long result;				\
    typedef  struct {char a[8];} doublewordsize;	\
    doublewordsize *ptrp = (doublewordsize*)(base);	\
  __asm__ ("stdcx. %2,%y1\n"				\
	   "\tmfocrf %0,0x80"				\
	   : "=r" (result),				\
	     "=m" (*ptrp)				\
	   : "r" (value) : "cr0");			\
  (result & 0x20000000); })


#define __mffs() __extension__			\
  ({double result;				\
  __asm__ volatile ("mffs %0" : "=f" (result)); \
  result; })

#define __mtfsf(mask,value) \
  __asm__ volatile ("mtfsf %0,%1" : : "n" (mask), "f" ((double) (value)))
  
#define __mtfsfi(bits,field) \
  __asm__ volatile ("mtfsfi %0,%1" : : "n" (bits), "n" (field))

#define __mtfsb0(bit) __asm__ volatile ("mtfsb0 %0" : : "n" (bit))
#define __mtfsb1(bit) __asm__ volatile ("mtfsb1 %0" : : "n" (bit))

#define __setflm(v) __extension__	      \
  ({double result;			      \
  __asm__ volatile ("mffs %0\n\tmtfsf 255,%1" \
		    : "=f" (result)	      \
		    : "f" ((double) (v)));    \
  result; })

// __builtin_fabs may perform unnecessary rounding

static __inline__ double __fabs(double x) __attribute__((always_inline));
static __inline__ double
__fabs(double x)
{
  double r;
  __asm__("fabs %0,%1" : "=f"(r) : "f"(x));
  return r;
}

static __inline__ float __fabsf(double x) __attribute__((always_inline));
static __inline__ float
__fabsf(double x)
{
  float r;
  __asm__("fabs %0,%1" : "=f"(r) : "f"(x));
  return r;
}

static __inline__ double __fnabs(double x) __attribute__((always_inline));
static __inline__ double
__fnabs(double x)
{
  double r;
  __asm__("fnabs %0,%1" : "=f"(r) : "f"(x));
  return r;
}

static __inline__ float __fnabsf(double x) __attribute__((always_inline));
static __inline__ float
__fnabsf(double x)
{
  float r;
  __asm__("fnabs %0,%1" : "=f"(r) : "f"(x));
  return r;
}

static __inline__ double __fmadd(double x, double y, double z)
  __attribute__((always_inline));
static __inline__ double
__fmadd(double x, double y, double z)
{
  double r;
  __asm__("fmadd %0,%1,%2,%3" : "=f"(r) : "f"(x),"f"(y),"f"(z));
  return r;
}

static __inline__ double __fmsub(double x, double y, double z)
  __attribute__((always_inline));
static __inline__ double
__fmsub(double x, double y, double z)
{
  double r;
  __asm__("fmsub %0,%1,%2,%3" : "=f"(r) : "f"(x),"f"(y),"f"(z));
  return r;
}

static __inline__ double __fnmadd(double x, double y, double z)
  __attribute__((always_inline));
static __inline__ double
__fnmadd(double x, double y, double z)
{
  double r;
  __asm__("fnmadd %0,%1,%2,%3" : "=f"(r) : "f"(x),"f"(y),"f"(z));
  return r;
}

static __inline__ double __fnmsub(double x, double y, double z)
  __attribute__((always_inline));
static __inline__ double
__fnmsub(double x, double y, double z)
{
  double r;
  __asm__("fnmsub %0,%1,%2,%3" : "=f"(r) : "f"(x),"f"(y),"f"(z));
  return r;
}

static __inline__ float __fmadds(double x, double y, double z)
  __attribute__((always_inline));
static __inline__ float
__fmadds(double x, double y, double z)
{
  float r;
  __asm__("fmadds %0,%1,%2,%3" : "=f"(r) : "f"(x),"f"(y),"f"(z));
  return r;
}

static __inline__ float __fmsubs(double x, double y, double z)
  __attribute__((always_inline));
static __inline__ float
__fmsubs(double x, double y, double z)
{
  float r;
  __asm__("fmsubs %0,%1,%2,%3" : "=f"(r) : "f"(x),"f"(y),"f"(z));
  return r;
}

static __inline__ float __fnmadds(double x, double y, double z)
  __attribute__((always_inline));
static __inline__ float
__fnmadds(double x, double y, double z)
{
  float r;
  __asm__("fnmadds %0,%1,%2,%3" : "=f"(r) : "f"(x),"f"(y),"f"(z));
  return r;
}

static __inline__ float __fnmsubs(double x, double y, double z)
  __attribute__((always_inline));
static __inline__ float
__fnmsubs(double x, double y, double z)
{
  float r;
  __asm__("fnmsubs %0,%1,%2,%3" : "=f"(r) : "f"(x),"f"(y),"f"(z));
  return r;
}

static __inline__ double __fsel(double x, double y, double z)
  __attribute__((always_inline));
static __inline__ double
__fsel(double x, double y, double z)
{
  double r;
  __asm__("fsel %0,%1,%2,%3" : "=f"(r) : "f"(x),"f"(y),"f"(z));
  return r;
}

static __inline__ float __fsels(double x, double y, double z)
  __attribute__((always_inline));
static __inline__ float
__fsels(double x, double y, double z)
{
  float r;
  __asm__("fsel %0,%1,%2,%3" : "=f"(r) : "f"(x),"f"(y),"f"(z));
  return r;
}

static __inline__ double __frsqrte(double x) __attribute__((always_inline));
static __inline__ double
__frsqrte(double x)
{
  double r;
  __asm__("frsqrte %0,%1" : "=f"(r) : "f"(x));
  return r;
}

static __inline__ float __fres(double x) __attribute__((always_inline));
static __inline__ float
__fres(double x)
{
  float r;
  __asm__("fres %0,%1" : "=f"(r) : "f"(x));
  return r;
}

static __inline__ double __fre(double x) __attribute__((always_inline));
static __inline__ double
__fre(double x)
{
  double r;
  __asm__("fre %0,%1" : "=f"(r) : "f"(x));
  return r;
}

static __inline__ double __fsqrt(double x) __attribute__((always_inline));
static __inline__ double
__fsqrt(double x)
{
  double r;
  __asm__("fsqrt %0,%1" : "=f"(r) : "f"(x));
  return r;
}

static __inline__ float __fsqrts(double x) __attribute__((always_inline));
static __inline__ float
__fsqrts(double x)
{
  float r;
  __asm__("fsqrts %0,%1" : "=f"(r) : "f"(x));
  return r;
}

static __inline__ long long __fctid(double x) __attribute__((always_inline));
static __inline__ long long
__fctid(double x)
{
  long long r;
  __asm__("fctid %0, %1" : "=f"(r) : "f"(x) );
  return r;
}

static __inline__ long long __fctiw(double x) __attribute__((always_inline));
static __inline__ long long
__fctiw(double x)
{
  long long r;
  __asm__("fctiw %0, %1" : "=f"(r) : "f"(x) );
  return r;
}

static __inline__ double __fcfid (long long x) __attribute__((always_inline));
static __inline__ double
__fcfid (long long x)
{
  double r;
  __asm__("fcfid %0, %1" : "=f"(r) : "f"(x) );
  return r;
}

#ifdef __cplusplus
}
#endif

#endif /* _PPU_INTRINSICS_H */

#endif
