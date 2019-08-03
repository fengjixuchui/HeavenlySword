//--------------------------------------------------
//!
//!	\file effecttrail_utils.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _EFFECT_TRAIL_UTILS_H
#define _EFFECT_TRAIL_UTILS_H

#include "renderstate_block.h"

class Transform;
class EffectTrail_EdgeDef;

//--------------------------------------------------
//!
//!	TRAIL_TEXTURE_MODE
//! Are we textured, animated or what?
//!
//--------------------------------------------------
enum TRAIL_TEXTURE_MODE
{
	TTM_UNTEXTURED,			// no tex
	TTM_SIMPLE_TEXTURED,	// single tex
	TTM_ANIM_TEXTURED,		// animate through a texture dict
};

//--------------------------------------------------
//!
//!	TrailUtils
//! Static class holding helper functions for trails
//!
//--------------------------------------------------
class TrailUtils
{
public:
	static u_int ConstructTrailEffect( void* pDef,	const CHashedString& pEntName,
													const CHashedString& pTransformName,
													void* pAdditional = 0);

	static u_int ConstructTrailEffect( void* pDef,	const Transform* pTrans,
													void* pAdditional = 0);

	static const char* GetTechniqueName( u_int iTechnique );
	static u_int GetTechniqueID( TRAIL_TEXTURE_MODE eTexMode, const RenderStateBlock& rs );
};

#endif
