/***************************************************************************************************
*
*	$Header:: /game/material.h 3     13/08/03 10:39 Simonb                                         $
*
*	The material and technique class, and the material instance class.
*
*	CHANGES
*
*	2/7/2003	SimonB	Created
*
***************************************************************************************************/

#ifndef _MATERIAL_H
#define _MATERIAL_H

#if defined( PLATFORM_PC )
#	include "gfx/material_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "gfx/material_ps3.h"
#endif

#endif // ndef _MATERIAL_H
