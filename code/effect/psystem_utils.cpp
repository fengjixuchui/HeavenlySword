//--------------------------------------------------
//!
//!	\file psystem_utils.cpp
//!	misc helper functions for particle systems
	//!
//--------------------------------------------------

#include "effect/psystem_utils.h"
#include "objectdatabase/dataobject.h"
#include "psystem_simple.h"
#include "psystem_complex.h"
#include "effect_manager.h"
#include "effect_error.h"
#include "effect_util.h"
#include "particle_movement.h"

//--------------------------------------------------
//!
//!	PSystemUtils::ConstructParticleEffect
//! Returns the GUID of the effect
//!
//--------------------------------------------------
u_int PSystemUtils::ConstructParticleEffect( void* pDef, const CMatrix& frame, const EmitterDef* pEmitterOveride )
{
#ifndef _RELEASE
	static char aErrors[MAX_PATH];
#endif

	if( pDef )
	{
		const char* pType = EffectUtils::GetInterfaceType( pDef );

		if (stricmp(pType,"PSystemSimpleDef")==0)
		{
			PSystemSimpleDef* pDerivedDef = (PSystemSimpleDef*) (pDef);
			
			if ((!pEmitterOveride) && (!pDerivedDef->m_pDefaultEmitterDef))
			{
				#ifndef _RELEASE
				sprintf( aErrors, "ConstructParticleEffect called with no emitter definition" );
				EffectErrorMSG::AddDebugError( aErrors );
				#endif
				return 0;
			}
			
			return EffectManager::Get().AddSortableEffect( NT_NEW_CHUNK( Mem::MC_EFFECTS ) PSystemSimple( *pDerivedDef, frame, pEmitterOveride ) );
		}
		else if (stricmp(pType,"PSystemComplexDef")==0)
		{
			PSystemComplexDef* pDerivedDef = (PSystemComplexDef*) (pDef);

			if ((!pEmitterOveride) && (!pDerivedDef->m_pDefaultEmitterDef))
			{
				#ifndef _RELEASE
				sprintf( aErrors, "ConstructParticleEffect called with no emitter definition" );
				EffectErrorMSG::AddDebugError( aErrors );
				#endif
				return 0;
			}

			return EffectManager::Get().AddSortableEffect( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PSystemComplex( *pDerivedDef, frame, pEmitterOveride ) );
		}
		else if (stricmp(pType,"AnyString")==0)
		{
			void* pNewDef = EffectUtils::GetPtrFromAnyString( pDef );
			return ConstructParticleEffect( pNewDef, frame, pEmitterOveride );
		}
		else
		{
			#ifndef _RELEASE
			sprintf( aErrors, "Unrecognised effect type: %s being constructed", pType );
			EffectErrorMSG::AddDebugError( aErrors );
			#endif
			return 0;
		}
	}
	else
	{
		#ifndef _RELEASE
		sprintf( aErrors, "ConstructParticleEffect called with no definition" );
		EffectErrorMSG::AddDebugError( aErrors );
		#endif
		return 0;
	}
	return 0;
}

