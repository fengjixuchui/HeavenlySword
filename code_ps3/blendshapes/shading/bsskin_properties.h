//--------------------------------------------------
//!
//!	\file bsskin_properties.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _BSSKIN_PROPERTIES_H_
#define _BSSKIN_PROPERTIES_H_


#include "tbd/xmlfileinterface.h"


struct BSSkinProperties
{
	HAS_INTERFACE(BSSkinProperties)

	float m_diffuseWrap;
	float m_specularFacing;
	float m_specularGlancing;
	CVector m_fuzzColour;
	float	m_fuzzTightness;
	CVector m_subColour;
	float	m_normalMapStrength;
	float	m_specularStrength;
};


#endif // end of _BSSKIN_PROPERTIES_H_
