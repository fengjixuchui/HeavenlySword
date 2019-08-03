//--------------------------------------------------------------------------------------------------
/**
	@file		Fw.h
	
	@brief		Core Framework Includes

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_H
#define	FW_H

// Include version information
#include <Fw/FwVersion.h>

#ifdef __cplusplus	// this is a forced include, but we don't want it for C programs
					// or they fail to compile quite horrendously

// Include Framework switches
#include <Fw/FwSwitches.h>

// Include System headers
#include <stddef.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <float.h>
#include <stdarg.h>
#include <string.h>
#include <altivec.h>  
#include <sdk_version.h>

// endian-ness handling
#define	FW_BIG_ENDIAN 1
#define FW_LITTLE_ENDIAN (!FW_BIG_ENDIAN) 

// Now include our other common include files
#include <Fw/FwTypes.h>
#include <Fw/FwPlatform.h>
#include <Fw/FwBasePrintf.h>
#include <Fw/FwBaseAssert.h>
#include <Fw/FwInternalAssert.h>
#include <Fw/FwHelpers.h>
#include <Fw/FwEndian.h>
#include <Fw/FwMem/FwMem.h>
#include <Fw/FwSingleton.h>

#endif	// __cplusplus
#endif	// FW_H