//--------------------------------------------------
//!
//!	PSystemUtils::ConstructParticleEffect
//! Returns the GUID of the effect
//!
//--------------------------------------------------
u_int PSystemUtils::ConstructParticleEffect( void* pDef, const Transform* pTransform, const EmitterDef* pEmitterOveride )
{
	ntAssert(pTransform);

	#ifndef _RELEASE
	static char aErrors[MAX_PATH];
	#endif

	if( pDef )
	{
		const char* pType =  EffectUtils::GetInterfaceType( pDef );
		
		if (stricmp(pType,"PSystemSimpleDef")==0)
		{
			PSystemSimpleDef* pDerivedDef = (PSystemSimpleDef*)pDef;

			if ((!pEmitterOveride) && (!pDerivedDef->m_pDefaultEmitterDef))
			{
				#ifndef _RELEASE
				sprintf( aErrors, "ConstructParticleEffect called with no emitter definition" );
				EffectErrorMSG::AddDebugError( aErrors );
				#endif
				return 0;
			}

			return EffectManager::Get().AddSortableEffect( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PSystemSimple( *pDerivedDef, *pTransform, pEmitterOveride ) );
		}
		else if (stricmp(pType,"PSystemComplexDef")==0)
		{
			PSystemComplexDef* pDerivedDef = (PSystemComplexDef*)pDef;

			if ((!pEmitterOveride) && (!pDerivedDef->m_pDefaultEmitterDef))
			{
				#ifndef _RELEASE
				sprintf( aErrors, "ConstructParticleEffect called with no emitter definition" );
				EffectErrorMSG::AddDebugError( aErrors );
				#endif
				return 0;
			}

			return EffectManager::Get().AddSortableEffect( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PSystemComplex( *pDerivedDef, *pTransform, pEmitterOveride ) );
		}
		else if (stricmp(pType,"AnyString")==0)
		{
			void* pNewDef = EffectUtils::GetPtrFromAnyString( pDef );
			return ConstructParticleEffect( pNewDef, pTransform, pEmitterOveride );
		}
		else
		{
			#ifndef _RELEASE
			sprintf( aErrors, "Unrecognised effect type: %s being constructed", pType );
			EffectErrorMSG::AddDebugError( aErrors );
			#endif
		}
	}
	else
	{
		#ifndef _RELEASE
		sprintf( aErrors, "ConstructParticleEffect called with no definition" );
		EffectErrorMSG::AddDebugError( aErrors );
		#endif
	}
	return 0;
}

//--------------------------------------------------
//!
//!	PSystemUtils::ConstructParticleEffect
//!
//--------------------------------------------------
u_int PSystemUtils::ConstructParticleEffect( void* pDef, const CHashedString& pParentName,
														 const CHashedString& pParentTransform,
														 const EmitterDef* pEmitterOveride )
{
	Transform* pTransform = EffectUtils::FindNamedTransform( pParentName, pParentTransform );
	
	// we will already spit out errors on this
	if (pTransform == NULL)
		return 0;

	return ConstructParticleEffect( pDef, pTransform, pEmitterOveride );
}

//--------------------------------------------------
//!
//!	PSystemUtils::GetTechniqueName
//!
//--------------------------------------------------
const char* PSystemUtils::GetTechniqueName( u_int iTechnique )
{
	switch(iTechnique)
	{
	case 0: return "psprite";
	case 1: return "psprite_tex";
	case 2: return "psprite_animtex";
	case 3: return "psprite_randtex";

	case 4: return "psprite_imagetrans";
	case 5: return "psprite_tex_imagetrans";
	case 6: return "psprite_animtex_imagetrans";
	case 7: return "psprite_randtex_imagetrans";
	
	case 8: return "qsprite";
	case 9: return "qsprite_tex";
	case 10: return "qsprite_animtex";
	case 11: return "qsprite_randtex";

	case 12: return "qsprite_imagetrans";
	case 13: return "qsprite_tex_imagetrans";
	case 14: return "qsprite_animtex_imagetrans";
	case 15: return "qsprite_randtex_imagetrans";
	
	case 16: return "qsprite_dh";
	case 17: return "qsprite_tex_dh";
	case 18: return "qsprite_animtex_dh";
	case 19: return "qsprite_randtex_dh";
	}
	ntAssert(0);
	return NULL;
}

const char* PSystemUtils::GetTechniqueName( const PSystemSimple& effect )
{
	u_int iTechnique = GetTechniqueID( effect.GetDefinition().m_eTexMode, effect.GetRenderstates() );
	return GetTechniqueName(iTechnique);
}

const char* PSystemUtils::GetTechniqueName( const PSystemComplex& effect )
{
	u_int iTechnique = GetTechniqueID( effect.GetDefinition().m_eTexMode, effect.GetRenderstates() );
	return GetTechniqueName(iTechnique);
}

