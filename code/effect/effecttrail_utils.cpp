//--------------------------------------------------
//!
//!	\file effecttrail_utils.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "effect_util.h"
#include "effecttrail_utils.h"
#include "effecttrail_simple.h"
#include "effecttrail_line.h"
#include "effect/effect_manager.h"
#include "effect_error.h"
#include "objectdatabase/dataobject.h"

//--------------------------------------------------
//!
//!	TrailUtils::ConstructTrailEffect
//! Returns the GUID of the effect
//!
//--------------------------------------------------
u_int TrailUtils::ConstructTrailEffect( void* pDef, const CHashedString& pEntName,
													const CHashedString& pTransformName,
													void* pAdditional )
{
	Transform* pTransform = EffectUtils::FindNamedTransform( pEntName, pTransformName );

	// we will already spit out errors on this
	if (pTransform == NULL)
		return 0;

	return ConstructTrailEffect( pDef, pTransform, pAdditional );
}

//--------------------------------------------------
//!
//!	TrailUtils::ConstructTrailEffect
//! Returns the GUID of the effect
//!
//--------------------------------------------------
u_int TrailUtils::ConstructTrailEffect( void* pDef, const Transform* pTransform,
													void* pAdditional )
{
	ntAssert(pTransform);

	if( pDef )
	{
		const char* pType =  EffectUtils::GetInterfaceType( pDef );

		if (stricmp(pType,"EffectTrail_SimpleDef")==0)
		{
			EffectTrail_EdgeDef* pEdge = (EffectTrail_EdgeDef*)pAdditional;
			EffectTrail_SimpleDef* pDerivedDef = (EffectTrail_SimpleDef*)pDef;
			return EffectManager::Get().AddSortableEffect( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) EffectTrail_Simple( pDerivedDef, pEdge, pTransform ) );
		}
		else if (stricmp(pType,"EffectTrail_LineDef")==0)
		{
			CPoint* pOffset = (CPoint*)pAdditional;
			EffectTrail_LineDef* pDerivedDef = (EffectTrail_LineDef*)pDef;
			return EffectManager::Get().AddSortableEffect( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) EffectTrail_Line( pDerivedDef, pTransform, pOffset ) );
		}
		else if (stricmp(pType,"AnyString")==0)
		{
			void* pNewDef = EffectUtils::GetPtrFromAnyString( pDef );
			return ConstructTrailEffect( pNewDef, pTransform, pAdditional );
		}
		else
		{
			#ifndef _RELEASE
			static char aErrors[MAX_PATH];
			sprintf( aErrors, "Unrecognised effect type: %s being constructed", pType );
			EffectErrorMSG::AddDebugError( aErrors );
			#endif
		}
	}
	else
	{
		#ifndef _RELEASE
		static char aErrors[MAX_PATH];
		sprintf( aErrors, "ConstructParticleEffect called with no definition" );
		EffectErrorMSG::AddDebugError( aErrors );
		#endif
	}
	return 0;	
}

//--------------------------------------------------
//!
//!	TrailUtils::GetTechniqueName
//! corresponds to technique names in FX files
//!
//--------------------------------------------------
const char* TrailUtils::GetTechniqueName( u_int iTechnique )
{
	switch(iTechnique)
	{
	case 0: return "normal";
	case 1: return "normal_tex";
	case 2: return "normal_animtex";

	case 3: return "normal_imagetrans";
	case 4: return "normal_tex_imagetrans";
	case 5: return "normal_animtex_imagetrans";

	case 6: return "normal_dh";
	case 7: return "normal_tex_dh";
	case 8: return "normal_animtex_dh";
	}
	ntAssert(0);
	return NULL;
}

//--------------------------------------------------
//!
//!	TrailUtils::GetTechniqueID
//! Calc technique name index based on tex mode and RS
//!
//--------------------------------------------------
u_int TrailUtils::GetTechniqueID( TRAIL_TEXTURE_MODE eTexMode, const RenderStateBlock& rs )
{
	u_int iTechnique = 0;

	// what texture mode are we at?
	switch (eTexMode)
	{
	case TTM_SIMPLE_TEXTURED:	iTechnique += 1;	break;
	case TTM_ANIM_TEXTURED:		iTechnique += 2;	break;
	default: break;
	}

	// do we need an explicit image transform?
	if (rs.m_renderType == ERT_LOW_DYNAMIC_RANGE)
		iTechnique += 3;
	else if (rs.m_renderType == ERT_HDR_DEPTH_HAZED)
		iTechnique += 6;

	return iTechnique;
}
