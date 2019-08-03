/***************************************************************************************************
*
*	$Header:: /game/vecmath.h 25    21/07/03 16:37 Dean                                            $
*
*	Vector Maths 
*
*	You'll notice that inlines come *after* the classes. This makes things a lot clearer.
*
*	NOTES
*
*	You'll see that most of the methods available return vectors (and matrices) by value. This means 
*	that memory reads & writes can be eliminated by the compiler as it sees fit. Common operations on
*	vectors tend to just use a few SSE registers, with a final write to memory - even when stringing
*	together multiple operations.
*
*	CHANGES
*
*	18/11/2002	Dean	Created
*
***************************************************************************************************/

#ifndef	_VECMATH_H
#define	_VECMATH_H

#if defined( PLATFORM_PC )
#	include "core/vecmath_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "core/vecmath_ps3.h"
#endif


#endif	//_VECMATH_H
