# 1 "ntlib_spu/ntDMA.cpp"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "ntlib_spu/ntDMA.cpp"
# 14 "ntlib_spu/ntDMA.cpp"
# 1 "ntlib_spu/ntDMA.h" 1
# 24 "ntlib_spu/ntDMA.h"
# 1 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_mfcio.h" 1 3 4
# 32 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_mfcio.h" 3 4
# 1 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_intrinsics.h" 1 3 4
# 79 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_intrinsics.h" 3 4
# 1 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_internals.h" 1 3 4
# 63 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_internals.h" 3 4
typedef int qword __attribute__((__mode__(V16QI)));
# 367 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_internals.h" 3 4
static inline vector float __hack_spu_convtf (vector signed int, vector float, vector float) __attribute__((__always_inline__));
static inline vector float __hack_spu_convtf (vector unsigned int, vector float, vector float) __attribute__((__always_inline__));
static inline vector float
__hack_spu_convtf (vector signed int ra, vector float from_signed, vector float from_unsigned)
{
  (void)ra;
  (void)from_unsigned;
  return from_signed;
}
static inline vector float
__hack_spu_convtf (vector unsigned int ra, vector float from_signed, vector float from_unsigned)
{
  (void)ra;
  (void)from_signed;
  return from_unsigned;
}
# 407 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_internals.h" 3 4
static inline vector signed short spu_extend (vector signed char a) __attribute__((__always_inline__));
static inline vector signed int spu_extend (vector signed short a) __attribute__((__always_inline__));
static inline vector signed long long spu_extend (vector signed int a) __attribute__((__always_inline__));
static inline vector double spu_extend (vector float a) __attribute__((__always_inline__));
static inline vector unsigned int spu_add (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_add (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_add (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_add (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector float spu_add (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_add (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector unsigned short spu_add (vector unsigned short a, unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_add (vector signed short a, short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_add (vector unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_add (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_addx (vector signed int a, vector signed int b, vector signed int c) __attribute__((__always_inline__));
static inline vector unsigned int spu_addx (vector unsigned int a, vector unsigned int b, vector unsigned int c) __attribute__((__always_inline__));
static inline vector signed int spu_genc (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_genc (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_gencx (vector signed int a, vector signed int b, vector signed int c) __attribute__((__always_inline__));
static inline vector unsigned int spu_gencx (vector unsigned int a, vector unsigned int b, vector unsigned int c) __attribute__((__always_inline__));
static inline vector signed int spu_madd (vector signed short a, vector signed short b, vector signed int c) __attribute__((__always_inline__));
static inline vector float spu_madd (vector float a, vector float b, vector float c) __attribute__((__always_inline__));
static inline vector double spu_madd (vector double a, vector double b, vector double c) __attribute__((__always_inline__));
static inline vector float spu_msub (vector float a, vector float b, vector float c) __attribute__((__always_inline__));
static inline vector double spu_msub (vector double a, vector double b, vector double c) __attribute__((__always_inline__));
static inline vector unsigned int spu_mhhadd (vector unsigned short a, vector unsigned short b, vector unsigned int c) __attribute__((__always_inline__));
static inline vector signed int spu_mhhadd (vector signed short a, vector signed short b, vector signed int c) __attribute__((__always_inline__));
static inline vector unsigned int spu_mule (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed int spu_mule (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector float spu_mul (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_mul (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector signed int spu_mulo (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_mulo (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed int spu_mulo (vector signed short a, short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_mulo (vector unsigned short a, unsigned short b) __attribute__((__always_inline__));
static inline vector float spu_nmsub (vector float a, vector float b, vector float c) __attribute__((__always_inline__));
static inline vector double spu_nmsub (vector double a, vector double b, vector double c) __attribute__((__always_inline__));
static inline vector unsigned short spu_sub (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_sub (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_sub (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_sub (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector float spu_sub (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_sub (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector unsigned short spu_sub (unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_sub (short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_sub (unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_sub (int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_subx (vector unsigned int a, vector unsigned int b, vector unsigned int c) __attribute__((__always_inline__));
static inline vector signed int spu_subx (vector signed int a, vector signed int b, vector signed int c) __attribute__((__always_inline__));
static inline vector unsigned int spu_genb (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_genb (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_genbx (vector unsigned int a, vector unsigned int b, vector unsigned int c) __attribute__((__always_inline__));
static inline vector signed int spu_genbx (vector signed int a, vector signed int b, vector signed int c) __attribute__((__always_inline__));
static inline vector unsigned char spu_cmpeq (vector unsigned char a, vector unsigned char b) __attribute__((__always_inline__));
static inline vector unsigned char spu_cmpeq (vector signed char a, vector signed char b) __attribute__((__always_inline__));
static inline vector unsigned short spu_cmpeq (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector unsigned short spu_cmpeq (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_cmpeq (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_cmpeq (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_cmpeq (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector unsigned char spu_cmpeq (vector unsigned char a, unsigned char b) __attribute__((__always_inline__));
static inline vector unsigned char spu_cmpeq (vector signed char a, signed char b) __attribute__((__always_inline__));
static inline vector unsigned short spu_cmpeq (vector unsigned short a, unsigned short b) __attribute__((__always_inline__));
static inline vector unsigned short spu_cmpeq (vector signed short a, short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_cmpeq (vector unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_cmpeq (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_cmpgt (vector unsigned char a, vector unsigned char b) __attribute__((__always_inline__));
static inline vector unsigned char spu_cmpgt (vector signed char a, vector signed char b) __attribute__((__always_inline__));
static inline vector unsigned short spu_cmpgt (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector unsigned short spu_cmpgt (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_cmpgt (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_cmpgt (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_cmpgt (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector unsigned char spu_cmpgt (vector unsigned char a, unsigned char b) __attribute__((__always_inline__));
static inline vector unsigned char spu_cmpgt (vector signed char a, signed char b) __attribute__((__always_inline__));
static inline vector unsigned short spu_cmpgt (vector unsigned short a, unsigned short b) __attribute__((__always_inline__));
static inline vector unsigned short spu_cmpgt (vector signed short a, short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_cmpgt (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_cmpgt (vector unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline void spu_hcmpeq (int a, int b) __attribute__((__always_inline__));
static inline void spu_hcmpeq (unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline void spu_hcmpgt (int a, int b) __attribute__((__always_inline__));
static inline void spu_hcmpgt (unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_cntb (vector signed char a) __attribute__((__always_inline__));
static inline vector unsigned char spu_cntb (vector unsigned char a) __attribute__((__always_inline__));
static inline vector unsigned int spu_cntlz (vector signed int a) __attribute__((__always_inline__));
static inline vector unsigned int spu_cntlz (vector unsigned int a) __attribute__((__always_inline__));
static inline vector unsigned int spu_cntlz (vector float a) __attribute__((__always_inline__));
static inline vector unsigned int spu_gather (vector signed int a) __attribute__((__always_inline__));
static inline vector unsigned int spu_gather (vector unsigned int a) __attribute__((__always_inline__));
static inline vector unsigned int spu_gather (vector signed short a) __attribute__((__always_inline__));
static inline vector unsigned int spu_gather (vector unsigned short a) __attribute__((__always_inline__));
static inline vector unsigned int spu_gather (vector signed char a) __attribute__((__always_inline__));
static inline vector unsigned int spu_gather (vector unsigned char a) __attribute__((__always_inline__));
static inline vector unsigned int spu_gather (vector float a) __attribute__((__always_inline__));
static inline vector unsigned char spu_maskb (unsigned short a) __attribute__((__always_inline__));
static inline vector unsigned char spu_maskb (short a) __attribute__((__always_inline__));
static inline vector unsigned char spu_maskb (unsigned int a) __attribute__((__always_inline__));
static inline vector unsigned char spu_maskb (int a) __attribute__((__always_inline__));
static inline vector unsigned short spu_maskh (unsigned char a) __attribute__((__always_inline__));
static inline vector unsigned short spu_maskh (signed char a) __attribute__((__always_inline__));
static inline vector unsigned short spu_maskh (char a) __attribute__((__always_inline__));
static inline vector unsigned short spu_maskh (unsigned short a) __attribute__((__always_inline__));
static inline vector unsigned short spu_maskh (short a) __attribute__((__always_inline__));
static inline vector unsigned short spu_maskh (unsigned int a) __attribute__((__always_inline__));
static inline vector unsigned short spu_maskh (int a) __attribute__((__always_inline__));
static inline vector unsigned int spu_maskw (unsigned char a) __attribute__((__always_inline__));
static inline vector unsigned int spu_maskw (signed char a) __attribute__((__always_inline__));
static inline vector unsigned int spu_maskw (char a) __attribute__((__always_inline__));
static inline vector unsigned int spu_maskw (unsigned short a) __attribute__((__always_inline__));
static inline vector unsigned int spu_maskw (short a) __attribute__((__always_inline__));
static inline vector unsigned int spu_maskw (unsigned int a) __attribute__((__always_inline__));
static inline vector unsigned int spu_maskw (int a) __attribute__((__always_inline__));
static inline vector signed long long spu_sel (vector signed long long a, vector signed long long b, vector unsigned long long c) __attribute__((__always_inline__));
static inline vector unsigned long long spu_sel (vector unsigned long long a, vector unsigned long long b, vector unsigned long long c) __attribute__((__always_inline__));
static inline vector signed int spu_sel (vector signed int a, vector signed int b, vector unsigned int c) __attribute__((__always_inline__));
static inline vector unsigned int spu_sel (vector unsigned int a, vector unsigned int b, vector unsigned int c) __attribute__((__always_inline__));
static inline vector signed short spu_sel (vector signed short a, vector signed short b, vector unsigned short c) __attribute__((__always_inline__));
static inline vector unsigned short spu_sel (vector unsigned short a, vector unsigned short b, vector unsigned short c) __attribute__((__always_inline__));
static inline vector signed char spu_sel (vector signed char a, vector signed char b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector unsigned char spu_sel (vector unsigned char a, vector unsigned char b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector float spu_sel (vector float a, vector float b, vector unsigned int c) __attribute__((__always_inline__));
static inline vector double spu_sel (vector double a, vector double b, vector unsigned long long c) __attribute__((__always_inline__));
static inline vector signed long long spu_sel (vector signed long long a, vector signed long long b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector unsigned long long spu_sel (vector unsigned long long a, vector unsigned long long b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector signed int spu_sel (vector signed int a, vector signed int b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector unsigned int spu_sel (vector unsigned int a, vector unsigned int b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector signed short spu_sel (vector signed short a, vector signed short b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector unsigned short spu_sel (vector unsigned short a, vector unsigned short b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector float spu_sel (vector float a, vector float b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector double spu_sel (vector double a, vector double b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector unsigned char spu_shuffle (vector unsigned char a, vector unsigned char b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector signed char spu_shuffle (vector signed char a, vector signed char b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector unsigned short spu_shuffle (vector unsigned short a, vector unsigned short b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector signed short spu_shuffle (vector signed short a, vector signed short b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector unsigned int spu_shuffle (vector unsigned int a, vector unsigned int b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector signed int spu_shuffle (vector signed int a, vector signed int b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector unsigned long long spu_shuffle (vector unsigned long long a, vector unsigned long long b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector signed long long spu_shuffle (vector signed long long a, vector signed long long b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector float spu_shuffle (vector float a, vector float b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector double spu_shuffle (vector double a, vector double b, vector unsigned char c) __attribute__((__always_inline__));
static inline vector unsigned char spu_and (vector unsigned char a, vector unsigned char b) __attribute__((__always_inline__));
static inline vector signed char spu_and (vector signed char a, vector signed char b) __attribute__((__always_inline__));
static inline vector unsigned short spu_and (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_and (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_and (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_and (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_and (vector unsigned long long a, vector unsigned long long b) __attribute__((__always_inline__));
static inline vector signed long long spu_and (vector signed long long a, vector signed long long b) __attribute__((__always_inline__));
static inline vector float spu_and (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_and (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector unsigned char spu_and (vector unsigned char a, unsigned char b) __attribute__((__always_inline__));
static inline vector signed char spu_and (vector signed char a, signed char b) __attribute__((__always_inline__));
static inline vector unsigned short spu_and (vector unsigned short a, unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_and (vector signed short a, short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_and (vector unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_and (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector signed long long spu_andc (vector signed long long a, vector signed long long b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_andc (vector unsigned long long a, vector unsigned long long b) __attribute__((__always_inline__));
static inline vector signed int spu_andc (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_andc (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed short spu_andc (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned short spu_andc (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed char spu_andc (vector signed char a, vector signed char b) __attribute__((__always_inline__));
static inline vector unsigned char spu_andc (vector unsigned char a, vector unsigned char b) __attribute__((__always_inline__));
static inline vector float spu_andc (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_andc (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector signed long long spu_eqv (vector signed long long a, vector signed long long b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_eqv (vector unsigned long long a, vector unsigned long long b) __attribute__((__always_inline__));
static inline vector signed int spu_eqv (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_eqv (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed short spu_eqv (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned short spu_eqv (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed char spu_eqv (vector signed char a, vector signed char b) __attribute__((__always_inline__));
static inline vector unsigned char spu_eqv (vector unsigned char a, vector unsigned char b) __attribute__((__always_inline__));
static inline vector float spu_eqv (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_eqv (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector signed long long spu_nand (vector signed long long a, vector signed long long b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_nand (vector unsigned long long a, vector unsigned long long b) __attribute__((__always_inline__));
static inline vector signed int spu_nand (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_nand (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed short spu_nand (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned short spu_nand (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed char spu_nand (vector signed char a, vector signed char b) __attribute__((__always_inline__));
static inline vector unsigned char spu_nand (vector unsigned char a, vector unsigned char b) __attribute__((__always_inline__));
static inline vector float spu_nand (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_nand (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector signed long long spu_nor (vector signed long long a, vector signed long long b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_nor (vector unsigned long long a, vector unsigned long long b) __attribute__((__always_inline__));
static inline vector signed int spu_nor (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_nor (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed short spu_nor (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned short spu_nor (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed char spu_nor (vector signed char a, vector signed char b) __attribute__((__always_inline__));
static inline vector unsigned char spu_nor (vector unsigned char a, vector unsigned char b) __attribute__((__always_inline__));
static inline vector float spu_nor (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_nor (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector unsigned char spu_or (vector unsigned char a, vector unsigned char b) __attribute__((__always_inline__));
static inline vector signed char spu_or (vector signed char a, vector signed char b) __attribute__((__always_inline__));
static inline vector unsigned short spu_or (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_or (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_or (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_or (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_or (vector unsigned long long a, vector unsigned long long b) __attribute__((__always_inline__));
static inline vector signed long long spu_or (vector signed long long a, vector signed long long b) __attribute__((__always_inline__));
static inline vector float spu_or (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_or (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector unsigned char spu_or (vector unsigned char a, unsigned char b) __attribute__((__always_inline__));
static inline vector signed char spu_or (vector signed char a, signed char b) __attribute__((__always_inline__));
static inline vector unsigned short spu_or (vector unsigned short a, unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_or (vector signed short a, short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_or (vector unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_or (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector signed long long spu_orc (vector signed long long a, vector signed long long b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_orc (vector unsigned long long a, vector unsigned long long b) __attribute__((__always_inline__));
static inline vector signed int spu_orc (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_orc (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed short spu_orc (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned short spu_orc (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed char spu_orc (vector signed char a, vector signed char b) __attribute__((__always_inline__));
static inline vector unsigned char spu_orc (vector unsigned char a, vector unsigned char b) __attribute__((__always_inline__));
static inline vector float spu_orc (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_orc (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector signed int spu_orx (vector signed int a) __attribute__((__always_inline__));
static inline vector unsigned int spu_orx (vector unsigned int a) __attribute__((__always_inline__));
static inline vector unsigned char spu_xor (vector unsigned char a, vector unsigned char b) __attribute__((__always_inline__));
static inline vector signed char spu_xor (vector signed char a, vector signed char b) __attribute__((__always_inline__));
static inline vector unsigned short spu_xor (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_xor (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_xor (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_xor (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_xor (vector unsigned long long a, vector unsigned long long b) __attribute__((__always_inline__));
static inline vector signed long long spu_xor (vector signed long long a, vector signed long long b) __attribute__((__always_inline__));
static inline vector float spu_xor (vector float a, vector float b) __attribute__((__always_inline__));
static inline vector double spu_xor (vector double a, vector double b) __attribute__((__always_inline__));
static inline vector unsigned char spu_xor (vector unsigned char a, unsigned char b) __attribute__((__always_inline__));
static inline vector signed char spu_xor (vector signed char a, signed char b) __attribute__((__always_inline__));
static inline vector unsigned short spu_xor (vector unsigned short a, unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_xor (vector signed short a, short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_xor (vector unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_xor (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rl (vector unsigned short a, vector signed short b) __attribute__((__always_inline__));
static inline vector signed short spu_rl (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rl (vector unsigned int a, vector signed int b) __attribute__((__always_inline__));
static inline vector signed int spu_rl (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rl (vector unsigned short a, short b) __attribute__((__always_inline__));
static inline vector signed short spu_rl (vector signed short a, short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rl (vector unsigned int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_rl (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_rlqw (vector unsigned char a, int b) __attribute__((__always_inline__));
static inline vector signed char spu_rlqw (vector signed char a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rlqw (vector unsigned short a, int b) __attribute__((__always_inline__));
static inline vector signed short spu_rlqw (vector signed short a, int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rlqw (vector unsigned int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_rlqw (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_rlqw (vector unsigned long long a, int b) __attribute__((__always_inline__));
static inline vector signed long long spu_rlqw (vector signed long long a, int b) __attribute__((__always_inline__));
static inline vector float spu_rlqw (vector float a, int b) __attribute__((__always_inline__));
static inline vector double spu_rlqw (vector double a, int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_rlqwbyte (vector unsigned char a, int b) __attribute__((__always_inline__));
static inline vector signed char spu_rlqwbyte (vector signed char a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rlqwbyte (vector unsigned short a, int b) __attribute__((__always_inline__));
static inline vector signed short spu_rlqwbyte (vector signed short a, int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rlqwbyte (vector unsigned int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_rlqwbyte (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_rlqwbyte (vector unsigned long long a, int b) __attribute__((__always_inline__));
static inline vector signed long long spu_rlqwbyte (vector signed long long a, int b) __attribute__((__always_inline__));
static inline vector float spu_rlqwbyte (vector float a, int b) __attribute__((__always_inline__));
static inline vector double spu_rlqwbyte (vector double a, int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_rlqwbytebc (vector unsigned char a, int b) __attribute__((__always_inline__));
static inline vector signed char spu_rlqwbytebc (vector signed char a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rlqwbytebc (vector unsigned short a, int b) __attribute__((__always_inline__));
static inline vector signed short spu_rlqwbytebc (vector signed short a, int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rlqwbytebc (vector unsigned int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_rlqwbytebc (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_rlqwbytebc (vector unsigned long long a, int b) __attribute__((__always_inline__));
static inline vector signed long long spu_rlqwbytebc (vector signed long long a, int b) __attribute__((__always_inline__));
static inline vector float spu_rlqwbytebc (vector float a, int b) __attribute__((__always_inline__));
static inline vector double spu_rlqwbytebc (vector double a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rlmask (vector unsigned short a, vector signed short b) __attribute__((__always_inline__));
static inline vector signed short spu_rlmask (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rlmask (vector unsigned int a, vector signed int b) __attribute__((__always_inline__));
static inline vector signed int spu_rlmask (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rlmask (vector unsigned short a, int b) __attribute__((__always_inline__));
static inline vector signed short spu_rlmask (vector signed short a, int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rlmask (vector unsigned int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_rlmask (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rlmaska (vector unsigned short a, vector signed short b) __attribute__((__always_inline__));
static inline vector signed short spu_rlmaska (vector signed short a, vector signed short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rlmaska (vector unsigned int a, vector signed int b) __attribute__((__always_inline__));
static inline vector signed int spu_rlmaska (vector signed int a, vector signed int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rlmaska (vector unsigned short a, int b) __attribute__((__always_inline__));
static inline vector signed short spu_rlmaska (vector signed short a, int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rlmaska (vector unsigned int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_rlmaska (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_rlmaskqw (vector unsigned char a, int b) __attribute__((__always_inline__));
static inline vector signed char spu_rlmaskqw (vector signed char a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rlmaskqw (vector unsigned short a, int b) __attribute__((__always_inline__));
static inline vector signed short spu_rlmaskqw (vector signed short a, int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rlmaskqw (vector unsigned int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_rlmaskqw (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_rlmaskqw (vector unsigned long long a, int b) __attribute__((__always_inline__));
static inline vector signed long long spu_rlmaskqw (vector signed long long a, int b) __attribute__((__always_inline__));
static inline vector float spu_rlmaskqw (vector float a, int b) __attribute__((__always_inline__));
static inline vector double spu_rlmaskqw (vector double a, int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_rlmaskqwbyte (vector unsigned char a, int b) __attribute__((__always_inline__));
static inline vector signed char spu_rlmaskqwbyte (vector signed char a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rlmaskqwbyte (vector unsigned short a, int b) __attribute__((__always_inline__));
static inline vector signed short spu_rlmaskqwbyte (vector signed short a, int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rlmaskqwbyte (vector unsigned int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_rlmaskqwbyte (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_rlmaskqwbyte (vector unsigned long long a, int b) __attribute__((__always_inline__));
static inline vector signed long long spu_rlmaskqwbyte (vector signed long long a, int b) __attribute__((__always_inline__));
static inline vector float spu_rlmaskqwbyte (vector float a, int b) __attribute__((__always_inline__));
static inline vector double spu_rlmaskqwbyte (vector double a, int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_rlmaskqwbytebc (vector unsigned char a, int b) __attribute__((__always_inline__));
static inline vector signed char spu_rlmaskqwbytebc (vector signed char a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_rlmaskqwbytebc (vector unsigned short a, int b) __attribute__((__always_inline__));
static inline vector signed short spu_rlmaskqwbytebc (vector signed short a, int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_rlmaskqwbytebc (vector unsigned int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_rlmaskqwbytebc (vector signed int a, int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_rlmaskqwbytebc (vector unsigned long long a, int b) __attribute__((__always_inline__));
static inline vector signed long long spu_rlmaskqwbytebc (vector signed long long a, int b) __attribute__((__always_inline__));
static inline vector float spu_rlmaskqwbytebc (vector float a, int b) __attribute__((__always_inline__));
static inline vector double spu_rlmaskqwbytebc (vector double a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_sl (vector unsigned short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector signed short spu_sl (vector signed short a, vector unsigned short b) __attribute__((__always_inline__));
static inline vector unsigned int spu_sl (vector unsigned int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_sl (vector signed int a, vector unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_sl (vector unsigned short a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed short spu_sl (vector signed short a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_sl (vector unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_sl (vector signed int a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed long long spu_slqw (vector signed long long a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_slqw (vector unsigned long long a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_slqw (vector signed int a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_slqw (vector unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed short spu_slqw (vector signed short a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_slqw (vector unsigned short a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed char spu_slqw (vector signed char a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_slqw (vector unsigned char a, unsigned int b) __attribute__((__always_inline__));
static inline vector float spu_slqw (vector float a, unsigned int b) __attribute__((__always_inline__));
static inline vector double spu_slqw (vector double a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed long long spu_slqwbyte (vector signed long long a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_slqwbyte (vector unsigned long long a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_slqwbyte (vector signed int a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_slqwbyte (vector unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed short spu_slqwbyte (vector signed short a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_slqwbyte (vector unsigned short a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed char spu_slqwbyte (vector signed char a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_slqwbyte (vector unsigned char a, unsigned int b) __attribute__((__always_inline__));
static inline vector float spu_slqwbyte (vector float a, unsigned int b) __attribute__((__always_inline__));
static inline vector double spu_slqwbyte (vector double a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed long long spu_slqwbytebc (vector signed long long a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_slqwbytebc (vector unsigned long long a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed int spu_slqwbytebc (vector signed int a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_slqwbytebc (vector unsigned int a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed short spu_slqwbytebc (vector signed short a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_slqwbytebc (vector unsigned short a, unsigned int b) __attribute__((__always_inline__));
static inline vector signed char spu_slqwbytebc (vector signed char a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_slqwbytebc (vector unsigned char a, unsigned int b) __attribute__((__always_inline__));
static inline vector float spu_slqwbytebc (vector float a, unsigned int b) __attribute__((__always_inline__));
static inline vector double spu_slqwbytebc (vector double a, unsigned int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_splats (unsigned char a) __attribute__((__always_inline__));
static inline vector signed char spu_splats (signed char a) __attribute__((__always_inline__));
static inline vector signed char spu_splats (char a) __attribute__((__always_inline__));
static inline vector unsigned short spu_splats (unsigned short a) __attribute__((__always_inline__));
static inline vector signed short spu_splats (short a) __attribute__((__always_inline__));
static inline vector unsigned int spu_splats (unsigned int a) __attribute__((__always_inline__));
static inline vector signed int spu_splats (int a) __attribute__((__always_inline__));
static inline vector unsigned long long spu_splats (unsigned long long a) __attribute__((__always_inline__));
static inline vector signed long long spu_splats (long long a) __attribute__((__always_inline__));
static inline vector float spu_splats (float a) __attribute__((__always_inline__));
static inline vector double spu_splats (double a) __attribute__((__always_inline__));
static inline unsigned char spu_extract (vector unsigned char a, int b) __attribute__((__always_inline__));
static inline signed char spu_extract (vector signed char a, int b) __attribute__((__always_inline__));
static inline unsigned short spu_extract (vector unsigned short a, int b) __attribute__((__always_inline__));
static inline short spu_extract (vector signed short a, int b) __attribute__((__always_inline__));
static inline unsigned int spu_extract (vector unsigned int a, int b) __attribute__((__always_inline__));
static inline int spu_extract (vector signed int a, int b) __attribute__((__always_inline__));
static inline unsigned long long spu_extract (vector unsigned long long a, int b) __attribute__((__always_inline__));
static inline long long spu_extract (vector signed long long a, int b) __attribute__((__always_inline__));
static inline float spu_extract (vector float a, int b) __attribute__((__always_inline__));
static inline double spu_extract (vector double a, int b) __attribute__((__always_inline__));
static inline vector unsigned char spu_insert (unsigned char a, vector unsigned char b, int c) __attribute__((__always_inline__));
static inline vector signed char spu_insert (signed char a, vector signed char b, int c) __attribute__((__always_inline__));
static inline vector unsigned short spu_insert (unsigned short a, vector unsigned short b, int c) __attribute__((__always_inline__));
static inline vector signed short spu_insert (short a, vector signed short b, int c) __attribute__((__always_inline__));
static inline vector unsigned int spu_insert (unsigned int a, vector unsigned int b, int c) __attribute__((__always_inline__));
static inline vector signed int spu_insert (int a, vector signed int b, int c) __attribute__((__always_inline__));
static inline vector unsigned long long spu_insert (unsigned long long a, vector unsigned long long b, int c) __attribute__((__always_inline__));
static inline vector signed long long spu_insert (long long a, vector signed long long b, int c) __attribute__((__always_inline__));
static inline vector float spu_insert (float a, vector float b, int c) __attribute__((__always_inline__));
static inline vector double spu_insert (double a, vector double b, int c) __attribute__((__always_inline__));
static inline vector unsigned char spu_promote (unsigned char a, int b) __attribute__((__always_inline__));
static inline vector signed char spu_promote (signed char a, int b) __attribute__((__always_inline__));
static inline vector signed char spu_promote (char a, int b) __attribute__((__always_inline__));
static inline vector unsigned short spu_promote (unsigned short a, int b) __attribute__((__always_inline__));
static inline vector signed short spu_promote (short a, int b) __attribute__((__always_inline__));
static inline vector unsigned int spu_promote (unsigned int a, int b) __attribute__((__always_inline__));
static inline vector signed int spu_promote (int a, int b) __attribute__((__always_inline__));
static inline vector unsigned long long spu_promote (unsigned long long a, int b) __attribute__((__always_inline__));
static inline vector signed long long spu_promote (long long a, int b) __attribute__((__always_inline__));
static inline vector float spu_promote (float a, int b) __attribute__((__always_inline__));
static inline vector double spu_promote (double a, int b) __attribute__((__always_inline__));

static inline vector signed short
spu_extend (vector signed char a)
{
  return __builtin_spu_extend_0 (a);
}
static inline vector signed int
spu_extend (vector signed short a)
{
  return __builtin_spu_extend_1 (a);
}
static inline vector signed long long
spu_extend (vector signed int a)
{
  return __builtin_spu_extend_2 (a);
}
static inline vector double
spu_extend (vector float a)
{
  return __builtin_spu_extend_3 (a);
}
static inline vector unsigned int
spu_add (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_add_0 (a, b);
}
static inline vector signed int
spu_add (vector signed int a, vector signed int b)
{
  return __builtin_spu_add_1 (a, b);
}
static inline vector unsigned short
spu_add (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_add_2 (a, b);
}
static inline vector signed short
spu_add (vector signed short a, vector signed short b)
{
  return __builtin_spu_add_3 (a, b);
}
static inline vector float
spu_add (vector float a, vector float b)
{
  return __builtin_spu_add_4 (a, b);
}
static inline vector double
spu_add (vector double a, vector double b)
{
  return __builtin_spu_add_5 (a, b);
}
static inline vector unsigned short
spu_add (vector unsigned short a, unsigned short b)
{
  return __builtin_spu_add_6 (a, b);
}
static inline vector signed short
spu_add (vector signed short a, short b)
{
  return __builtin_spu_add_7 (a, b);
}
static inline vector unsigned int
spu_add (vector unsigned int a, unsigned int b)
{
  return __builtin_spu_add_8 (a, b);
}
static inline vector signed int
spu_add (vector signed int a, int b)
{
  return __builtin_spu_add_9 (a, b);
}
static inline vector signed int
spu_addx (vector signed int a, vector signed int b, vector signed int c)
{
  return __builtin_spu_addx_0 (a, b, c);
}
static inline vector unsigned int
spu_addx (vector unsigned int a, vector unsigned int b, vector unsigned int c)
{
  return __builtin_spu_addx_1 (a, b, c);
}
static inline vector signed int
spu_genc (vector signed int a, vector signed int b)
{
  return __builtin_spu_genc_0 (a, b);
}
static inline vector unsigned int
spu_genc (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_genc_1 (a, b);
}
static inline vector signed int
spu_gencx (vector signed int a, vector signed int b, vector signed int c)
{
  return __builtin_spu_gencx_0 (a, b, c);
}
static inline vector unsigned int
spu_gencx (vector unsigned int a, vector unsigned int b, vector unsigned int c)
{
  return __builtin_spu_gencx_1 (a, b, c);
}
static inline vector signed int
spu_madd (vector signed short a, vector signed short b, vector signed int c)
{
  return __builtin_spu_madd_0 (a, b, c);
}
static inline vector float
spu_madd (vector float a, vector float b, vector float c)
{
  return __builtin_spu_madd_1 (a, b, c);
}
static inline vector double
spu_madd (vector double a, vector double b, vector double c)
{
  return __builtin_spu_madd_2 (a, b, c);
}
static inline vector float
spu_msub (vector float a, vector float b, vector float c)
{
  return __builtin_spu_msub_0 (a, b, c);
}
static inline vector double
spu_msub (vector double a, vector double b, vector double c)
{
  return __builtin_spu_msub_1 (a, b, c);
}
static inline vector unsigned int
spu_mhhadd (vector unsigned short a, vector unsigned short b, vector unsigned int c)
{
  return __builtin_spu_mhhadd_0 (a, b, c);
}
static inline vector signed int
spu_mhhadd (vector signed short a, vector signed short b, vector signed int c)
{
  return __builtin_spu_mhhadd_1 (a, b, c);
}
static inline vector unsigned int
spu_mule (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_mule_0 (a, b);
}
static inline vector signed int
spu_mule (vector signed short a, vector signed short b)
{
  return __builtin_spu_mule_1 (a, b);
}
static inline vector float
spu_mul (vector float a, vector float b)
{
  return __builtin_spu_mul_0 (a, b);
}
static inline vector double
spu_mul (vector double a, vector double b)
{
  return __builtin_spu_mul_1 (a, b);
}
static inline vector signed int
spu_mulo (vector signed short a, vector signed short b)
{
  return __builtin_spu_mulo_0 (a, b);
}
static inline vector unsigned int
spu_mulo (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_mulo_1 (a, b);
}
static inline vector signed int
spu_mulo (vector signed short a, short b)
{
  return __builtin_spu_mulo_2 (a, b);
}
static inline vector unsigned int
spu_mulo (vector unsigned short a, unsigned short b)
{
  return __builtin_spu_mulo_3 (a, b);
}
static inline vector float
spu_nmsub (vector float a, vector float b, vector float c)
{
  return __builtin_spu_nmsub_0 (a, b, c);
}
static inline vector double
spu_nmsub (vector double a, vector double b, vector double c)
{
  return __builtin_spu_nmsub_1 (a, b, c);
}
static inline vector unsigned short
spu_sub (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_sub_0 (a, b);
}
static inline vector signed short
spu_sub (vector signed short a, vector signed short b)
{
  return __builtin_spu_sub_1 (a, b);
}
static inline vector unsigned int
spu_sub (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_sub_2 (a, b);
}
static inline vector signed int
spu_sub (vector signed int a, vector signed int b)
{
  return __builtin_spu_sub_3 (a, b);
}
static inline vector float
spu_sub (vector float a, vector float b)
{
  return __builtin_spu_sub_4 (a, b);
}
static inline vector double
spu_sub (vector double a, vector double b)
{
  return __builtin_spu_sub_5 (a, b);
}
static inline vector unsigned short
spu_sub (unsigned short a, vector unsigned short b)
{
  return __builtin_spu_sub_6 (a, b);
}
static inline vector signed short
spu_sub (short a, vector signed short b)
{
  return __builtin_spu_sub_7 (a, b);
}
static inline vector unsigned int
spu_sub (unsigned int a, vector unsigned int b)
{
  return __builtin_spu_sub_8 (a, b);
}
static inline vector signed int
spu_sub (int a, vector signed int b)
{
  return __builtin_spu_sub_9 (a, b);
}
static inline vector unsigned int
spu_subx (vector unsigned int a, vector unsigned int b, vector unsigned int c)
{
  return __builtin_spu_subx_0 (a, b, c);
}
static inline vector signed int
spu_subx (vector signed int a, vector signed int b, vector signed int c)
{
  return __builtin_spu_subx_1 (a, b, c);
}
static inline vector unsigned int
spu_genb (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_genb_0 (a, b);
}
static inline vector signed int
spu_genb (vector signed int a, vector signed int b)
{
  return __builtin_spu_genb_1 (a, b);
}
static inline vector unsigned int
spu_genbx (vector unsigned int a, vector unsigned int b, vector unsigned int c)
{
  return __builtin_spu_genbx_0 (a, b, c);
}
static inline vector signed int
spu_genbx (vector signed int a, vector signed int b, vector signed int c)
{
  return __builtin_spu_genbx_1 (a, b, c);
}
static inline vector unsigned char
spu_cmpeq (vector unsigned char a, vector unsigned char b)
{
  return __builtin_spu_cmpeq_0 (a, b);
}
static inline vector unsigned char
spu_cmpeq (vector signed char a, vector signed char b)
{
  return __builtin_spu_cmpeq_1 (a, b);
}
static inline vector unsigned short
spu_cmpeq (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_cmpeq_2 (a, b);
}
static inline vector unsigned short
spu_cmpeq (vector signed short a, vector signed short b)
{
  return __builtin_spu_cmpeq_3 (a, b);
}
static inline vector unsigned int
spu_cmpeq (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_cmpeq_4 (a, b);
}
static inline vector unsigned int
spu_cmpeq (vector signed int a, vector signed int b)
{
  return __builtin_spu_cmpeq_5 (a, b);
}
static inline vector unsigned int
spu_cmpeq (vector float a, vector float b)
{
  return __builtin_spu_cmpeq_6 (a, b);
}
static inline vector unsigned char
spu_cmpeq (vector unsigned char a, unsigned char b)
{
  return __builtin_spu_cmpeq_7 (a, b);
}
static inline vector unsigned char
spu_cmpeq (vector signed char a, signed char b)
{
  return __builtin_spu_cmpeq_8 (a, b);
}
static inline vector unsigned short
spu_cmpeq (vector unsigned short a, unsigned short b)
{
  return __builtin_spu_cmpeq_9 (a, b);
}
static inline vector unsigned short
spu_cmpeq (vector signed short a, short b)
{
  return __builtin_spu_cmpeq_10 (a, b);
}
static inline vector unsigned int
spu_cmpeq (vector unsigned int a, unsigned int b)
{
  return __builtin_spu_cmpeq_11 (a, b);
}
static inline vector unsigned int
spu_cmpeq (vector signed int a, int b)
{
  return __builtin_spu_cmpeq_12 (a, b);
}
static inline vector unsigned char
spu_cmpgt (vector unsigned char a, vector unsigned char b)
{
  return __builtin_spu_cmpgt_0 (a, b);
}
static inline vector unsigned char
spu_cmpgt (vector signed char a, vector signed char b)
{
  return __builtin_spu_cmpgt_1 (a, b);
}
static inline vector unsigned short
spu_cmpgt (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_cmpgt_2 (a, b);
}
static inline vector unsigned short
spu_cmpgt (vector signed short a, vector signed short b)
{
  return __builtin_spu_cmpgt_3 (a, b);
}
static inline vector unsigned int
spu_cmpgt (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_cmpgt_4 (a, b);
}
static inline vector unsigned int
spu_cmpgt (vector signed int a, vector signed int b)
{
  return __builtin_spu_cmpgt_5 (a, b);
}
static inline vector unsigned int
spu_cmpgt (vector float a, vector float b)
{
  return __builtin_spu_cmpgt_6 (a, b);
}
static inline vector unsigned char
spu_cmpgt (vector unsigned char a, unsigned char b)
{
  return __builtin_spu_cmpgt_7 (a, b);
}
static inline vector unsigned char
spu_cmpgt (vector signed char a, signed char b)
{
  return __builtin_spu_cmpgt_8 (a, b);
}
static inline vector unsigned short
spu_cmpgt (vector unsigned short a, unsigned short b)
{
  return __builtin_spu_cmpgt_9 (a, b);
}
static inline vector unsigned short
spu_cmpgt (vector signed short a, short b)
{
  return __builtin_spu_cmpgt_10 (a, b);
}
static inline vector unsigned int
spu_cmpgt (vector signed int a, int b)
{
  return __builtin_spu_cmpgt_11 (a, b);
}
static inline vector unsigned int
spu_cmpgt (vector unsigned int a, unsigned int b)
{
  return __builtin_spu_cmpgt_12 (a, b);
}
static inline void
spu_hcmpeq (int a, int b)
{
  return __builtin_spu_hcmpeq_0 (a, b);
}
static inline void
spu_hcmpeq (unsigned int a, unsigned int b)
{
  return __builtin_spu_hcmpeq_1 (a, b);
}
static inline void
spu_hcmpgt (int a, int b)
{
  return __builtin_spu_hcmpgt_0 (a, b);
}
static inline void
spu_hcmpgt (unsigned int a, unsigned int b)
{
  return __builtin_spu_hcmpgt_1 (a, b);
}
static inline vector unsigned char
spu_cntb (vector signed char a)
{
  return __builtin_spu_cntb_0 (a);
}
static inline vector unsigned char
spu_cntb (vector unsigned char a)
{
  return __builtin_spu_cntb_1 (a);
}
static inline vector unsigned int
spu_cntlz (vector signed int a)
{
  return __builtin_spu_cntlz_0 (a);
}
static inline vector unsigned int
spu_cntlz (vector unsigned int a)
{
  return __builtin_spu_cntlz_1 (a);
}
static inline vector unsigned int
spu_cntlz (vector float a)
{
  return __builtin_spu_cntlz_2 (a);
}
static inline vector unsigned int
spu_gather (vector signed int a)
{
  return __builtin_spu_gather_0 (a);
}
static inline vector unsigned int
spu_gather (vector unsigned int a)
{
  return __builtin_spu_gather_1 (a);
}
static inline vector unsigned int
spu_gather (vector signed short a)
{
  return __builtin_spu_gather_2 (a);
}
static inline vector unsigned int
spu_gather (vector unsigned short a)
{
  return __builtin_spu_gather_3 (a);
}
static inline vector unsigned int
spu_gather (vector signed char a)
{
  return __builtin_spu_gather_4 (a);
}
static inline vector unsigned int
spu_gather (vector unsigned char a)
{
  return __builtin_spu_gather_5 (a);
}
static inline vector unsigned int
spu_gather (vector float a)
{
  return __builtin_spu_gather_6 (a);
}
static inline vector unsigned char
spu_maskb (unsigned short a)
{
  return __builtin_spu_maskb_0 (a);
}
static inline vector unsigned char
spu_maskb (short a)
{
  return __builtin_spu_maskb_1 (a);
}
static inline vector unsigned char
spu_maskb (unsigned int a)
{
  return __builtin_spu_maskb_2 (a);
}
static inline vector unsigned char
spu_maskb (int a)
{
  return __builtin_spu_maskb_3 (a);
}
static inline vector unsigned short
spu_maskh (unsigned char a)
{
  return __builtin_spu_maskh_0 (a);
}
static inline vector unsigned short
spu_maskh (signed char a)
{
  return __builtin_spu_maskh_1 (a);
}
static inline vector unsigned short
spu_maskh (char a)
{
  return __builtin_spu_maskh_1 (a);
}
static inline vector unsigned short
spu_maskh (unsigned short a)
{
  return __builtin_spu_maskh_2 (a);
}
static inline vector unsigned short
spu_maskh (short a)
{
  return __builtin_spu_maskh_3 (a);
}
static inline vector unsigned short
spu_maskh (unsigned int a)
{
  return __builtin_spu_maskh_4 (a);
}
static inline vector unsigned short
spu_maskh (int a)
{
  return __builtin_spu_maskh_5 (a);
}
static inline vector unsigned int
spu_maskw (unsigned char a)
{
  return __builtin_spu_maskw_0 (a);
}
static inline vector unsigned int
spu_maskw (signed char a)
{
  return __builtin_spu_maskw_1 (a);
}
static inline vector unsigned int
spu_maskw (char a)
{
  return __builtin_spu_maskw_1 (a);
}
static inline vector unsigned int
spu_maskw (unsigned short a)
{
  return __builtin_spu_maskw_2 (a);
}
static inline vector unsigned int
spu_maskw (short a)
{
  return __builtin_spu_maskw_3 (a);
}
static inline vector unsigned int
spu_maskw (unsigned int a)
{
  return __builtin_spu_maskw_4 (a);
}
static inline vector unsigned int
spu_maskw (int a)
{
  return __builtin_spu_maskw_5 (a);
}
static inline vector signed long long
spu_sel (vector signed long long a, vector signed long long b, vector unsigned long long c)
{
  return __builtin_spu_sel_0 (a, b, c);
}
static inline vector unsigned long long
spu_sel (vector unsigned long long a, vector unsigned long long b, vector unsigned long long c)
{
  return __builtin_spu_sel_1 (a, b, c);
}
static inline vector signed int
spu_sel (vector signed int a, vector signed int b, vector unsigned int c)
{
  return __builtin_spu_sel_2 (a, b, c);
}
static inline vector unsigned int
spu_sel (vector unsigned int a, vector unsigned int b, vector unsigned int c)
{
  return __builtin_spu_sel_3 (a, b, c);
}
static inline vector signed short
spu_sel (vector signed short a, vector signed short b, vector unsigned short c)
{
  return __builtin_spu_sel_4 (a, b, c);
}
static inline vector unsigned short
spu_sel (vector unsigned short a, vector unsigned short b, vector unsigned short c)
{
  return __builtin_spu_sel_5 (a, b, c);
}
static inline vector signed char
spu_sel (vector signed char a, vector signed char b, vector unsigned char c)
{
  return __builtin_spu_sel_6 (a, b, c);
}
static inline vector unsigned char
spu_sel (vector unsigned char a, vector unsigned char b, vector unsigned char c)
{
  return __builtin_spu_sel_7 (a, b, c);
}
static inline vector float
spu_sel (vector float a, vector float b, vector unsigned int c)
{
  return __builtin_spu_sel_8 (a, b, c);
}
static inline vector double
spu_sel (vector double a, vector double b, vector unsigned long long c)
{
  return __builtin_spu_sel_9 (a, b, c);
}
static inline vector signed long long
spu_sel (vector signed long long a, vector signed long long b, vector unsigned char c)
{
  return __builtin_spu_sel_0o (a, b, c);
}
static inline vector unsigned long long
spu_sel (vector unsigned long long a, vector unsigned long long b, vector unsigned char c)
{
  return __builtin_spu_sel_1o (a, b, c);
}
static inline vector signed int
spu_sel (vector signed int a, vector signed int b, vector unsigned char c)
{
  return __builtin_spu_sel_2o (a, b, c);
}
static inline vector unsigned int
spu_sel (vector unsigned int a, vector unsigned int b, vector unsigned char c)
{
  return __builtin_spu_sel_3o (a, b, c);
}
static inline vector signed short
spu_sel (vector signed short a, vector signed short b, vector unsigned char c)
{
  return __builtin_spu_sel_4o (a, b, c);
}
static inline vector unsigned short
spu_sel (vector unsigned short a, vector unsigned short b, vector unsigned char c)
{
  return __builtin_spu_sel_5o (a, b, c);
}
static inline vector float
spu_sel (vector float a, vector float b, vector unsigned char c)
{
  return __builtin_spu_sel_8o (a, b, c);
}
static inline vector double
spu_sel (vector double a, vector double b, vector unsigned char c)
{
  return __builtin_spu_sel_9o (a, b, c);
}
static inline vector unsigned char
spu_shuffle (vector unsigned char a, vector unsigned char b, vector unsigned char c)
{
  return __builtin_spu_shuffle_0 (a, b, c);
}
static inline vector signed char
spu_shuffle (vector signed char a, vector signed char b, vector unsigned char c)
{
  return __builtin_spu_shuffle_1 (a, b, c);
}
static inline vector unsigned short
spu_shuffle (vector unsigned short a, vector unsigned short b, vector unsigned char c)
{
  return __builtin_spu_shuffle_2 (a, b, c);
}
static inline vector signed short
spu_shuffle (vector signed short a, vector signed short b, vector unsigned char c)
{
  return __builtin_spu_shuffle_3 (a, b, c);
}
static inline vector unsigned int
spu_shuffle (vector unsigned int a, vector unsigned int b, vector unsigned char c)
{
  return __builtin_spu_shuffle_4 (a, b, c);
}
static inline vector signed int
spu_shuffle (vector signed int a, vector signed int b, vector unsigned char c)
{
  return __builtin_spu_shuffle_5 (a, b, c);
}
static inline vector unsigned long long
spu_shuffle (vector unsigned long long a, vector unsigned long long b, vector unsigned char c)
{
  return __builtin_spu_shuffle_6 (a, b, c);
}
static inline vector signed long long
spu_shuffle (vector signed long long a, vector signed long long b, vector unsigned char c)
{
  return __builtin_spu_shuffle_7 (a, b, c);
}
static inline vector float
spu_shuffle (vector float a, vector float b, vector unsigned char c)
{
  return __builtin_spu_shuffle_8 (a, b, c);
}
static inline vector double
spu_shuffle (vector double a, vector double b, vector unsigned char c)
{
  return __builtin_spu_shuffle_9 (a, b, c);
}
static inline vector unsigned char
spu_and (vector unsigned char a, vector unsigned char b)
{
  return __builtin_spu_and_0 (a, b);
}
static inline vector signed char
spu_and (vector signed char a, vector signed char b)
{
  return __builtin_spu_and_1 (a, b);
}
static inline vector unsigned short
spu_and (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_and_2 (a, b);
}
static inline vector signed short
spu_and (vector signed short a, vector signed short b)
{
  return __builtin_spu_and_3 (a, b);
}
static inline vector unsigned int
spu_and (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_and_4 (a, b);
}
static inline vector signed int
spu_and (vector signed int a, vector signed int b)
{
  return __builtin_spu_and_5 (a, b);
}
static inline vector unsigned long long
spu_and (vector unsigned long long a, vector unsigned long long b)
{
  return __builtin_spu_and_6 (a, b);
}
static inline vector signed long long
spu_and (vector signed long long a, vector signed long long b)
{
  return __builtin_spu_and_7 (a, b);
}
static inline vector float
spu_and (vector float a, vector float b)
{
  return __builtin_spu_and_8 (a, b);
}
static inline vector double
spu_and (vector double a, vector double b)
{
  return __builtin_spu_and_9 (a, b);
}
static inline vector unsigned char
spu_and (vector unsigned char a, unsigned char b)
{
  return __builtin_spu_and_10 (a, b);
}
static inline vector signed char
spu_and (vector signed char a, signed char b)
{
  return __builtin_spu_and_11 (a, b);
}
static inline vector unsigned short
spu_and (vector unsigned short a, unsigned short b)
{
  return __builtin_spu_and_12 (a, b);
}
static inline vector signed short
spu_and (vector signed short a, short b)
{
  return __builtin_spu_and_13 (a, b);
}
static inline vector unsigned int
spu_and (vector unsigned int a, unsigned int b)
{
  return __builtin_spu_and_14 (a, b);
}
static inline vector signed int
spu_and (vector signed int a, int b)
{
  return __builtin_spu_and_15 (a, b);
}
static inline vector signed long long
spu_andc (vector signed long long a, vector signed long long b)
{
  return __builtin_spu_andc_0 (a, b);
}
static inline vector unsigned long long
spu_andc (vector unsigned long long a, vector unsigned long long b)
{
  return __builtin_spu_andc_1 (a, b);
}
static inline vector signed int
spu_andc (vector signed int a, vector signed int b)
{
  return __builtin_spu_andc_2 (a, b);
}
static inline vector unsigned int
spu_andc (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_andc_3 (a, b);
}
static inline vector signed short
spu_andc (vector signed short a, vector signed short b)
{
  return __builtin_spu_andc_4 (a, b);
}
static inline vector unsigned short
spu_andc (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_andc_5 (a, b);
}
static inline vector signed char
spu_andc (vector signed char a, vector signed char b)
{
  return __builtin_spu_andc_6 (a, b);
}
static inline vector unsigned char
spu_andc (vector unsigned char a, vector unsigned char b)
{
  return __builtin_spu_andc_7 (a, b);
}
static inline vector float
spu_andc (vector float a, vector float b)
{
  return __builtin_spu_andc_8 (a, b);
}
static inline vector double
spu_andc (vector double a, vector double b)
{
  return __builtin_spu_andc_9 (a, b);
}
static inline vector signed long long
spu_eqv (vector signed long long a, vector signed long long b)
{
  return __builtin_spu_eqv_0 (a, b);
}
static inline vector unsigned long long
spu_eqv (vector unsigned long long a, vector unsigned long long b)
{
  return __builtin_spu_eqv_1 (a, b);
}
static inline vector signed int
spu_eqv (vector signed int a, vector signed int b)
{
  return __builtin_spu_eqv_2 (a, b);
}
static inline vector unsigned int
spu_eqv (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_eqv_3 (a, b);
}
static inline vector signed short
spu_eqv (vector signed short a, vector signed short b)
{
  return __builtin_spu_eqv_4 (a, b);
}
static inline vector unsigned short
spu_eqv (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_eqv_5 (a, b);
}
static inline vector signed char
spu_eqv (vector signed char a, vector signed char b)
{
  return __builtin_spu_eqv_6 (a, b);
}
static inline vector unsigned char
spu_eqv (vector unsigned char a, vector unsigned char b)
{
  return __builtin_spu_eqv_7 (a, b);
}
static inline vector float
spu_eqv (vector float a, vector float b)
{
  return __builtin_spu_eqv_8 (a, b);
}
static inline vector double
spu_eqv (vector double a, vector double b)
{
  return __builtin_spu_eqv_9 (a, b);
}
static inline vector signed long long
spu_nand (vector signed long long a, vector signed long long b)
{
  return __builtin_spu_nand_0 (a, b);
}
static inline vector unsigned long long
spu_nand (vector unsigned long long a, vector unsigned long long b)
{
  return __builtin_spu_nand_1 (a, b);
}
static inline vector signed int
spu_nand (vector signed int a, vector signed int b)
{
  return __builtin_spu_nand_2 (a, b);
}
static inline vector unsigned int
spu_nand (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_nand_3 (a, b);
}
static inline vector signed short
spu_nand (vector signed short a, vector signed short b)
{
  return __builtin_spu_nand_4 (a, b);
}
static inline vector unsigned short
spu_nand (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_nand_5 (a, b);
}
static inline vector signed char
spu_nand (vector signed char a, vector signed char b)
{
  return __builtin_spu_nand_6 (a, b);
}
static inline vector unsigned char
spu_nand (vector unsigned char a, vector unsigned char b)
{
  return __builtin_spu_nand_7 (a, b);
}
static inline vector float
spu_nand (vector float a, vector float b)
{
  return __builtin_spu_nand_8 (a, b);
}
static inline vector double
spu_nand (vector double a, vector double b)
{
  return __builtin_spu_nand_9 (a, b);
}
static inline vector signed long long
spu_nor (vector signed long long a, vector signed long long b)
{
  return __builtin_spu_nor_0 (a, b);
}
static inline vector unsigned long long
spu_nor (vector unsigned long long a, vector unsigned long long b)
{
  return __builtin_spu_nor_1 (a, b);
}
static inline vector signed int
spu_nor (vector signed int a, vector signed int b)
{
  return __builtin_spu_nor_2 (a, b);
}
static inline vector unsigned int
spu_nor (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_nor_3 (a, b);
}
static inline vector signed short
spu_nor (vector signed short a, vector signed short b)
{
  return __builtin_spu_nor_4 (a, b);
}
static inline vector unsigned short
spu_nor (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_nor_5 (a, b);
}
static inline vector signed char
spu_nor (vector signed char a, vector signed char b)
{
  return __builtin_spu_nor_6 (a, b);
}
static inline vector unsigned char
spu_nor (vector unsigned char a, vector unsigned char b)
{
  return __builtin_spu_nor_7 (a, b);
}
static inline vector float
spu_nor (vector float a, vector float b)
{
  return __builtin_spu_nor_8 (a, b);
}
static inline vector double
spu_nor (vector double a, vector double b)
{
  return __builtin_spu_nor_9 (a, b);
}
static inline vector unsigned char
spu_or (vector unsigned char a, vector unsigned char b)
{
  return __builtin_spu_or_0 (a, b);
}
static inline vector signed char
spu_or (vector signed char a, vector signed char b)
{
  return __builtin_spu_or_1 (a, b);
}
static inline vector unsigned short
spu_or (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_or_2 (a, b);
}
static inline vector signed short
spu_or (vector signed short a, vector signed short b)
{
  return __builtin_spu_or_3 (a, b);
}
static inline vector unsigned int
spu_or (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_or_4 (a, b);
}
static inline vector signed int
spu_or (vector signed int a, vector signed int b)
{
  return __builtin_spu_or_5 (a, b);
}
static inline vector unsigned long long
spu_or (vector unsigned long long a, vector unsigned long long b)
{
  return __builtin_spu_or_6 (a, b);
}
static inline vector signed long long
spu_or (vector signed long long a, vector signed long long b)
{
  return __builtin_spu_or_7 (a, b);
}
static inline vector float
spu_or (vector float a, vector float b)
{
  return __builtin_spu_or_8 (a, b);
}
static inline vector double
spu_or (vector double a, vector double b)
{
  return __builtin_spu_or_9 (a, b);
}
static inline vector unsigned char
spu_or (vector unsigned char a, unsigned char b)
{
  return __builtin_spu_or_10 (a, b);
}
static inline vector signed char
spu_or (vector signed char a, signed char b)
{
  return __builtin_spu_or_11 (a, b);
}
static inline vector unsigned short
spu_or (vector unsigned short a, unsigned short b)
{
  return __builtin_spu_or_12 (a, b);
}
static inline vector signed short
spu_or (vector signed short a, short b)
{
  return __builtin_spu_or_13 (a, b);
}
static inline vector unsigned int
spu_or (vector unsigned int a, unsigned int b)
{
  return __builtin_spu_or_14 (a, b);
}
static inline vector signed int
spu_or (vector signed int a, int b)
{
  return __builtin_spu_or_15 (a, b);
}
static inline vector signed long long
spu_orc (vector signed long long a, vector signed long long b)
{
  return __builtin_spu_orc_0 (a, b);
}
static inline vector unsigned long long
spu_orc (vector unsigned long long a, vector unsigned long long b)
{
  return __builtin_spu_orc_1 (a, b);
}
static inline vector signed int
spu_orc (vector signed int a, vector signed int b)
{
  return __builtin_spu_orc_2 (a, b);
}
static inline vector unsigned int
spu_orc (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_orc_3 (a, b);
}
static inline vector signed short
spu_orc (vector signed short a, vector signed short b)
{
  return __builtin_spu_orc_4 (a, b);
}
static inline vector unsigned short
spu_orc (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_orc_5 (a, b);
}
static inline vector signed char
spu_orc (vector signed char a, vector signed char b)
{
  return __builtin_spu_orc_6 (a, b);
}
static inline vector unsigned char
spu_orc (vector unsigned char a, vector unsigned char b)
{
  return __builtin_spu_orc_7 (a, b);
}
static inline vector float
spu_orc (vector float a, vector float b)
{
  return __builtin_spu_orc_8 (a, b);
}
static inline vector double
spu_orc (vector double a, vector double b)
{
  return __builtin_spu_orc_9 (a, b);
}
static inline vector signed int
spu_orx (vector signed int a)
{
  return __builtin_spu_orx_0 (a);
}
static inline vector unsigned int
spu_orx (vector unsigned int a)
{
  return __builtin_spu_orx_1 (a);
}
static inline vector unsigned char
spu_xor (vector unsigned char a, vector unsigned char b)
{
  return __builtin_spu_xor_0 (a, b);
}
static inline vector signed char
spu_xor (vector signed char a, vector signed char b)
{
  return __builtin_spu_xor_1 (a, b);
}
static inline vector unsigned short
spu_xor (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_xor_2 (a, b);
}
static inline vector signed short
spu_xor (vector signed short a, vector signed short b)
{
  return __builtin_spu_xor_3 (a, b);
}
static inline vector unsigned int
spu_xor (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_xor_4 (a, b);
}
static inline vector signed int
spu_xor (vector signed int a, vector signed int b)
{
  return __builtin_spu_xor_5 (a, b);
}
static inline vector unsigned long long
spu_xor (vector unsigned long long a, vector unsigned long long b)
{
  return __builtin_spu_xor_6 (a, b);
}
static inline vector signed long long
spu_xor (vector signed long long a, vector signed long long b)
{
  return __builtin_spu_xor_7 (a, b);
}
static inline vector float
spu_xor (vector float a, vector float b)
{
  return __builtin_spu_xor_8 (a, b);
}
static inline vector double
spu_xor (vector double a, vector double b)
{
  return __builtin_spu_xor_9 (a, b);
}
static inline vector unsigned char
spu_xor (vector unsigned char a, unsigned char b)
{
  return __builtin_spu_xor_10 (a, b);
}
static inline vector signed char
spu_xor (vector signed char a, signed char b)
{
  return __builtin_spu_xor_11 (a, b);
}
static inline vector unsigned short
spu_xor (vector unsigned short a, unsigned short b)
{
  return __builtin_spu_xor_12 (a, b);
}
static inline vector signed short
spu_xor (vector signed short a, short b)
{
  return __builtin_spu_xor_13 (a, b);
}
static inline vector unsigned int
spu_xor (vector unsigned int a, unsigned int b)
{
  return __builtin_spu_xor_14 (a, b);
}
static inline vector signed int
spu_xor (vector signed int a, int b)
{
  return __builtin_spu_xor_15 (a, b);
}
static inline vector unsigned short
spu_rl (vector unsigned short a, vector signed short b)
{
  return __builtin_spu_rl_0 (a, b);
}
static inline vector signed short
spu_rl (vector signed short a, vector signed short b)
{
  return __builtin_spu_rl_1 (a, b);
}
static inline vector unsigned int
spu_rl (vector unsigned int a, vector signed int b)
{
  return __builtin_spu_rl_2 (a, b);
}
static inline vector signed int
spu_rl (vector signed int a, vector signed int b)
{
  return __builtin_spu_rl_3 (a, b);
}
static inline vector unsigned short
spu_rl (vector unsigned short a, short b)
{
  return __builtin_spu_rl_4 (a, b);
}
static inline vector signed short
spu_rl (vector signed short a, short b)
{
  return __builtin_spu_rl_5 (a, b);
}
static inline vector unsigned int
spu_rl (vector unsigned int a, int b)
{
  return __builtin_spu_rl_6 (a, b);
}
static inline vector signed int
spu_rl (vector signed int a, int b)
{
  return __builtin_spu_rl_7 (a, b);
}
static inline vector unsigned char
spu_rlqw (vector unsigned char a, int b)
{
  return __builtin_spu_rlqw_0 (a, b);
}
static inline vector signed char
spu_rlqw (vector signed char a, int b)
{
  return __builtin_spu_rlqw_1 (a, b);
}
static inline vector unsigned short
spu_rlqw (vector unsigned short a, int b)
{
  return __builtin_spu_rlqw_2 (a, b);
}
static inline vector signed short
spu_rlqw (vector signed short a, int b)
{
  return __builtin_spu_rlqw_3 (a, b);
}
static inline vector unsigned int
spu_rlqw (vector unsigned int a, int b)
{
  return __builtin_spu_rlqw_4 (a, b);
}
static inline vector signed int
spu_rlqw (vector signed int a, int b)
{
  return __builtin_spu_rlqw_5 (a, b);
}
static inline vector unsigned long long
spu_rlqw (vector unsigned long long a, int b)
{
  return __builtin_spu_rlqw_6 (a, b);
}
static inline vector signed long long
spu_rlqw (vector signed long long a, int b)
{
  return __builtin_spu_rlqw_7 (a, b);
}
static inline vector float
spu_rlqw (vector float a, int b)
{
  return __builtin_spu_rlqw_8 (a, b);
}
static inline vector double
spu_rlqw (vector double a, int b)
{
  return __builtin_spu_rlqw_9 (a, b);
}
static inline vector unsigned char
spu_rlqwbyte (vector unsigned char a, int b)
{
  return __builtin_spu_rlqwbyte_0 (a, b);
}
static inline vector signed char
spu_rlqwbyte (vector signed char a, int b)
{
  return __builtin_spu_rlqwbyte_1 (a, b);
}
static inline vector unsigned short
spu_rlqwbyte (vector unsigned short a, int b)
{
  return __builtin_spu_rlqwbyte_2 (a, b);
}
static inline vector signed short
spu_rlqwbyte (vector signed short a, int b)
{
  return __builtin_spu_rlqwbyte_3 (a, b);
}
static inline vector unsigned int
spu_rlqwbyte (vector unsigned int a, int b)
{
  return __builtin_spu_rlqwbyte_4 (a, b);
}
static inline vector signed int
spu_rlqwbyte (vector signed int a, int b)
{
  return __builtin_spu_rlqwbyte_5 (a, b);
}
static inline vector unsigned long long
spu_rlqwbyte (vector unsigned long long a, int b)
{
  return __builtin_spu_rlqwbyte_6 (a, b);
}
static inline vector signed long long
spu_rlqwbyte (vector signed long long a, int b)
{
  return __builtin_spu_rlqwbyte_7 (a, b);
}
static inline vector float
spu_rlqwbyte (vector float a, int b)
{
  return __builtin_spu_rlqwbyte_8 (a, b);
}
static inline vector double
spu_rlqwbyte (vector double a, int b)
{
  return __builtin_spu_rlqwbyte_9 (a, b);
}
static inline vector unsigned char
spu_rlqwbytebc (vector unsigned char a, int b)
{
  return __builtin_spu_rlqwbytebc_0 (a, b);
}
static inline vector signed char
spu_rlqwbytebc (vector signed char a, int b)
{
  return __builtin_spu_rlqwbytebc_1 (a, b);
}
static inline vector unsigned short
spu_rlqwbytebc (vector unsigned short a, int b)
{
  return __builtin_spu_rlqwbytebc_2 (a, b);
}
static inline vector signed short
spu_rlqwbytebc (vector signed short a, int b)
{
  return __builtin_spu_rlqwbytebc_3 (a, b);
}
static inline vector unsigned int
spu_rlqwbytebc (vector unsigned int a, int b)
{
  return __builtin_spu_rlqwbytebc_4 (a, b);
}
static inline vector signed int
spu_rlqwbytebc (vector signed int a, int b)
{
  return __builtin_spu_rlqwbytebc_5 (a, b);
}
static inline vector unsigned long long
spu_rlqwbytebc (vector unsigned long long a, int b)
{
  return __builtin_spu_rlqwbytebc_6 (a, b);
}
static inline vector signed long long
spu_rlqwbytebc (vector signed long long a, int b)
{
  return __builtin_spu_rlqwbytebc_7 (a, b);
}
static inline vector float
spu_rlqwbytebc (vector float a, int b)
{
  return __builtin_spu_rlqwbytebc_8 (a, b);
}
static inline vector double
spu_rlqwbytebc (vector double a, int b)
{
  return __builtin_spu_rlqwbytebc_9 (a, b);
}
static inline vector unsigned short
spu_rlmask (vector unsigned short a, vector signed short b)
{
  return __builtin_spu_rlmask_0 (a, b);
}
static inline vector signed short
spu_rlmask (vector signed short a, vector signed short b)
{
  return __builtin_spu_rlmask_1 (a, b);
}
static inline vector unsigned int
spu_rlmask (vector unsigned int a, vector signed int b)
{
  return __builtin_spu_rlmask_2 (a, b);
}
static inline vector signed int
spu_rlmask (vector signed int a, vector signed int b)
{
  return __builtin_spu_rlmask_3 (a, b);
}
static inline vector unsigned short
spu_rlmask (vector unsigned short a, int b)
{
  return __builtin_spu_rlmask_4 (a, b);
}
static inline vector signed short
spu_rlmask (vector signed short a, int b)
{
  return __builtin_spu_rlmask_5 (a, b);
}
static inline vector unsigned int
spu_rlmask (vector unsigned int a, int b)
{
  return __builtin_spu_rlmask_6 (a, b);
}
static inline vector signed int
spu_rlmask (vector signed int a, int b)
{
  return __builtin_spu_rlmask_7 (a, b);
}
static inline vector unsigned short
spu_rlmaska (vector unsigned short a, vector signed short b)
{
  return __builtin_spu_rlmaska_0 (a, b);
}
static inline vector signed short
spu_rlmaska (vector signed short a, vector signed short b)
{
  return __builtin_spu_rlmaska_1 (a, b);
}
static inline vector unsigned int
spu_rlmaska (vector unsigned int a, vector signed int b)
{
  return __builtin_spu_rlmaska_2 (a, b);
}
static inline vector signed int
spu_rlmaska (vector signed int a, vector signed int b)
{
  return __builtin_spu_rlmaska_3 (a, b);
}
static inline vector unsigned short
spu_rlmaska (vector unsigned short a, int b)
{
  return __builtin_spu_rlmaska_4 (a, b);
}
static inline vector signed short
spu_rlmaska (vector signed short a, int b)
{
  return __builtin_spu_rlmaska_5 (a, b);
}
static inline vector unsigned int
spu_rlmaska (vector unsigned int a, int b)
{
  return __builtin_spu_rlmaska_6 (a, b);
}
static inline vector signed int
spu_rlmaska (vector signed int a, int b)
{
  return __builtin_spu_rlmaska_7 (a, b);
}
static inline vector unsigned char
spu_rlmaskqw (vector unsigned char a, int b)
{
  return __builtin_spu_rlmaskqw_0 (a, b);
}
static inline vector signed char
spu_rlmaskqw (vector signed char a, int b)
{
  return __builtin_spu_rlmaskqw_1 (a, b);
}
static inline vector unsigned short
spu_rlmaskqw (vector unsigned short a, int b)
{
  return __builtin_spu_rlmaskqw_2 (a, b);
}
static inline vector signed short
spu_rlmaskqw (vector signed short a, int b)
{
  return __builtin_spu_rlmaskqw_3 (a, b);
}
static inline vector unsigned int
spu_rlmaskqw (vector unsigned int a, int b)
{
  return __builtin_spu_rlmaskqw_4 (a, b);
}
static inline vector signed int
spu_rlmaskqw (vector signed int a, int b)
{
  return __builtin_spu_rlmaskqw_5 (a, b);
}
static inline vector unsigned long long
spu_rlmaskqw (vector unsigned long long a, int b)
{
  return __builtin_spu_rlmaskqw_6 (a, b);
}
static inline vector signed long long
spu_rlmaskqw (vector signed long long a, int b)
{
  return __builtin_spu_rlmaskqw_7 (a, b);
}
static inline vector float
spu_rlmaskqw (vector float a, int b)
{
  return __builtin_spu_rlmaskqw_8 (a, b);
}
static inline vector double
spu_rlmaskqw (vector double a, int b)
{
  return __builtin_spu_rlmaskqw_9 (a, b);
}
static inline vector unsigned char
spu_rlmaskqwbyte (vector unsigned char a, int b)
{
  return __builtin_spu_rlmaskqwbyte_0 (a, b);
}
static inline vector signed char
spu_rlmaskqwbyte (vector signed char a, int b)
{
  return __builtin_spu_rlmaskqwbyte_1 (a, b);
}
static inline vector unsigned short
spu_rlmaskqwbyte (vector unsigned short a, int b)
{
  return __builtin_spu_rlmaskqwbyte_2 (a, b);
}
static inline vector signed short
spu_rlmaskqwbyte (vector signed short a, int b)
{
  return __builtin_spu_rlmaskqwbyte_3 (a, b);
}
static inline vector unsigned int
spu_rlmaskqwbyte (vector unsigned int a, int b)
{
  return __builtin_spu_rlmaskqwbyte_4 (a, b);
}
static inline vector signed int
spu_rlmaskqwbyte (vector signed int a, int b)
{
  return __builtin_spu_rlmaskqwbyte_5 (a, b);
}
static inline vector unsigned long long
spu_rlmaskqwbyte (vector unsigned long long a, int b)
{
  return __builtin_spu_rlmaskqwbyte_6 (a, b);
}
static inline vector signed long long
spu_rlmaskqwbyte (vector signed long long a, int b)
{
  return __builtin_spu_rlmaskqwbyte_7 (a, b);
}
static inline vector float
spu_rlmaskqwbyte (vector float a, int b)
{
  return __builtin_spu_rlmaskqwbyte_8 (a, b);
}
static inline vector double
spu_rlmaskqwbyte (vector double a, int b)
{
  return __builtin_spu_rlmaskqwbyte_9 (a, b);
}
static inline vector unsigned char
spu_rlmaskqwbytebc (vector unsigned char a, int b)
{
  return __builtin_spu_rlmaskqwbytebc_0 (a, b);
}
static inline vector signed char
spu_rlmaskqwbytebc (vector signed char a, int b)
{
  return __builtin_spu_rlmaskqwbytebc_1 (a, b);
}
static inline vector unsigned short
spu_rlmaskqwbytebc (vector unsigned short a, int b)
{
  return __builtin_spu_rlmaskqwbytebc_2 (a, b);
}
static inline vector signed short
spu_rlmaskqwbytebc (vector signed short a, int b)
{
  return __builtin_spu_rlmaskqwbytebc_3 (a, b);
}
static inline vector unsigned int
spu_rlmaskqwbytebc (vector unsigned int a, int b)
{
  return __builtin_spu_rlmaskqwbytebc_4 (a, b);
}
static inline vector signed int
spu_rlmaskqwbytebc (vector signed int a, int b)
{
  return __builtin_spu_rlmaskqwbytebc_5 (a, b);
}
static inline vector unsigned long long
spu_rlmaskqwbytebc (vector unsigned long long a, int b)
{
  return __builtin_spu_rlmaskqwbytebc_6 (a, b);
}
static inline vector signed long long
spu_rlmaskqwbytebc (vector signed long long a, int b)
{
  return __builtin_spu_rlmaskqwbytebc_7 (a, b);
}
static inline vector float
spu_rlmaskqwbytebc (vector float a, int b)
{
  return __builtin_spu_rlmaskqwbytebc_8 (a, b);
}
static inline vector double
spu_rlmaskqwbytebc (vector double a, int b)
{
  return __builtin_spu_rlmaskqwbytebc_9 (a, b);
}
static inline vector unsigned short
spu_sl (vector unsigned short a, vector unsigned short b)
{
  return __builtin_spu_sl_0 (a, b);
}
static inline vector signed short
spu_sl (vector signed short a, vector unsigned short b)
{
  return __builtin_spu_sl_1 (a, b);
}
static inline vector unsigned int
spu_sl (vector unsigned int a, vector unsigned int b)
{
  return __builtin_spu_sl_2 (a, b);
}
static inline vector signed int
spu_sl (vector signed int a, vector unsigned int b)
{
  return __builtin_spu_sl_3 (a, b);
}
static inline vector unsigned short
spu_sl (vector unsigned short a, unsigned int b)
{
  return __builtin_spu_sl_4 (a, b);
}
static inline vector signed short
spu_sl (vector signed short a, unsigned int b)
{
  return __builtin_spu_sl_5 (a, b);
}
static inline vector unsigned int
spu_sl (vector unsigned int a, unsigned int b)
{
  return __builtin_spu_sl_6 (a, b);
}
static inline vector signed int
spu_sl (vector signed int a, unsigned int b)
{
  return __builtin_spu_sl_7 (a, b);
}
static inline vector signed long long
spu_slqw (vector signed long long a, unsigned int b)
{
  return __builtin_spu_slqw_0 (a, b);
}
static inline vector unsigned long long
spu_slqw (vector unsigned long long a, unsigned int b)
{
  return __builtin_spu_slqw_1 (a, b);
}
static inline vector signed int
spu_slqw (vector signed int a, unsigned int b)
{
  return __builtin_spu_slqw_2 (a, b);
}
static inline vector unsigned int
spu_slqw (vector unsigned int a, unsigned int b)
{
  return __builtin_spu_slqw_3 (a, b);
}
static inline vector signed short
spu_slqw (vector signed short a, unsigned int b)
{
  return __builtin_spu_slqw_4 (a, b);
}
static inline vector unsigned short
spu_slqw (vector unsigned short a, unsigned int b)
{
  return __builtin_spu_slqw_5 (a, b);
}
static inline vector signed char
spu_slqw (vector signed char a, unsigned int b)
{
  return __builtin_spu_slqw_6 (a, b);
}
static inline vector unsigned char
spu_slqw (vector unsigned char a, unsigned int b)
{
  return __builtin_spu_slqw_7 (a, b);
}
static inline vector float
spu_slqw (vector float a, unsigned int b)
{
  return __builtin_spu_slqw_8 (a, b);
}
static inline vector double
spu_slqw (vector double a, unsigned int b)
{
  return __builtin_spu_slqw_9 (a, b);
}
static inline vector signed long long
spu_slqwbyte (vector signed long long a, unsigned int b)
{
  return __builtin_spu_slqwbyte_0 (a, b);
}
static inline vector unsigned long long
spu_slqwbyte (vector unsigned long long a, unsigned int b)
{
  return __builtin_spu_slqwbyte_1 (a, b);
}
static inline vector signed int
spu_slqwbyte (vector signed int a, unsigned int b)
{
  return __builtin_spu_slqwbyte_2 (a, b);
}
static inline vector unsigned int
spu_slqwbyte (vector unsigned int a, unsigned int b)
{
  return __builtin_spu_slqwbyte_3 (a, b);
}
static inline vector signed short
spu_slqwbyte (vector signed short a, unsigned int b)
{
  return __builtin_spu_slqwbyte_4 (a, b);
}
static inline vector unsigned short
spu_slqwbyte (vector unsigned short a, unsigned int b)
{
  return __builtin_spu_slqwbyte_5 (a, b);
}
static inline vector signed char
spu_slqwbyte (vector signed char a, unsigned int b)
{
  return __builtin_spu_slqwbyte_6 (a, b);
}
static inline vector unsigned char
spu_slqwbyte (vector unsigned char a, unsigned int b)
{
  return __builtin_spu_slqwbyte_7 (a, b);
}
static inline vector float
spu_slqwbyte (vector float a, unsigned int b)
{
  return __builtin_spu_slqwbyte_8 (a, b);
}
static inline vector double
spu_slqwbyte (vector double a, unsigned int b)
{
  return __builtin_spu_slqwbyte_9 (a, b);
}
static inline vector signed long long
spu_slqwbytebc (vector signed long long a, unsigned int b)
{
  return __builtin_spu_slqwbytebc_0 (a, b);
}
static inline vector unsigned long long
spu_slqwbytebc (vector unsigned long long a, unsigned int b)
{
  return __builtin_spu_slqwbytebc_1 (a, b);
}
static inline vector signed int
spu_slqwbytebc (vector signed int a, unsigned int b)
{
  return __builtin_spu_slqwbytebc_2 (a, b);
}
static inline vector unsigned int
spu_slqwbytebc (vector unsigned int a, unsigned int b)
{
  return __builtin_spu_slqwbytebc_3 (a, b);
}
static inline vector signed short
spu_slqwbytebc (vector signed short a, unsigned int b)
{
  return __builtin_spu_slqwbytebc_4 (a, b);
}
static inline vector unsigned short
spu_slqwbytebc (vector unsigned short a, unsigned int b)
{
  return __builtin_spu_slqwbytebc_5 (a, b);
}
static inline vector signed char
spu_slqwbytebc (vector signed char a, unsigned int b)
{
  return __builtin_spu_slqwbytebc_6 (a, b);
}
static inline vector unsigned char
spu_slqwbytebc (vector unsigned char a, unsigned int b)
{
  return __builtin_spu_slqwbytebc_7 (a, b);
}
static inline vector float
spu_slqwbytebc (vector float a, unsigned int b)
{
  return __builtin_spu_slqwbytebc_8 (a, b);
}
static inline vector double
spu_slqwbytebc (vector double a, unsigned int b)
{
  return __builtin_spu_slqwbytebc_9 (a, b);
}
static inline vector unsigned char
spu_splats (unsigned char a)
{
  return __builtin_spu_splats_0 (a);
}
static inline vector signed char
spu_splats (signed char a)
{
  return __builtin_spu_splats_1 (a);
}
static inline vector signed char
spu_splats (char a)
{
  return __builtin_spu_splats_1 (a);
}
static inline vector unsigned short
spu_splats (unsigned short a)
{
  return __builtin_spu_splats_2 (a);
}
static inline vector signed short
spu_splats (short a)
{
  return __builtin_spu_splats_3 (a);
}
static inline vector unsigned int
spu_splats (unsigned int a)
{
  return __builtin_spu_splats_4 (a);
}
static inline vector signed int
spu_splats (int a)
{
  return __builtin_spu_splats_5 (a);
}
static inline vector unsigned long long
spu_splats (unsigned long long a)
{
  return __builtin_spu_splats_6 (a);
}
static inline vector signed long long
spu_splats (long long a)
{
  return __builtin_spu_splats_7 (a);
}
static inline vector float
spu_splats (float a)
{
  return __builtin_spu_splats_8 (a);
}
static inline vector double
spu_splats (double a)
{
  return __builtin_spu_splats_9 (a);
}
static inline unsigned char
spu_extract (vector unsigned char a, int b)
{
  return __builtin_spu_extract_0 (a, b);
}
static inline signed char
spu_extract (vector signed char a, int b)
{
  return __builtin_spu_extract_1 (a, b);
}
static inline unsigned short
spu_extract (vector unsigned short a, int b)
{
  return __builtin_spu_extract_2 (a, b);
}
static inline short
spu_extract (vector signed short a, int b)
{
  return __builtin_spu_extract_3 (a, b);
}
static inline unsigned int
spu_extract (vector unsigned int a, int b)
{
  return __builtin_spu_extract_4 (a, b);
}
static inline int
spu_extract (vector signed int a, int b)
{
  return __builtin_spu_extract_5 (a, b);
}
static inline unsigned long long
spu_extract (vector unsigned long long a, int b)
{
  return __builtin_spu_extract_6 (a, b);
}
static inline long long
spu_extract (vector signed long long a, int b)
{
  return __builtin_spu_extract_7 (a, b);
}
static inline float
spu_extract (vector float a, int b)
{
  return __builtin_spu_extract_8 (a, b);
}
static inline double
spu_extract (vector double a, int b)
{
  return __builtin_spu_extract_9 (a, b);
}
static inline vector unsigned char
spu_insert (unsigned char a, vector unsigned char b, int c)
{
  return __builtin_spu_insert_0 (a, b, c);
}
static inline vector signed char
spu_insert (signed char a, vector signed char b, int c)
{
  return __builtin_spu_insert_1 (a, b, c);
}
static inline vector unsigned short
spu_insert (unsigned short a, vector unsigned short b, int c)
{
  return __builtin_spu_insert_2 (a, b, c);
}
static inline vector signed short
spu_insert (short a, vector signed short b, int c)
{
  return __builtin_spu_insert_3 (a, b, c);
}
static inline vector unsigned int
spu_insert (unsigned int a, vector unsigned int b, int c)
{
  return __builtin_spu_insert_4 (a, b, c);
}
static inline vector signed int
spu_insert (int a, vector signed int b, int c)
{
  return __builtin_spu_insert_5 (a, b, c);
}
static inline vector unsigned long long
spu_insert (unsigned long long a, vector unsigned long long b, int c)
{
  return __builtin_spu_insert_6 (a, b, c);
}
static inline vector signed long long
spu_insert (long long a, vector signed long long b, int c)
{
  return __builtin_spu_insert_7 (a, b, c);
}
static inline vector float
spu_insert (float a, vector float b, int c)
{
  return __builtin_spu_insert_8 (a, b, c);
}
static inline vector double
spu_insert (double a, vector double b, int c)
{
  return __builtin_spu_insert_9 (a, b, c);
}
static inline vector unsigned char
spu_promote (unsigned char a, int b)
{
  return __builtin_spu_promote_0 (a, b);
}
static inline vector signed char
spu_promote (signed char a, int b)
{
  return __builtin_spu_promote_1 (a, b);
}
static inline vector signed char
spu_promote (char a, int b)
{
  return __builtin_spu_promote_1 (a, b);
}
static inline vector unsigned short
spu_promote (unsigned short a, int b)
{
  return __builtin_spu_promote_2 (a, b);
}
static inline vector signed short
spu_promote (short a, int b)
{
  return __builtin_spu_promote_3 (a, b);
}
static inline vector unsigned int
spu_promote (unsigned int a, int b)
{
  return __builtin_spu_promote_4 (a, b);
}
static inline vector signed int
spu_promote (int a, int b)
{
  return __builtin_spu_promote_5 (a, b);
}
static inline vector unsigned long long
spu_promote (unsigned long long a, int b)
{
  return __builtin_spu_promote_6 (a, b);
}
static inline vector signed long long
spu_promote (long long a, int b)
{
  return __builtin_spu_promote_7 (a, b);
}
static inline vector float
spu_promote (float a, int b)
{
  return __builtin_spu_promote_8 (a, b);
}
static inline vector double
spu_promote (double a, int b)
{
  return __builtin_spu_promote_9 (a, b);
}



extern "C" {
# 2865 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_internals.h" 3 4
static __inline__ vector float spu_re (vector float ra) __attribute__((__always_inline__));
static __inline__ vector float spu_rsqrte (vector float ra) __attribute__((__always_inline__));

static __inline__ vector float
spu_re (vector float ra)
{
  return (vector float) __builtin_si_fi((qword) (ra),__builtin_si_frest((qword) (ra)));
}
static __inline__ vector float
spu_rsqrte (vector float ra)
{
  return (vector float) __builtin_si_fi((qword) (ra),__builtin_si_frsqest((qword) (ra)));
}


static __inline__ void spu_mfcdma32(volatile void *ls, unsigned int ea, unsigned int size, unsigned int tagid, unsigned int cmd) __attribute__((__always_inline__));
static __inline__ void spu_mfcdma64(volatile void *ls, unsigned int eahi, unsigned int ealow, unsigned int size, unsigned int tagid, unsigned int cmd) __attribute__((__always_inline__));
static __inline__ unsigned int spu_mfcstat(unsigned int type) __attribute__((__always_inline__));

static __inline__ void
spu_mfcdma32(volatile void *ls, unsigned int ea, unsigned int size, unsigned int tagid, unsigned int cmd)
{
      __builtin_si_wrch(16,__builtin_si_from_ptr(ls));
      __builtin_si_wrch(18,__builtin_si_from_uint(ea));
      __builtin_si_wrch(19,__builtin_si_from_uint(size));
      __builtin_si_wrch(20,__builtin_si_from_uint(tagid));
      __builtin_si_wrch(21,__builtin_si_from_uint(cmd));
}
static __inline__ void
spu_mfcdma64(volatile void *ls, unsigned int eahi, unsigned int ealow, unsigned int size, unsigned int tagid, unsigned int cmd)
{
      __builtin_si_wrch(16,__builtin_si_from_ptr(ls));
      __builtin_si_wrch(17,__builtin_si_from_uint(eahi));
      __builtin_si_wrch(18,__builtin_si_from_uint(ealow));
      __builtin_si_wrch(19,__builtin_si_from_uint(size));
      __builtin_si_wrch(20,__builtin_si_from_uint(tagid));
      __builtin_si_wrch(21,__builtin_si_from_uint(cmd));
}
static __inline__ unsigned int
spu_mfcstat(unsigned int type)
{
      __builtin_si_wrch(23,__builtin_si_from_uint(type));
      return __builtin_si_to_uint(__builtin_si_rdch(24));
}


}
# 80 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_intrinsics.h" 2 3 4
# 33 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_mfcio.h" 2 3 4
# 1 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/stdint.h" 1 3 4
# 10 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/stdint.h" 3 4
# 1 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 1 3 4
# 9 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
# 1 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/sys/types.h" 1 3 4
# 9 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/sys/types.h" 3 4
# 1 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/sys/cdefs.h" 1 3 4
# 10 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/sys/types.h" 2 3 4
# 1 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/sys/integertypes.h" 1 3 4
# 11 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/sys/integertypes.h" 3 4
namespace std { typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed int intptr_t;
typedef unsigned int uintptr_t;

}

 using ::std:: int8_t;
using ::std:: uint8_t;
using ::std:: int16_t;
using ::std:: uint16_t;
using ::std:: int32_t;
using ::std:: uint32_t;
using ::std:: intptr_t;
using ::std:: uintptr_t;
# 11 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/sys/types.h" 2 3 4




namespace std { typedef unsigned long size_t;

}

 using ::std:: size_t;
# 10 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 2 3 4
# 1 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/stdarg.h" 1 3 4
# 11 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/stdarg.h" 3 4
typedef __builtin_va_list va_list ;
# 11 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 2 3 4
# 259 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
typedef int _Int32t;
typedef unsigned int _Uint32t;







typedef long _Ptrdifft;






typedef unsigned long _Sizet;
# 395 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
namespace std {}
# 563 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
namespace std {
typedef bool _Bool;
}
# 629 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
namespace std {
# 657 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
typedef long long _Longlong;
typedef unsigned long long _ULonglong;
# 680 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
typedef wchar_t _Wchart;
typedef wchar_t _Wintt;
# 751 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
typedef va_list _Va_list;
# 771 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
extern "C" {
void _Atexit(void (*)(void));
}
# 785 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
typedef char _Sysch_t;







}
# 824 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
namespace std {
extern "C++" {

class _Lockit
 {
public:




 explicit _Lockit()
  {
  }

 explicit _Lockit(int)
  {
  }

 ~_Lockit()
  {
  }
# 890 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
 };

class _Mutex
 {
public:


    void _Lock()
  {
  }

 void _Unlock()
  {
 }
# 917 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
 };
}
}
# 945 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/yvals.h" 3 4
namespace std {
using ::va_list;
}
# 11 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/stdint.h" 2 3 4
# 26 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/stdint.h" 3 4
namespace std {






typedef signed char int_least8_t;
typedef short int_least16_t;
typedef _Int32t int_least32_t;
typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef _Uint32t uint_least32_t;


typedef signed char int_fast8_t;
typedef short int_fast16_t;
typedef _Int32t int_fast32_t;
typedef unsigned char uint_fast8_t;
typedef unsigned short uint_fast16_t;
typedef _Uint32t uint_fast32_t;
# 57 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/stdint.h" 3 4
typedef _Longlong int64_t;



typedef _ULonglong uint64_t;

typedef _Longlong int_least64_t;
typedef _ULonglong uint_least64_t;


typedef _Longlong int_fast64_t;
typedef _ULonglong uint_fast64_t;



typedef _Longlong intmax_t;
typedef _ULonglong uintmax_t;
# 191 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/stdint.h" 3 4
}
# 208 "c:/usr/local/cell/host-win32/spu/bin/../../../target/spu/include/stdint.h" 3 4
using ::std:: int64_t;



using ::std:: uint64_t;

using ::std:: int_least8_t; using ::std:: int_least16_t;
using ::std:: int_least32_t; using ::std:: int_least64_t;
using ::std:: uint_least8_t; using ::std:: uint_least16_t;
using ::std:: uint_least32_t; using ::std:: uint_least64_t;

using ::std:: intmax_t; using ::std:: uintmax_t;


using ::std:: int_fast8_t; using ::std:: int_fast16_t;
using ::std:: int_fast32_t; using ::std:: int_fast64_t;
using ::std:: uint_fast8_t; using ::std:: uint_fast16_t;
using ::std:: uint_fast32_t; using ::std:: uint_fast64_t;
# 34 "c:/usr/local/cell/host-win32/spu/bin/../lib/gcc/spu-lv2/3.4.1/include/spu_mfcio.h" 2 3 4







__extension__

typedef struct mfc_list_element {
  uint64_t notify : 1;
  uint64_t reserved : 15;
  uint64_t size : 16;
  uint64_t eal : 32;
} mfc_list_element_t;
# 25 "ntlib_spu/ntDMA.h" 2
# 51 "ntlib_spu/ntDMA.h"
namespace ntDMA { struct Params; namespace Debug { inline void Check( const Params & ); } }
# 60 "ntlib_spu/ntDMA.h"
typedef uint32_t ntDMA_ID;
# 123 "ntlib_spu/ntDMA.h"
namespace ntDMA
{







 struct Params
 {






  Params()
  : m_LSAddr ( NULL )
  , m_EAlong ( 0 )
  , m_TransferSize ( 0 )
  , m_ID ( InvalidDMAParams )
  {}






  void Init64( void *local_store_addr, int64_t effective_addr, uint32_t length_in_bytes, ntDMA_ID dma_id )
  {
   m_LSAddr = static_cast< int8_t * >( local_store_addr );
   m_EAlong = effective_addr;
   m_TransferSize = length_in_bytes;
   m_ID = dma_id;

   ntError_p( m_ID >= 0 && m_ID < 32, ("[DMA] DMA ids must be between 0 and 31 inclusive, you have requested an invalid id of %i\n", m_ID) );
   if ( m_ID < 0 || m_ID >= 32 )
   {
    m_ID = InvalidDMAParams;
   }

   ntDMA::Debug::Check( *this );
  }






  void Init32( void *local_store_addr, int32_t effective_addr, uint32_t length_in_bytes, ntDMA_ID dma_id )
  {
   m_LSAddr = static_cast< int8_t * >( local_store_addr );
   m_TransferSize = length_in_bytes;
   m_ID = dma_id;

   m_EA[ 0 ] = 0;
   m_EA[ 1 ] = effective_addr;

   ntError_p( m_ID >= 0 && m_ID < 32, ("[DMA] DMA ids must be between 0 and 31 inclusive, you have requested an invalid id of %i\n", m_ID) );
   if ( m_ID < 0 || m_ID >= 32 )
   {
    m_ID = InvalidDMAParams;
   }

   ntDMA::Debug::Check( *this );
  }


  int8_t * m_LSAddr;


  union
  {
   uint64_t m_EAlong;
   uint32_t m_EA[ 2 ];
  };


  uint32_t m_TransferSize;



  ntDMA_ID m_ID;


  static const ntDMA_ID InvalidDMAParams = ntDMA_ID( -1 );
 };



 inline void DmaToSPU( Params params );
 inline void DmaToPPU( Params params );
# 227 "ntlib_spu/ntDMA.h"
 inline void StoreToEaThenFetchFromEa( const Params &params, int64_t ea );
 inline void StoreToEaThenFetchFromEa( const Params &params, int32_t ea );



 inline bool HasCompleted( ntDMA_ID dma_id );


 inline void StallForCompletion( ntDMA_ID dma_id );


 inline void Synchronise();


 inline int32_t NumberOfAvailableDMAQueueSlots();


 inline ntDMA_ID GetFreshID();

 static const uint32_t MaxSingleDMATransferSize = 0x4000;
}
# 265 "ntlib_spu/ntDMA.h"
void ntDMA::DmaToSPU( ntDMA::Params params )
{
 ntDMA::Debug::Check( params );


  ntError_p( NumberOfAvailableDMAQueueSlots() > 0, ("[DMA] MFC queue is full - about to stall requesting a DMA transfer.") );


 do
 {
  uint32_t size = params.m_TransferSize < ntDMA::MaxSingleDMATransferSize ? params.m_TransferSize : ntDMA::MaxSingleDMATransferSize;

  asm __volatile__("" : : : "memory");
  spu_mfcdma64( params.m_LSAddr, params.m_EA[ 0 ] , params.m_EA[ 1 ] ,
      size, params.m_ID, 0x0040 );
  asm __volatile__("" : : : "memory");

  params.m_TransferSize -= size;
  params.m_EA[ 1 ] += size;
  params.m_LSAddr += size;
 }
 while ( params.m_TransferSize > 0 );

 do {} while( 0 );
}

void ntDMA::DmaToPPU( ntDMA::Params params )
{
 ntDMA::Debug::Check( params );


  ntError_p( NumberOfAvailableDMAQueueSlots() > 0, ("[DMA] MFC queue is full - about to stall requesting a DMA transfer.") );


 do
 {
  uint32_t size = params.m_TransferSize < ntDMA::MaxSingleDMATransferSize ? params.m_TransferSize : ntDMA::MaxSingleDMATransferSize;

  asm __volatile__("" : : : "memory");
  spu_mfcdma64( static_cast< volatile void * >( params.m_LSAddr ), params.m_EA[ 0 ] ,
      params.m_EA[ 1 ] , params.m_TransferSize, params.m_ID, 0x0020 );
  asm __volatile__("" : : : "memory");

  params.m_TransferSize -= size;
  params.m_EA[ 1 ] += size;
  params.m_LSAddr += size;
 }
 while ( params.m_TransferSize > 0 );

 do {} while( 0 );
}

bool ntDMA::HasCompleted( ntDMA_ID dma_id )
{
 ntError_p( dma_id >= 0 && dma_id < 32, ("[DMA] Invalid DMA id detected. ID = %i", dma_id) );
 __builtin_si_wrch((22),__builtin_si_from_uint(1 << dma_id));

 asm __volatile__("" : : : "memory");
 uint32_t status = spu_mfcstat( 0 );
 asm __volatile__("" : : : "memory");

 return status != 0;
}

void ntDMA::StallForCompletion( ntDMA_ID dma_id )
{
 ntError_p( dma_id >= 0 && dma_id < 32, ("[DMA] Invalid DMA id detected. ID = %i", dma_id) );
 __builtin_si_wrch((22),__builtin_si_from_uint(1 << dma_id));

 asm __volatile__("" : : : "memory");
 spu_mfcstat( 2 );
 asm __volatile__("" : : : "memory");
}

void ntDMA::Synchronise()
{
 __builtin_si_wrch((22),__builtin_si_from_uint(0xffffffff));

 asm __volatile__("" : : : "memory");
 spu_mfcstat( 2 );
 asm __volatile__("" : : : "memory");
}

int32_t ntDMA::NumberOfAvailableDMAQueueSlots()
{
 return __builtin_si_to_uint(__builtin_si_rchcnt((21)));
}

ntDMA_ID ntDMA::GetFreshID()
{
 static ntDMA_ID sCurrentID = 0;
 return sCurrentID > 31 ? sCurrentID = 0, 0 : sCurrentID++;
}

void ntDMA::StoreToEaThenFetchFromEa( const ntDMA::Params &params, int32_t ea )
{
 union PtrConv_
 {
  int64_t ea64;
  int32_t ea32[ 2 ];
 };
 PtrConv_ ptr;
 ptr.ea32[ 0 ] = 0;
 ptr.ea32[ 1 ] = ea;

 StoreToEaThenFetchFromEa( params, ptr.ea64 );
}

void ntDMA::StoreToEaThenFetchFromEa( const ntDMA::Params &params, int64_t ea )
{
 ntDMA::Debug::Check( params );
 ntError_p( params.m_TransferSize <= MaxSingleDMATransferSize, ("Single DMAs are limited to transfers of less than 16KB. m_TransferSize=%i", params.m_TransferSize) );
 ntError_p( ( ea % 0xf ) == 0, ("[DMA] Effective address not aligned to 16-byte boundary. EA = 0x%X", ea) );


  ntError_p( NumberOfAvailableDMAQueueSlots() > 1, ("[DMA] MFC queue doesn't have enough slots free - about to stall requesting a DMA transfer.") );


 asm __volatile__("" : : : "memory");
 spu_mfcdma64( static_cast< volatile void * >( params.m_LSAddr ), params.m_EA[ 0 ] ,
     params.m_EA[ 1 ] , params.m_TransferSize, params.m_ID, 0x0020 );
 asm __volatile__("" : : : "memory");

 asm __volatile__("" : : : "memory");
 spu_mfcdma64( params.m_LSAddr, ea >> 32 , (int32_t)ea ,
     params.m_TransferSize, params.m_ID, 0x0042 );
 asm __volatile__("" : : : "memory");

 do {} while( 0 );
}

inline void ntDMA::Debug::Check( const ntDMA::Params &params )
{
 ntError_p( params.m_ID >= 0 && params.m_ID < 32, ("[DMA] Invalid DMA id detected. ID = %i", params.m_ID) );
 ntError_p( ( reinterpret_cast< intptr_t >( params.m_LSAddr ) & 0xf ) == 0, ("[DMA] Local store address not aligned to 16-byte boundary. LS = 0x%X", params.m_LSAddr) );
 ntError_p( ( params.m_EAlong & 0xf ) == 0, ("[DMA] Effective address not aligned to 16-byte boundary. EA = 0x%X", params.m_EA[ 1 ]) );
 ntError_p( ( params.m_TransferSize & 0xf ) == 0, ("[DMA] Transfer size is not a multiple of 16-bytes. Size = %i", params.m_TransferSize) );
 ntError_p( params.m_TransferSize > 0, ("[DMA] Transfer size must be greater than zero. Size = %i", params.m_TransferSize) );
}
# 15 "ntlib_spu/ntDMA.cpp" 2
