/***************************************************************************************************
*
*	DESCRIPTION		The entry point for the animation update system on SPUs.
*
*	NOTES
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
struct ScissorRegion
{
    int    left;
    int    top;
    int    right;
    int    bottom;
}; 

#include "ntlib_spu/vecmath_spu_ps3.h"
#include "ntlib_spu/util_spu.h"
#include "ntlib_spu/debug_spu.h"
#include "ntlib_spu/fixups_spu.h"

#include "core/semantics.h"

#include <stdlib.h> // for qsort
#include <string.h> // for memcpy
#include "heresy/heresy_capi.h"

#include "gfx/renderer_ppu_spu.h"
#include "core/perfanalyzer_spu_ps3.h"

SPU_MODULE_FIXUP()


void FillSHMatricesInPixelShader( restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, const BatchRenderRenderContext* pContext)
{
	// this whole function need alot of work...
	switch( pPatch->m_offset )
	{
	case 0:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[0][0] );
		break;
	case 1:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[0][1] );
		break;
	case 2:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[0][2] );
		break;
	case 3:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[0][3] );
		break;

	case 4:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[1][0] );
		break;
	case 5:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[1][1] );
		break;
	case 6:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[1][2] );
		break;
	case 7:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[1][3] );
		break;

	case 8:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[2][0] );
		break;
	case 9:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[2][1] );
		break;
	case 10:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[2][2] );
		break;
	case 11:
		Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_SHMatrices[2][3] );
		break;	
	}
}




void FixupPixelShaderConstant(	restrict Heresy_PushBuffer* pPB, 
								const restrict Heresy_PushBufferPatch* pPatch, 
								const BatchRenderRenderContext* pContext, 
								const RendererMaterialInstanceData* pInstanceData )
{
	const static CVector halfVec = CVector( 0.5f, 0.5f, 0.5f, 0.5f );
	switch( pPatch->m_Semantic )
	{
		case PROPERTY_EXPOSURE_AND_TONEMAP:
		{	
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_ExposureAndToneMapConsts.QuadwordValue() );	
			break;
		}
		case PROPERTY_REFRACTION_WARP:
		{
			float cameraWidth = pContext->m_fScreenAspectRatio * tanf( 0.5f * pContext->m_fCameraFOVAngle );
			float refractionWarp = 0.25f * cameraWidth;
			CVector temp( refractionWarp, refractionWarp, refractionWarp, refractionWarp );
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, temp.QuadwordValue() );
			break;
		}
		// properties that could be on vertex or pixel shaders...
		case PROPERTY_ALPHATEST_THRESHOLD:
		{
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, halfVec.QuadwordValue() );
		}
		break;

		case PROPERTY_FILL_SH_MATRICES:
		{
			FillSHMatricesInPixelShader( pPB, pPatch, pContext );
		}
		break;
		case PROPERTY_KEY_DIR_COLOUR:
		{
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_keyColour.QuadwordValue() );
		}
		break;
		case PROPERTY_KEY_DIR_OBJECTSPACE:
		{
			CMatrix WorldToObj;
			WorldToObj = pInstanceData->obObjectToWorld.GetAffineInverse();
			CDirection temp( pContext->m_toKeyLight.QuadwordValue() * WorldToObj );
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, temp.QuadwordValue() );
		}
		break;
		case PROPERTY_VIEW_POSITION_OBJECTSPACE:
		{
			CMatrix mWorldToObj = pInstanceData->obObjectToWorld.GetAffineInverse();
			CPoint temp( pContext->m_eyePos.QuadwordValue() * mWorldToObj );
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, temp.QuadwordValue() );
			break;
		}
		case PROPERTY_BSSKIN_DIFFUSE_WRAP:
		case PROPERTY_BSSKIN_SPECULAR_FACING:
		case PROPERTY_BSSKIN_SPECULAR_GLANCING:
		case PROPERTY_BSSKIN_FUZZ_COLOUR:
		case PROPERTY_BSSKIN_FUZZ_TIGHTNESS:
		case PROPERTY_BSSKIN_SUBCOLOUR:
		case PROPERTY_BSSKIN_WRINKLE_REGIONS0_WEIGHTS:
		case PROPERTY_BSSKIN_WRINKLE_REGIONS1_WEIGHTS:
		case PROPERTY_BSSKIN_NORMAL_MAP_STRENGTH:
		case PROPERTY_BSSKIN_SPECULAR_STRENGTH:
		case PROPERTY_SPEEDTREE_ALPHATRESHOLD:
		{
			// nothing, this value are set by speedtree code...
		}
		break;
		//----------------------
		// Pixel shader properties follow
		//----------------------
		case PROPERTY_KEY_DIR_REFLECTANCESPACE:
		case PROPERTY_KEY_DIR_WORLDSPACE:
		{
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_toKeyLight.QuadwordValue());
		}
		break;
		case PROPERTY_REFLECTANCE_MAP_COLOUR:
		{
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_reflectanceCol.QuadwordValue() );
		}
		break;

		case PROPERTY_SHADOW_PLANES:
		{
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_shadowMapPlanesW.QuadwordValue() );
		}
		break;
		case PROPERTY_SHADOW_RADII:
		case PROPERTY_SHADOW_RADII1:
		case PROPERTY_SHADOW_RADII2:
		case PROPERTY_SHADOW_RADII3:
		{
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, pContext->m_shadowRadii[pPatch->m_Semantic - PROPERTY_SHADOW_RADII].QuadwordValue() );
		}
		break;
		case PROPERTY_SHADOW_PLANE0:
		case PROPERTY_SHADOW_PLANE1:
		case PROPERTY_SHADOW_PLANE2:
		case PROPERTY_SHADOW_PLANE3:
		case PROPERTY_SHADOW_PLANE4:
		case PROPERTY_PARALLAX_SCALE_AND_BIAS:
		{
			CVector scaleAndBias = CVector( 0.02f, -0.01f, 0.0f, 0.0f );
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, scaleAndBias.QuadwordValue() );
		}
		break;
		case PROPERTY_VPOS_TO_UV:
		{
			CVector posToUV = CVector(	1.f / (pContext->m_RenderTargetSize.X() - 1.0f),
										1.f / (pContext->m_RenderTargetSize.Y() - 1.0f),
										1.f / pContext->m_RenderTargetSize.X(),
										-1.f / pContext->m_RenderTargetSize.Y() );
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, posToUV.QuadwordValue() );
		}
		break;
		case PROPERTY_HAIR_BASETEX_FREQ:
		case PROPERTY_HAIR_SHIFTTEX_FREQ:
		case PROPERTY_HAIR_MASKTEX_FREQ:
		{
			CVector tmp(5.0f,5.0f,0.0f,0.0f);
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, tmp.QuadwordValue() );
			break;
		}

		case PROPERTY_DEPTH_HAZE_KILL_POWER:
		{
			CVector tmp(8.0f,8.0f,8.0f,8.0f);
			Heresy_FixupPixelShaderConstantScratch16B_reg( pPB, pPatch, tmp.QuadwordValue() );
			break;
		}

		case PROPERTY_DEPTHOFFIELD_PARAMS:
		default:
			ntError_p( false, ("SPU Renderer - cannot set pixel property %i\n", pPatch->m_Semantic) );
	};
}


void FixupTexture(restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, const BatchRenderRenderContext* pContext )  
{
	switch( pPatch->m_Semantic )
	{
	// texture properties
	case TEXTURE_SHADOW_MAP:
	case TEXTURE_SHADOW_MAP1:
	case TEXTURE_SHADOW_MAP2:
	case TEXTURE_SHADOW_MAP3:
		{
			Heresy_FixupTextureLeaveOverrides( pPB, &pContext->m_pShadowMap, pPatch );
		}
		break;

	case TEXTURE_STENCIL_MAP:
		{
			Heresy_FixupTextureLeaveOverrides( pPB, &pContext->m_pStencilTarget, pPatch );
		}
		break;
	case TEXTURE_IRRADIANCE_CACHE:
		{
			Heresy_FixupTextureLeaveOverrides( pPB, &pContext->m_pIrradianceCache, pPatch );
			break;
		}
	case TEXTURE_REFLECTANCE_MAP:
		{
			Heresy_FixupTextureLeaveOverrides( pPB, &pContext->m_reflectanceMap, pPatch );
		}
		break;
	default:
		ntError( false );
	}
}

#define MAX_BONES	200
	
ntDMA_ID g_bonesId[2];

uint8_t g_i32UnAlignedIndices[MAX_BONES + 32 ];
CVector g_Bones[MAX_BONES * 3 * 2];
CVector g_FinalBones[64 * 3];

void PatchSkinMatrices( restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, const RendererMaterialInstanceData* pInstanceData  ) 
{
	// stall for indices
	ntDMA::StallForCompletion( g_bonesId[0] );
	// stall for matrices
	ntDMA::StallForCompletion( g_bonesId[1] );

	// compensate for indices misalignment
	uint8_t *indices = g_i32UnAlignedIndices + (((uint32_t)pInstanceData->pBoneIndices) & 0xF);

	CVector *bones = g_FinalBones;

	// upload only the matrices used by this mesh
	for(uint16_t iBone = 0; iBone < (pInstanceData->BoneCount); ++iBone)
	{
		// get the matrix index
		uint16_t iMatrixIndex = 3 * ((uint16_t)indices[iBone]);
		// copy this bone
		*bones++ = g_Bones[iMatrixIndex + 0];
		*bones++ = g_Bones[iMatrixIndex + 1];
		*bones++ = g_Bones[iMatrixIndex + 2];   

	//	memset(bones, 0x0, 16*3);
	//	bones += 3;


	}

	Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, g_FinalBones, 3 * pInstanceData->BoneCount);
}

void FixupVertexShaderConstant(restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, const BatchRenderRenderContext* pContext, const RendererMaterialInstanceData* pInstanceData )
{
	switch( pPatch->m_Semantic )
	{
		case PROPERTY_GENERIC_TEXCOORD0_SCALE_BIAS:
		case PROPERTY_GENERIC_TEXCOORD1_SCALE_BIAS:
		case PROPERTY_GENERIC_TEXCOORD2_SCALE_BIAS:
		case PROPERTY_GENERIC_TEXCOORD3_SCALE_BIAS:
		case PROPERTY_GENERIC_TEXCOORD4_SCALE_BIAS:
		case PROPERTY_GENERIC_TEXCOORD5_SCALE_BIAS:
		case PROPERTY_GENERIC_TEXCOORD6_SCALE_BIAS:
		case PROPERTY_GENERIC_TEXCOORD7_SCALE_BIAS:
		{
			CVector temp(1.0f, 1.0f, 0.0f, 0.0f);
			Heresy_FixupCopyN128bitsVertexShaderConstant(pPB, pPatch, &temp, 1);
		}
		break;

	// properties that could be on vertex or pixel shaders...
		case PROPERTY_FILL_SH_MATRICES:
		{
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, (void*)&pContext->m_SHMatrices, 4*3 );
		}
		break;
		case PROPERTY_KEY_DIR_COLOUR:
		{
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &pContext->m_keyColour, 1 );
		}
		break;
		case PROPERTY_KEY_DIR_OBJECTSPACE:
		{
			CMatrix mWorldToObj = pInstanceData->obObjectToWorld.GetAffineInverse();
			CDirection temp( pContext->m_toKeyLight.QuadwordValue() * mWorldToObj );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &temp, 1 );
		}
		break;
		case PROPERTY_SPEEDTREE_ALPHATRESHOLD:
		{

		}
		break;
		case PROPERTY_BLEND_TRANSFORMS:
		{
			PatchSkinMatrices( pPB, pPatch, pInstanceData );
			break;
		}

		case PROPERTY_POSITION_RECONST_MAT:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp( pInstanceData->obReconstructionMatrix );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &temp, 4 );
			break;
		}
		case PROPERTY_PROJECTION_NORECONST_MAT:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp( pInstanceData->obObjectToWorld * pContext->m_worldToScreen );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &temp, 4 );
			break;
		}
		case PROPERTY_PROJECTION:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp, temp2;	
			MatrixMultiply( temp, pInstanceData->obObjectToWorld, pContext->m_worldToScreen );
			if ( pInstanceData->bApplyPosReconstMatrix )
			{
				MatrixMultiply( temp2, pInstanceData->obReconstructionMatrix, temp );
				Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &temp2, 4 );
			}
			else Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &temp, 4 );

			break;
		}

		case PROPERTY_WORLD_TRANSFORM:
		case PROPERTY_REFLECTANCE_MAP_TRANSFORM:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &pInstanceData->obObjectToWorld, 4 );
			break;
		}
		case PROPERTY_SHADOW_MAP_TRANSFORM:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix mWorldToObj = pInstanceData->obObjectToWorld.GetAffineInverse();
			CMatrix temp( mWorldToObj * pContext->m_shadowMapProjection[0] );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &temp, 4 );
			break;
		}
		case PROPERTY_SHADOW_MAP_TRANSFORM1:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix mWorldToObj = pInstanceData->obObjectToWorld.GetAffineInverse();
			CMatrix temp( mWorldToObj * pContext->m_shadowMapProjection[1] );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &temp, 4 );
			break;
		}
		case PROPERTY_SHADOW_MAP_TRANSFORM2:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix mWorldToObj = pInstanceData->obObjectToWorld.GetAffineInverse();
			CMatrix temp( mWorldToObj * pContext->m_shadowMapProjection[2] );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &temp, 4 );
			break;
		}
		case PROPERTY_SHADOW_MAP_TRANSFORM3:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix mWorldToObj = pInstanceData->obObjectToWorld.GetAffineInverse();
			CMatrix temp( mWorldToObj * pContext->m_shadowMapProjection[3] );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &temp, 4 );
			break;
		}
		case PROPERTY_VIEW_POSITION_OBJECTSPACE:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix mWorldToObj = pInstanceData->obObjectToWorld.GetAffineInverse();
			CPoint temp( pContext->m_eyePos.QuadwordValue() * mWorldToObj );
			Heresy_FixupCopyN128bitsVertexShaderConstant(pPB, pPatch, &temp, 1);
			break;
		}
		case PROPERTY_GAME_TIME:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_FixupCopyN128bitsVertexShaderConstant(pPB, pPatch, &pContext->m_gameTime, 1);
			break;
		}
		case PROPERTY_VIEW_TRANSFORM:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp;
			MatrixMultiply( temp, pInstanceData->obObjectToWorld, pContext->m_worldToView );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &temp, 4 );
			break;
		}
		case PROPERTY_DEPTH_HAZE_CONSTS_A:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &pContext->m_depthHazeA, 1 );
			break;	
		}
		case PROPERTY_DEPTH_HAZE_CONSTS_G:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
//			Heresy_Set128bit_reg( pPB->m_pStart + pPatch->m_offset, pContext->m_depthHazeG.QuadwordValue() );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &pContext->m_depthHazeG, 1 );
			break;
		}
		case PROPERTY_DEPTH_HAZE_BETA1PLUSBETA2:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
//			Heresy_Set128bit_reg( pPB->m_pStart + pPatch->m_offset, pContext->m_depthHazeBeta1PlusBeta2.QuadwordValue() );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &pContext->m_depthHazeBeta1PlusBeta2, 1 );
			break;
		}
		case PROPERTY_DEPTH_HAZE_RECIP_BETA1PLUSBETA2:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
//			Heresy_Set128bit_reg( pPB->m_pStart + pPatch->m_offset, pContext->m_depthHazeOneOverBeta1PlusBeta2.QuadwordValue() );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &pContext->m_depthHazeOneOverBeta1PlusBeta2, 1 );
			break;
		}
		case PROPERTY_DEPTH_HAZE_BETADASH1:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
//			Heresy_Set128bit_reg( pPB->m_pStart + pPatch->m_offset, pContext->m_depthHazeBetaDash1.QuadwordValue() );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &pContext->m_depthHazeBetaDash1, 1 );
			break;
		}
		case PROPERTY_DEPTH_HAZE_BETADASH2:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
//			Heresy_Set128bit_reg( pPB->m_pStart + pPatch->m_offset, pContext->m_depthHazeBetaDash2.QuadwordValue() );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &pContext->m_depthHazeBetaDash2, 1 );
			break;
		}
		case PROPERTY_SUN_DIRECTION_OBJECTSPACE:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix mWorldToObj = pInstanceData->obObjectToWorld.GetAffineInverse();
			CDirection obLightInObject( pContext->m_sunDirection.QuadwordValue() * mWorldToObj );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &obLightInObject, 1 );
			break;
		}
		case PROPERTY_SUN_COLOUR:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
//			Heresy_Set128bit_reg( pPB->m_pStart + pPatch->m_offset, pContext->m_depthHazeSunColour.QuadwordValue() );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &pContext->m_depthHazeSunColour, 1 );
			break;
		}
		case PROPERTY_HAZE_FRAGMENT_EXTINCTION:
		{	
			CVector vOpjPos = CVector(0.0f, 0.0f, 0.0f, 1.0f) * (pInstanceData->obObjectToWorld);

			float s = (vOpjPos * pContext->m_worldToView).Length();
			CVector Fex = -s * pContext->m_depthHazeBeta1PlusBeta2;
			Fex = CVector( expf(Fex.Y()), expf(Fex.Y()), expf(Fex.Z()), 0.0f );
		
			Fex.Max(CVector(0.0f, 0.0f, 0.0f, 0.0f));
			Fex.Min(CVector(1.0f, 1.0f, 1.0f, 0.0f));

			Fex = CVector(1.0f, 1.0f, 1.0f, 0.0f);
					
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &Fex, 1);	
			break;
		}
		case PROPERTY_HAZE_FRAGMENT_SCATTER:
		{	


			CVector vOpjPos = CVector(0.0f, 0.0f, 0.0f, 1.0f) * (pInstanceData->obObjectToWorld);

			float s = (vOpjPos * pContext->m_worldToView).Length();
			CVector Fex = -s * pContext->m_depthHazeBeta1PlusBeta2;
			Fex = CVector( expf(Fex.Y()), expf(Fex.Y()), expf(Fex.Z()), 0.0f );

			CDirection V = CDirection(pContext->m_eyePos) - CDirection(vOpjPos);
			V.Normalise();

			CVector sunDir = CVector(pContext->m_sunDirection) * (pInstanceData->obObjectToWorld.GetAffineInverse());
			float fCosTheta = V.Dot( CDirection(sunDir) );
			float fPhase1Theta = pContext->m_depthHazeA.X() + fCosTheta  * fCosTheta;

			float fPhase2Theta = 0;
			if ( fCosTheta > 0.0f )
			{
				fPhase2Theta = powf( fCosTheta, pContext->m_depthHazeG.Y() );
				fPhase2Theta = fPhase2Theta * pContext->m_depthHazeG.X() * pContext->m_depthHazeG.Z();
			}
			
			float ax = pContext->m_depthHazeA.X();
			CVector I = ( pContext->m_depthHazeBetaDash1 * fPhase1Theta) + ( pContext->m_depthHazeBetaDash2 * fPhase2Theta );
			CVector Scattering = ( I * pContext->m_depthHazeOneOverBeta1PlusBeta2 ) * pContext->m_depthHazeA.Z() * 
						( CVector( ax, ax, ax, 0.0f ) - Fex ) * pContext->m_depthHazeSunColour * pContext->m_depthHazeSunColour.W();

			Scattering.Max(CVector(0.0f, 0.0f, 0.0f, 0.0f));

			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &Scattering, 1);	
			break;
		}

	case PROPERTY_SPEEDTREE_WINDMATRICES:
	case PROPERTY_SPEEDTREE_LEAFCLUSTERS:
	case PROPERTY_SPEEDTREE_LEAFANGLES:
		{
			// nothing, this value are set by speedtree code...
		}
		break;

	default: // should never happen
		ntError_p( false, ("SPU Renderer - cannot set vertex property %i\n", pPatch->m_Semantic) );
		break; 
	}
}

// goes through all the patchs fixing up the push buffer passed in,
// if this pushbuffer requires a pixelshader relocate op return true
restrict Heresy_PushBufferPatch* PatchProperties( restrict Heresy_PushBuffer* pPB,
						const BatchRenderRenderContext* pContext,
						const RendererMaterialInstanceData* pInstance )
{
	restrict Heresy_PushBufferPatch* pRequirePSRelocate = 0;
	// we start from the bottom so that while we apply pixel shader patches we also can fetch in advance
	// bones indices and matrices.
	// dunno if there's enough time to cover mem latencies, but it's better than nothing for sure (marco)
	restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (((uint32_t)pPB) + sizeof(Heresy_PushBuffer) + sizeof(Heresy_PushBufferPatch) * (pPB->m_iNumPatches - 1));

	for( uint16_t i = 0; i < pPB->m_iNumPatches;i++, pPatch-- )
	{
		switch( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) )
		{
		case HPBP_PIXEL_CONSTANT_FIXUP:
			FixupPixelShaderConstant( pPB, pPatch, pContext, pInstance );
			break;
		case HPBP_TEXTURE_FIXUP:
			FixupTexture( pPB, pPatch, pContext );
			break;
		case HPBP_VERTEX_CONSTANT_FIXUP:
			FixupVertexShaderConstant( pPB, pPatch, pContext, pInstance );
			break;
		case HPBP_PIXEL_SHADER_FIXUP:
			pRequirePSRelocate = pPatch;
			break;
		default: // should never happen
	//		ntAssert( false );
			break;
		} 
	}
 
	
	return pRequirePSRelocate;
}

#define MAX_PUSH_BUFFER_SIZE 20 * 1024
uint32_t g_PushBufferTempSpace[ (MAX_PUSH_BUFFER_SIZE / 4) * 2 ];
uint32_t g_PushBufferHeaderTempSpace[ (MAX_PUSH_BUFFER_SIZE / 4) * 2 ];
uint16_t g_iPushBufferBuffer;

void SwapPushBufferBuffers( const uint32_t* pOriginalPushBufferData, const uint32_t iPBSize, Heresy_PushBuffer* pHeader )
{
	g_iPushBufferBuffer ^= 1;
	uint16_t iOffset = g_iPushBufferBuffer * ((uint16_t)(MAX_PUSH_BUFFER_SIZE / 4));

	memcpy( &g_PushBufferTempSpace[ iOffset ], pOriginalPushBufferData, iPBSize );

	pHeader->m_pStart = &g_PushBufferTempSpace[ iOffset ];
	pHeader->m_pCurrent = pHeader->m_pStart + (iPBSize/4);
	pHeader->m_pEnd = pHeader->m_pStart + (MAX_PUSH_BUFFER_SIZE / 4);

}

void DoPushBufferGeneration(uint16_t iteration,
							restrict Heresy_PushBuffer* pPB,
							SPUPushBufferManagement* pBufferMan,
							const BatchRenderRenderContext* pContext,
							const RendererMaterialInstanceData* pInstance,
							const RendererData* pGlobalData,
							uint32_t &i32DestPixelsShaderMem)
{

	// if our instance is skinned let fecth bone indices and matrices in advance
	if ( (pInstance->BoneCount > 0) && (pInstance->pBoneIndices != NULL) && (pInstance->pSkinMatrixArray != NULL) )
	{
		// stall for indices
		ntDMA::StallForCompletion( g_bonesId[0] );
		// stall for matrices
		ntDMA::StallForCompletion( g_bonesId[1] );
	
		uint32_t iEAAddr;
		uint32_t iDMASize;
		ntDMA::Params dmaParams;

		iEAAddr = ((uint32_t)pInstance->pBoneIndices) & ~0xf;
		iDMASize = ((pInstance->BoneCount) & ~0xf) + 32;
		dmaParams.Init32( g_i32UnAlignedIndices, iEAAddr, iDMASize, g_bonesId[0] );

		// start fetching bone indices
		ntDMA::DmaToSPU( dmaParams );
	
		iEAAddr = ((uint32_t)pInstance->pSkinMatrixArray) & ~0xf;
		iDMASize = 200 * 3 * sizeof(CVector);
		dmaParams.Init32( g_Bones, iEAAddr, iDMASize, g_bonesId[1] );

		// start fetching bone matrices
		ntDMA::DmaToSPU( dmaParams );
	}


	restrict Heresy_PushBufferPatch* pRequirePSRelocate = 	PatchProperties( pPB, pContext, pInstance  );

	if( pRequirePSRelocate != 0 )
	{
		restrict Heresy_PushBufferPatch* pPatch = pRequirePSRelocate;
		// we need to do a custom fixup job, cos we need to use the EA address, the buffer should be dma'able 
		Heresy_Assert( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) == HPBP_PIXEL_SHADER_FIXUP );

		uint16_t iShaderSize = (pPatch->m_Semantic & HPBP_PIXEL_SHADER_SIZE_MASK)<<2;
		uint32_t* pSrcShader = ((uint32_t*)pPatch) + ((pPatch->m_iData & HPBP_DATA_MASK)>>HPBP_DATA_MASK_LENGTH);
		uint32_t* pAddrReg = pPB->m_pStart + pPatch->m_offset;
	#if _DEBUG
		Heresy_Assert( *pAddrReg == 0xFEFEFEFE );
	#endif
		*pAddrReg = Heresy_MainRamAddressToRSX( &pBufferMan->m_HeresyGlobal, (void*)i32DestPixelsShaderMem ) | HFRL_MAIN_XDDR;
 
		ntDMA::Params dmaParams;
		ntDMA_ID dmaId = ntDMA::GetFreshID();
		dmaParams.Init32( pSrcShader, i32DestPixelsShaderMem, iShaderSize, dmaId );

		ntDMA::DmaToPPU( dmaParams );
		// make sure last DMA has completely so its safe to re-use its buffer
		ntDMA::StallForCompletion( dmaId );
		ntDMA::FreeID( dmaId ); 

		// update and align pixel shaders mem pointer (pixel shaders alignment is 64 bytes )
		i32DestPixelsShaderMem += iShaderSize;
		i32DestPixelsShaderMem = (i32DestPixelsShaderMem + (0x40 - 1)) & (~(0x40 - 1));
	}

	if ( pGlobalData->renderingPass == kRendPassShadowMap )
	{
		uint32_t flag = (1<<2); //RFF_CAST_SHADOWMAP0 (can't include renderable.h here)
		uint32_t FrameFlags = pInstance->ui32FrameFlag;
		for( uint16_t i=0;i < 4;i++, flag<<=1 )
		{
			if( FrameFlags & flag )
			{
				ntAssert( pPB->m_iMarker[i] != 0xFFFF );

				uint32_t* pPatchPB = pPB->m_pStart + pPB->m_iMarker[i];
				CMatrix mat = pInstance->obObjectToWorld * pContext->m_shadowMapProjection[i];
				Heresy_Set32bit( pPatchPB, Heresy_Cmd( RSX_NOOP, 0) | HPBC_NOINCREMENT );
				pPatchPB += (1 + 2); // skip NOP + skip set vertex constant command and constant num
				Heresy_CopyN128bits( pPatchPB, &mat, 4 );
				pPatchPB += (16 + 1); // skip 4 quad words + set scissor rect command
				uint16_t iX = pContext->m_shadowScissor[i].left;
				uint16_t iY = pContext->m_shadowScissor[i].top;
				uint16_t iWidth = pContext->m_shadowScissor[i].right - pContext->m_shadowScissor[i].left;
				uint16_t iHeight = pContext->m_shadowScissor[i].bottom - pContext->m_shadowScissor[i].top;
				*pPatchPB = Heresy_ScissorHorizontal( iX, iWidth );
				*(pPatchPB+1) = Heresy_ScissorVertical( iY, iHeight );
			}
		}
	} 
}


//
//	Entry point for the code on this SPU.
//
extern "C" void SpuMain( SPUArgumentList &params )
{
	//ntBreakpoint();

	INSERT_PA_BOOKMARK ( PABM_SPU_RENDERER );

	// A custom context structure
	GetArrayInput( BatchRenderRenderContext*, pContext, kRendContext );
	GetArrayInput( SPUPushBufferManagement*, pPushBufferMan, kRendPushBufManagement );
	GetArrayInput( RendererData*, pGlobalData, kRendGlobalData );
	GetArrayInput( RendererMaterialInstanceData*, pMeshData, kRendMeshesData );

	// we need to double buffer push-buffers for DMA purposes (should be enough buffers to stop stalling...)
	// cos I'm lazy and my profiles seem to indicate that we don't have pushbuffer bigger than 9K including fixups
	// I'm using a max 16K push-buffer size.
	g_iPushBufferBuffer = 1;

	// for now we will just build complete push-buffers for each instance, this is an obvious waste
	// particular of RAM which is very precious for the SPU... in future we will split it into 2
	// material data and instance specific data... it may actually be advantages for armies at least
	// to do more work on GPU (say extra matrix multipys) to reduce instance data/gpu pushs...

	// this is streaming pushbuffer generation
	// there are two streams, the pushbuffer itself and the pixel shader temp space (PSTP)
	// the pre-fixed up pushbuffer (PFPB) is dma'ed in LS, this includes the pre-fixed up pixel shader (PFPS)
	// fixup occurs both to the PFPB and the PFPS
	
	uint16_t iNumMeshes = (uint16_t) pGlobalData->meshCount;
	ntAssert( iNumMeshes != 0 );

	ntDMA_ID outId[2], inId[2][2];
	outId[0] = ntDMA::GetFreshID();
	outId[1] = ntDMA::GetFreshID();
	inId[0][0] = ntDMA::GetFreshID();
	inId[0][1] = ntDMA::GetFreshID();
	inId[1][0] = ntDMA::GetFreshID();
	inId[1][1] = ntDMA::GetFreshID();
	g_bonesId[0] = ntDMA::GetFreshID();
	g_bonesId[1] = ntDMA::GetFreshID();

	// last time round the loop is a special case (we have to add the ret
	uint32_t i32DestPushBuffAdd = (uint32_t)pGlobalData->pDestPixelShaders;
	for( uint16_t i=0; i < iNumMeshes;i++)
	{
		ntDMA::Params dmaParams, dmaParams2;

		uint16_t iOffset = g_iPushBufferBuffer * ((uint16_t)(MAX_PUSH_BUFFER_SIZE / 4));
		dmaParams.Init32( &g_PushBufferHeaderTempSpace[ iOffset ] , (uint32_t)pMeshData[i].pPushBufferHeader, pMeshData[i].ui32PushBufferHeaderSize, inId[0][g_iPushBufferBuffer] );
		ntDMA::DmaToSPU( dmaParams );
		uint32_t alignedPBSize = (pMeshData[i].ui32PushBufferSize + 0xF) & ~0xF;
		dmaParams2.Init32( &g_PushBufferTempSpace[ iOffset ] , (uint32_t)pMeshData[i].pPushBuffer, alignedPBSize, inId[1][g_iPushBufferBuffer] );
		ntDMA::DmaToSPU( dmaParams2 );

		ntDMA::StallForCompletion( inId[0][g_iPushBufferBuffer] ); 
		ntDMA::StallForCompletion( inId[1][g_iPushBufferBuffer] ); 

		Heresy_PushBuffer* pPushBufferHeader = (Heresy_PushBuffer*)&g_PushBufferHeaderTempSpace[ iOffset ];
		pPushBufferHeader->m_pStart = &g_PushBufferTempSpace[ iOffset ];
		pPushBufferHeader->m_pCurrent = pPushBufferHeader->m_pStart + (pMeshData[i].ui32PushBufferSize >> 2);
		pPushBufferHeader->m_pEnd = pPushBufferHeader->m_pStart + (MAX_PUSH_BUFFER_SIZE / 4);

		// we use a double buffering scheme.. we start stalling for the second last DMA transfer to be complete
		ntDMA::StallForCompletion( outId[g_iPushBufferBuffer^1] ); 
		// then swap our buffer
//		SwapPushBufferBuffers( pPushBuffer, iPBSize, pPushBufferHeader );
		
		// generate push buffer
		DoPushBufferGeneration( i, pPushBufferHeader, pPushBufferMan, pContext, &pMeshData[i], pGlobalData, i32DestPushBuffAdd);

		// to make life easier and DMAs fast we align the push buffer to quadword
		Heresy_AlignPushBuffer( pPushBufferHeader, 16 ); // quadword alignment

		// Dump this instance push buffer into main push buffer
		uint32_t* pDMAOut = pPushBufferHeader->m_pStart;
		uint32_t iDMASize = pPushBufferHeader->m_pCurrent - pPushBufferHeader->m_pStart;
  		uint32_t iEAAddr = (uint32_t) pPushBufferMan->m_pOutPushBuffer.m_pCurrent;
		pPushBufferMan->m_pOutPushBuffer.m_pCurrent += iDMASize;

		dmaParams.Init32( pDMAOut, iEAAddr, iDMASize<<2, outId[g_iPushBufferBuffer] );
		ntDMA::DmaToPPU( dmaParams );

		g_iPushBufferBuffer ^= 1;
	}
	
	ntDMA::StallForCompletion( outId[1] );
	ntDMA::StallForCompletion( outId[0] ); 
	ntDMA::FreeID( outId[1] );
	ntDMA::FreeID( outId[0] );  
	ntDMA::FreeID( inId[0][0] );
	ntDMA::FreeID( inId[0][1] );
	ntDMA::FreeID( inId[1][0] );
	ntDMA::FreeID( inId[1][1] );
	ntDMA::FreeID( g_bonesId[1] ); 
	ntDMA::FreeID( g_bonesId[0] );  

	// SPU Management is an input output structure so should be put back into main ram 
}