//--------------------------------------------------
//!
//!	PSystemUtils::SimpleShaderCostTable
//! derived from num shader instructions
//!
//--------------------------------------------------
#define MAX_SHADER_TYPES	2
#define NUM_TECHNIQUES		20
#define CPU_GPU_COST		2
#define NUM_PRIMITIVES		5

#define TABLE_WIDTH		(MAX_SHADER_TYPES*NUM_TECHNIQUES)
#define TABLE_HEIGHT	(CPU_GPU_COST*NUM_PRIMITIVES)

// these figures come from running Z:\Heavenly_Sword\content\fxshaders\profilefx.sh
// --------------------------------------------------------------------------------

// Technique reads horizontally, in vertexshader / pixelshader pairs, in blocks of four (NOTEX/TEX/ANIMTEX/RANDTEX):
// pointsprite, pointsprite_imagetransform, quadsprite, quadsprite_imagetransform, quadsprite_depthhaze

// Vertically it reads by primitive type, in CPU / GPU pairs.

// last updated 14/01/05 by WD
int SimpleShaderCostTable[TABLE_HEIGHT*TABLE_WIDTH] = 
{
	// PT_SIMPLE_SPRITE
	13,1,	13,3,	13,15,	13,15,		13,6,	13,8,	13,20,	13,20,		16,2,	16,4,	16,16,	16,10,		16,6,	16,8,	16,20,	16,14,		44,4,	44,6,	44,18,	44,12,		
	27,1,	27,3,	27,15,	27,15,		27,6,	27,8,	27,20,	27,20,		28,1,	28,3,	28,15,	28,9,		28,6,	28,8,	28,20,	28,14,		56,4,	56,6,	56,18,	56,12,		

	// PT_ROTATING_SPRITE
	14,1,	14,19,	14,29,	14,29,		14,6,	14,24,	14,34,	14,34,		29,2,	29,4,	29,16,	29,10,		29,6,	29,8,	29,20,	29,14,		57,4,	57,6,	57,18,	57,12,		
	37,1,	37,19,	37,29,	37,29,		37,6,	37,24,	37,34,	37,34,		38,1,	38,3,	38,15,	38,9,		38,6,	38,8,	38,20,	38,14,		66,4,	66,6,	66,18,	66,12,		
	
	// PT_WORLD_ALIGNED_QUAD
	16,2,	16,4,	16,16,	16,10,		16,6,	16,8,	16,20,	16,14,		16,2,	16,4,	16,16,	16,10,		16,6,	16,8,	16,20,	16,14,		44,4,	44,6,	44,18,	44,12,		
	30,2,	30,4,	30,16,	30,10,		30,6,	30,8,	30,20,	30,14,		30,2,	30,4,	30,16,	30,10,		30,6,	30,8,	30,20,	30,14,		58,4,	58,6,	58,18,	58,12,		
	
	// PT_AXIS_ALIGNED_RAY
	22,2,	22,4,	22,16,	22,10,		22,6,	22,8,	22,20,	22,14,		22,2,	22,4,	22,16,	22,10,		22,6,	22,8,	22,20,	22,14,		50,4,	50,6,	50,18,	50,12,		
	36,2,	36,4,	36,16,	36,10,		36,6,	36,8,	36,20,	36,14,		36,2,	36,4,	36,16,	36,10,		36,6,	36,8,	36,20,	36,14,		64,4,	64,6,	64,18,	64,12,		
	
	// PT_VELOCITY_ALIGNED_RAY
	27,2,	27,4,	27,16,	27,10,		27,6,	27,8,	27,20,	27,14,		27,2,	27,4,	27,16,	27,10,		27,6,	27,8,	27,20,	27,14,		55,4,	55,6,	55,18,	55,12,		
	48,2,	48,4,	48,16,	48,10,		48,6,	48,8,	48,20,	48,14,		48,2,	48,4,	48,16,	48,10,		48,6,	48,8,	48,20,	48,14,		76,4,	76,6,	76,18,	76,12,		
};

