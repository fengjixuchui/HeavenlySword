/***************************************************************************************************
*
*	$Header:: /game/effects.h 3     13/08/03 10:39 Simonb                                          $
*
*	Includes the PC effects if we're on the PC.
*
*	CHANGES
*
*	22/5/2003	SimonB	Created
*
***************************************************************************************************/

#ifndef _EFFECT_SKY_H
#define _EFFECT_SKY_H

#if defined( PLATFORM_PC )
#	include "effect/effect_sky_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "effect/effect_sky_ps3.h"
#endif

#endif // ndef _EFFECT_SKY_H
