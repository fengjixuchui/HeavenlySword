//--------------------------------------------------
//!
//!	\file stam_solver.cpp
//!	Jos Stam's stable fluids solver. 
//!
//--------------------------------------------------
#include "waves.h"


#include <basetypes_spu.h>
#include <debug_spu.h>
#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/vecmath_spu_soa_ps3.h"
#include "ntlib_spu/ntDma.h"



#define VECTORMATH_SHUF_XAZW (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_A, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_W }
#define VECTORMATH_SHUF_XBZW (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_B, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_W }
#define VECTORMATH_SHUF_XCZW (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_C, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_W }
#define VECTORMATH_SHUF_XDZW (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_D, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_W }



// empty