int FunctionalShaderCostTable[TABLE_HEIGHT*TABLE_WIDTH] = 
{
	// PT_SIMPLE_SPRITE
	11,4,	11,6,	11,18,	11,18,		11,8,	11,10,	11,22,	11,22,		14,4,	14,6,	14,18,	14,12,		14,8,	14,10,	14,22,	14,16,		42,6,	42,8,	42,20,	42,14,		
	42,2,	42,4,	42,16,	42,16,		42,7,	42,9,	42,21,	42,21,		41,2,	41,4,	41,16,	41,10,		41,7,	41,9,	41,21,	41,15,		69,5,	69,7,	69,19,	69,13,		

	// PT_ROTATING_SPRITE
	12,2,	12,20,	12,30,	12,30,		12,7,	12,25,	12,35,	12,35,		27,4,	27,6,	27,18,	27,12,		27,8,	27,10,	27,22,	27,16,		55,6,	55,8,	55,20,	55,14,		
	46,2,	46,20,	46,30,	46,30,		46,7,	46,25,	46,35,	46,35,		51,2,	51,4,	51,16,	51,10,		51,7,	51,9,	51,21,	51,15,		79,5,	79,7,	79,19,	79,13,		

	// PT_WORLD_ALIGNED_QUAD
	14,4,	14,6,	14,18,	14,12,		14,8,	14,10,	14,22,	14,16,		14,4,	14,6,	14,18,	14,12,		14,8,	14,10,	14,22,	14,16,		42,6,	42,8,	42,20,	42,14,		
	41,2,	41,4,	41,16,	41,10,		41,7,	41,9,	41,21,	41,15,		41,2,	41,4,	41,16,	41,10,		41,7,	41,9,	41,21,	41,15,		69,5,	69,7,	69,19,	69,13,		

	// PT_AXIS_ALIGNED_RAY
	20,4,	20,6,	20,18,	20,12,		20,8,	20,10,	20,22,	20,16,		20,4,	20,6,	20,18,	20,12,		20,8,	20,10,	20,22,	20,16,		48,6,	48,8,	48,20,	48,14,		
	47,2,	47,4,	47,16,	47,10,		47,7,	47,9,	47,21,	47,15,		47,2,	47,4,	47,16,	47,10,		47,7,	47,9,	47,21,	47,15,		75,5,	75,7,	75,19,	75,13,		
	
	// PT_VELOCITY_ALIGNED_RAY
	25,4,	25,6,	25,18,	25,12,		25,8,	25,10,	25,22,	25,16,		25,4,	25,6,	25,18,	25,12,		25,8,	25,10,	25,22,	25,16,		53,6,	53,8,	53,20,	53,14,		
	59,2,	59,4,	59,16,	59,10,		59,7,	59,9,	59,21,	59,15,		59,2,	59,4,	59,16,	59,10,		59,7,	59,9,	59,21,	59,15,		87,5,	87,7,	87,19,	87,13,		
};


int SimpleCPUCostTable[NUM_PRIMITIVES*2] = 
{
	96,		256,
	109,	260,
	340,	500,
	336,	500,
	340,	500,
};

int FunctionalCPUCostTable[NUM_PRIMITIVES*2] = 
{
	140,	440,
	147,	450,
	470,	650,
	480,	660,
	505,	717,
};

//--------------------------------------------------
//!
//!	PSystemUtils::GetVSCost
//!
//--------------------------------------------------
const char* PSystemUtils::GetVSCost( u_int iScore )
{
	if (iScore >= 300)
		return "VHIGH";
	else if (iScore >= 200)
		return "HIGH";
	else if (iScore >= 100)
		return "MID";
	else if (iScore >= 50)
		return "LOW";
	else
		return "VLOW";
}

//--------------------------------------------------
//!
//!	PSystemUtils::GetPSCost
//!
//--------------------------------------------------
const char* PSystemUtils::GetCPUCost( u_int iScore )
{
	if (iScore >= 2000)
		return "KAHHHHHHHHHHHN";
	else if (iScore >= 1500)
		return "VVHIGH";
	else if (iScore >= 1000)
		return "VHIGH";
	else if (iScore >= 600)
		return "HIGH";
	else if (iScore >= 300)	
		return "MID";
	else if (iScore >= 150)
		return "LOW";
	else
		return "VLOW";
}

