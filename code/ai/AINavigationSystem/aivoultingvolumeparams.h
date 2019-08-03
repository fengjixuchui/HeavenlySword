//! -------------------------------------------
//! aivoultingvolumeparams.h
//!
//! Add/Remove here the parameters needed for
//! voulting volumes
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIVOULTINGVOLUMEPARAMS_H
#define _AIVOULTINGVOLUMEPARAMS_H

typedef struct _SVaultingParams
{
	_SVaultingParams() : fVaultingDistance(1.0f) {}

	float fVaultingDistance;
} SVaultingParams;

#endif // _AIVOULTINGVOLUMEPARAMS_H
