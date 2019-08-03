#include "blendshapes/shading/bsskin_properties.h"


#include "core/semantics.h"


#include "objectdatabase/dataobject.h"


void ForceLinkFunctionBSSkin()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionBSSkin() !ATTN!\n");
}



START_CHUNKED_INTERFACE( BSSkinProperties, Mem::MC_PROCEDURAL )
	PUBLISH_VAR_WITH_DEFAULT( m_diffuseWrap, 0.0f )
	PUBLISH_VAR_WITH_DEFAULT( m_specularFacing, 11.0f )
	PUBLISH_VAR_WITH_DEFAULT( m_specularGlancing, 1.0f )
	PUBLISH_VAR_WITH_DEFAULT( m_fuzzColour, CVecMath::GetZeroVector() )
	PUBLISH_VAR_WITH_DEFAULT( m_fuzzTightness, 6.0f )
	PUBLISH_VAR_WITH_DEFAULT( m_subColour, CVecMath::GetZeroVector() )
	PUBLISH_VAR_WITH_DEFAULT( m_normalMapStrength, 1.0f )
	PUBLISH_VAR_WITH_DEFAULT( m_specularStrength, 1.0f )
END_STD_INTERFACE


//eof
