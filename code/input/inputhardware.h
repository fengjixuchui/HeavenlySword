/***************************************************************************************************
*
*   $Header:: /game/inputhardware.h 4     14/08/03 10:48 Dean                                      $
*
*	Header file for CInputHardware class
*
*	CHANGES		
*
*	10/01/2001	Dean	Created
*
***************************************************************************************************/

#ifndef	_INPUTHARDWARE_H
#define	_INPUTHARDWARE_H

// Different play modes require different types of dead-zone handling
enum PAD_DEADZONE_MODE
{
	// No dead zone handling. 
	DZ_NONE,

	// Standard ninja dead zone handling
	DZ_STD,
};


#if defined( PLATFORM_PC )
#	include "input/inputhardware_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "input/inputhardware_ps3.h"
#endif

#endif	//_INPUTHARDWARE_H