//--------------------------------------------------
//!
//!	PSystemUtils::GetPSCost
//!
//--------------------------------------------------
const char* PSystemUtils::GetPSCost( u_int iScore )
{
	if (iScore >= 30)
		return "VVHIGH";
	else if (iScore >= 20)
		return "VHIGH";
	else if (iScore >= 15)
		return "HIGH";
	else if (iScore >= 10)
		return "MID";
	else if (iScore >= 5)
		return "LOW";
	else
		return "VLOW";
}

//--------------------------------------------------
//!
//!	PSystemUtils::GetApproxCost
//!
//--------------------------------------------------

void PSystemUtils::GetApproxCost(const PSystemSimple& effect, const char** ppCPU, const char** ppVS, const char** ppPS )
{
	u_int iTechnique = GetTechniqueID( effect.GetDefinition().m_eTexMode, effect.GetRenderstates() );

	u_int iXIndex = (iTechnique * MAX_SHADER_TYPES);
	u_int iYIndex = ((u_int)effect.GetDefinition().m_particleType * CPU_GPU_COST) + (effect.GetParticles().UsingCPUParticles() ? 0 : 1);
	u_int iIndex = (iYIndex * TABLE_WIDTH) + iXIndex;

	ntError( iIndex < (TABLE_WIDTH * TABLE_HEIGHT) );
	
	u_int iVScost = SimpleShaderCostTable[ iIndex ] * (effect.GetParticles().UsingPointSprites() ? 1 : 4);
	*ppVS = GetVSCost( iVScost );

	u_int iPScost = SimpleShaderCostTable[ iIndex+1 ];
	*ppPS = GetPSCost( iPScost );

	u_int iCPUCost = SimpleCPUCostTable[	((u_int)effect.GetDefinition().m_particleType * 2)+
											(effect.GetParticles().UsingCPUParticles() ? 1 : 0) ];

	iCPUCost += effect.GetDefinition().RequiresSorting() ? 2000 : 0;
	iCPUCost += effect.GetDefinition().m_bUseRayCast ? 2000 : 0;
	*ppCPU = GetCPUCost( iCPUCost );
}

void PSystemUtils::GetApproxCost(const PSystemComplex& effect, const char** ppCPU, const char** ppVS, const char** ppPS )
{
	u_int iTechnique = GetTechniqueID( effect.GetDefinition().m_eTexMode, effect.GetRenderstates() );

	u_int iXIndex = (iTechnique * MAX_SHADER_TYPES);
	u_int iYIndex = ((u_int)effect.GetDefinition().m_particleType * CPU_GPU_COST) + (effect.GetParticles().UsingCPUParticles() ? 0 : 1);
	u_int iIndex = (iYIndex * TABLE_WIDTH) + iXIndex;
	
	ntError( iIndex < (TABLE_WIDTH * TABLE_HEIGHT) );

	u_int iVScost = FunctionalShaderCostTable[ iIndex ] * (effect.GetParticles().UsingPointSprites() ? 1 : 4);
	*ppVS = GetVSCost( iVScost );

	u_int iPScost = FunctionalShaderCostTable[ iIndex+1 ];
	*ppPS = GetPSCost( iPScost );

	u_int iCPUCost = FunctionalCPUCostTable[	((u_int)effect.GetDefinition().m_particleType * 2)+
												(effect.GetParticles().UsingCPUParticles() ? 1 : 0) ];

	if (effect.GetDefinition().m_pAdvMovement)
	{
		if (effect.GetDefinition().m_pAdvMovement->m_bUseVelocityScaling)
			iCPUCost += 100;
		if (effect.GetDefinition().m_pAdvMovement->m_bUseRocketAcc)
			iCPUCost += 225;
		if (effect.GetDefinition().m_pAdvMovement->m_bUseWorldAcc)
			iCPUCost += 260;
		if (effect.GetDefinition().m_pAdvMovement->m_bUseSteering)
			iCPUCost += 900;
	}
		
	iCPUCost += effect.GetDefinition().RequiresSorting() ? 2000 : 0;
	iCPUCost += effect.GetDefinition().m_bUseRayCast ? 2000 : 0;
	*ppCPU = GetCPUCost( iCPUCost );
}

