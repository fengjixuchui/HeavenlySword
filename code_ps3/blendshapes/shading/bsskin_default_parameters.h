//--------------------------------------------------
//!
//!	\file bsskin_default_parameters.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _BSSKIN_DEFAULT_PARAMETERS_H_
#define _BSSKIN_DEFAULT_PARAMETERS_H_

#include "gfx/shader.h"

CVector GetBSSkinDefaultPropertyValue( const PROPERTY_SEMANTIC_TYPE type )
{
	switch( type )
	{
	case PROPERTY_BSSKIN_DIFFUSE_WRAP:
		return CVector( 0.8f, 0.0f, 0.0f, 1.0f );
	case PROPERTY_BSSKIN_SPECULAR_FACING:
		return CVector( 11.0f, 0.0f, 0.0f, 1.0f );
	case PROPERTY_BSSKIN_SPECULAR_GLANCING:
		return CVector( 8.0f, 0.0f, 0.0f, 1.0f );
	case PROPERTY_BSSKIN_FUZZ_COLOUR:
		return CVector( 0.8f, 0.8f, 0.8f, 1.0f );
	case PROPERTY_BSSKIN_FUZZ_TIGHTNESS:
		return CVector( 2.4f, 0.0f, 0.0f, 1.0f );
	case PROPERTY_BSSKIN_SUBCOLOUR:
		return CVector( 0.8f, 0.0f, 0.0f, 1.0f );
	case PROPERTY_BSSKIN_WRINKLE_REGIONS0_WEIGHTS:
	case PROPERTY_BSSKIN_WRINKLE_REGIONS1_WEIGHTS:
		return CVector(CONSTRUCT_CLEAR);
	default:
		return CVector(CONSTRUCT_CLEAR);
	}
}

#endif // end of _BSSKIN_DEFAULT_PARAMETERS_H_
