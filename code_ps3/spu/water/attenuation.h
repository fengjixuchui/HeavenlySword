//--------------------------------------------------
//!
//!	\file attenuation.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef __SPU__
#	error this file is for spu projects only 
#endif

#ifndef _ATTENUATION_H_
#define _ATTENUATION_H_


#include "water_utils.h"

#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/vecmath_spu_soa_ps3.h"


v128 DistanceAttenuation( const CPoint_SOA& relative_position, float att_linear, float att_quadratic );

v128 AgeAttenuation( float age, float lifespan, float fade_in, float fade_out );

v128 WidthAttenuation( const CPoint_SOA& relative_position, const CPoint_SOA& direction, float width, float falloff );

v128 RingAttenuation( const CPoint_SOA& relative_position, const CPoint_SOA& direction, float radius, float width );

v128 TrailAttenuation( const CPoint_SOA& relative_position, const CPoint_SOA& direction, float back_trail, float front_trail );

#endif // end of _ATTENUATION_H_