//--------------------------------------------------
//!
//!	PSystemUtils::GetTechniqueID
//! caluculates index of technique
//!
//--------------------------------------------------
u_int PSystemUtils::GetTechniqueID( PARTICLE_TEXTURE_MODE eTexMode, const RenderStateBlock& rs )
{
	u_int iTechnique = 0;

	// what texture mode are we at?
	switch (eTexMode)
	{
	case PTM_SIMPLE_TEXTURED:	iTechnique += 1;	break;
	case PTM_ANIM_TEXTURED:		iTechnique += 2;	break;
	case PTM_RAND_TEXTURED:		iTechnique += 3;	break;
	default: break;
	}

	if (rs.m_renderType == ERT_HDR_DEPTH_HAZED)
	{
		// depth haze requires interpolators, so we're always quad based
		iTechnique += 16;
	}
	else
	{
		// do we need an explicit image transform?
		if (rs.m_renderType == ERT_LOW_DYNAMIC_RANGE)
			iTechnique += 4;

		// are we a point or quadsprite?
		if (!rs.m_bPointSprite)
			iTechnique += 8;
	}

	return iTechnique;
}

//--------------------------------------------------
//!
//!	PSystemUtils::SetFXTechique
//! Assumes the above techniques are present in the FX file.
//!
//--------------------------------------------------
#ifdef PLATFORM_PC
void PSystemUtils::SetFXTechique(	ID3DXEffect* pFX, 
										PARTICLE_TEXTURE_MODE eTexMode,
										const RenderStateBlock& rs )
{
	ntAssert(pFX);
	u_int iTechnique = GetTechniqueID( eTexMode, rs );
	HRESULT hr;
	hr = pFX->SetTechnique( PSystemUtils::GetTechniqueName(iTechnique) );
	ntAssert_p( hr == S_OK,("Technique %s not found in this FX file", PSystemUtils::GetTechniqueName(iTechnique) ) );
}
#endif

//--------------------------------------------------
//!
//!	PSystemUtils::InitialiseQuadList
//! standardish init of most particle VB's
//!
//--------------------------------------------------
void PSystemUtils::InitialiseQuadList(	QuadList& quads, void* pCPUTemplate,
											u_int iBirthSlot, u_int iTextureSlot, bool bSetRand )
{
	ProceduralVB& vb = quads.GetGPUData();
	memset( vb.GetVertex(0), 0, vb.GetVertexSize() * vb.GetMaxVertices() );

	for ( u_int i = 0; i < vb.GetMaxVertices(); i++ )
		*((float*)vb.GetVertexElement( i, iBirthSlot )) = -MAX_POS_FLOAT;

#ifdef PLATFORM_PC
	static float texCoords[8] = 
	{
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
	};
#else
	static float texCoords[8] = 
	{
		0.0f, 1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
	};
#endif

	if (iTextureSlot != 0xffffffff)
	{
		float fRand = 0.0f;
		for ( u_int i = 0; i < vb.GetMaxVertices(); i++ )
		{
			float* pTexU = (float*)vb.GetVertexElement( i, iTextureSlot );
			float* pTexV = pTexU + 1;

			*pTexU = texCoords[0 + (i % 4)];
			*pTexV = texCoords[4 + (i % 4)];

			if (bSetRand)
			{
				if ((i % 4) == 0)
					fRand = erandf( 1.0f );

				float* pRand = pTexV + 1;
				*pRand = fRand;
			}
		}
	}

	if ( quads.GetCPUData().IsValid() )
	{
		for ( u_int i = 0; i < quads.GetCPUData().GetMaxVertices(); i++ )
			quads.GetCPUData().SetVertex( i, pCPUTemplate );
	}
}
