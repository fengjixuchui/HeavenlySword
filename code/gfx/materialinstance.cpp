/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/materialinstance.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/renderstates.h"
#include "anim/hierarchy.h"
#include "gfx/shadowsystem.h"
#include "gfx/depthhazeconsts.h"
#include "gfx/pictureinpicture.h"

#include "core/timer.h"
#include "core/gatso.h"

/***************************************************************************************************
*
*	FUNCTION		CMaterialPropertyIterator::CMaterialPropertyIterator
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

CMaterialPropertyIterator::CMaterialPropertyIterator( CMaterialProperty const* pobMaterialProperties, 
													  int iPropertyCount )
  : m_pobMaterialProperties( pobMaterialProperties ), 
	m_iNumberOfProperties( iPropertyCount ), 
	m_iPropertyIndex( 0 )
{
}


/***************************************************************************************************
*
*	FUNCTION		CMaterialPropertyIterator::NextProperty
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

CMaterialProperty const* CMaterialPropertyIterator::NextProperty( int iSemanticTag )
{
	// early out if null
	if( !m_pobMaterialProperties )
		return 0;

	// otherwise continue
	while( m_iPropertyIndex < m_iNumberOfProperties )
	{
		int const iPropertyTag = m_pobMaterialProperties[m_iPropertyIndex].m_iPropertyTag;
		
		// return the property if found
		if( iPropertyTag == iSemanticTag )
			return &m_pobMaterialProperties[m_iPropertyIndex];

		// stop looking if we've gone too far
		if( iPropertyTag > iSemanticTag )
			return 0;

		// advance
		++m_iPropertyIndex;
	}

	// no property found
	return 0;
}

/***************************************************************************************************
*
*	FUNCTION		CMaterialInstance::CMaterialInstance
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

CMaterialInstance::CMaterialInstance( CMaterial const* pobMaterial, 
									  CMeshVertexElement * pobVertexElements, 
									  int iVertexElementCount )
  : MaterialInstanceBase(false),
	m_pobMaterial( pobMaterial )
{
#ifdef USE_VERTEX_SHADER_CONSTANT_CACHE
	m_uiVertexCacheTick = 0;
#endif

	// order is important, the function SetVertexElement set the value of m_eBoundType
	SetVertexElement(pobVertexElements,iVertexElementCount);
	ntError_p( m_pobMaterial, ( "material instance must have non-null material" ) );
	BindShaders(m_eBoundType);
	m_bIsAlphaBlended = pobMaterial->IsAlphaBlended();
}
/***************************************************************************************************
*
*	FUNCTION		CMaterialInstance::CMaterialInstance
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

CMaterialInstance::CMaterialInstance( CMaterial const* pobMaterial )
	:	 MaterialInstanceBase(false),
		m_pobMaterial( pobMaterial )
{
#ifdef USE_VERTEX_SHADER_CONSTANT_CACHE
	m_uiVertexCacheTick = 0;
#endif

	m_bIsAlphaBlended = pobMaterial->IsAlphaBlended();
}

//-----------------------------------------------------
//!
//! UploadVertexShaderConstants
//! Send contents of cache to GPU
//!
//----------------------------------------------------
void CMaterialInstance::UploadVertexShaderConstants() const
{
#ifdef USE_VERTEX_SHADER_CONSTANT_CACHE

	CGatso::Start( "CMaterialInstance::UploadVertexShaderConstants" );
	if( m_iMinRegister < 0xFFFF )
	{
		Renderer::Get().SetVertexShaderConstant( m_iMinRegister, m_fVertexConstantCache+(m_iMinRegister*4), m_iMaxRegister-m_iMinRegister );
	}
	if( m_iMinSkinRegister < 0xFFFF )
	{
		Renderer::Get().SetVertexShaderConstant( m_iMinSkinRegister, m_fVertexConstantCache+(m_iMinSkinRegister*4), m_iNumSkinRegisters );
	}
	CGatso::Stop( "CMaterialInstance::UploadVertexShaderConstants" );

#endif // USE_VERTEX_SHADER_CONSTANT_CACHE
}

//-----------------------------------------------------
//!
//! SetVSConst
//! Buffer VS constants in a cache, or send them to the GPU
//!
//----------------------------------------------------
void CMaterialInstance::SetVSConst( Shader* pVertexShader, int iRegister, const void* pData, int iQWStorage ) const
{
	UNUSED(pVertexShader);
	CGatso::Start( "CMaterialInstance::SetVSConst" );

#ifdef USE_VERTEX_SHADER_CONSTANT_CACHE
	
	NT_MEMCPY( m_fVertexConstantCache + (iRegister*4), reinterpret_cast<const float*>(pData), iQWStorage*4*sizeof(float) );
	m_iMaxRegister = max(m_iMaxRegister, iRegister+iQWStorage );
	m_iMinRegister = min(m_iMinRegister, iRegister );

#else // USE_VERTEX_SHADER_CONSTANT_CACHE
	
	#ifdef PLATFORM_PC
	
	Renderer::Get().SetVertexShaderConstant( iRegister, pData, iQWStorage );
	
	#elif defined(PLATFORM_PS3)

	// NOTE! iRegister is probably NOT a real register number, but rather
	// an index into an array of bound constants in the shader. It MUST
	// come from  GcShader::GetConstantIndex( FwHashedString("some_constant" ) );
	pVertexShader->SetVSConstant( (u32)iRegister, pData, iQWStorage );
	
	#endif

#endif // USE_VERTEX_SHADER_CONSTANT_CACHE

	CGatso::Stop( "CMaterialInstance::SetVSConst" );
}

//-----------------------------------------------------
//!
//! SetPSConst
//! Send Pixel shader constants to the GPU
//! NOTE! on PS3 this must be done before pixel shader is set
//!
//----------------------------------------------------
void CMaterialInstance::SetPSConst( Shader* pPixelShader, int iRegister, const void* pData, int iQWStorage ) const
{
	CGatso::Start( "CMaterialInstance::SetPSConst" );
	
#ifdef PLATFORM_PC
	
	UNUSED(pPixelShader);
	Renderer::Get().SetPixelShaderConstant( iRegister, pData, iQWStorage );
	
#elif defined(PLATFORM_PS3)

	// NOTE! iRegister is NOT a real register number, but rather
	// an index into an array of bound constants in the shader. It MUST
	// come from  GcShader::GetConstantIndex( FwHashedString("some_constant" ) );
	pPixelShader->SetPSConstant( (u32)iRegister, pData, iQWStorage );
	
#endif

	CGatso::Stop( "CMaterialInstance::SetPSConst" );
}

/***************************************************************************************************
*
*	FUNCTION		UploadSkinMatrices
*
*	DESCRIPTION		Helper function to upload skin matrices
*
***************************************************************************************************/

void CMaterialInstance::UploadSkinMatrices( Shader* pVertexShader, CHierarchy* pobHierarchy, int iBaseRegister ) const
{
#ifndef PLATFORM_PS3
	ntAssert( iBaseRegister == SKIN_START_REGISTER ); // fixed address for skinned array
#endif

	CGatso::Start("UploadSkinMatrices");
	// ensure the transforms are up to date on the hierarchy
	pobHierarchy->UpdateSkinMatrices();
	ntError_p( m_pucBoneIndices, ( "no bone indices set on skinned material instance" ) );

	static const int iMaxBones = 200;
	ntAssert( m_iNumberOfBonesUsed < iMaxBones );

#ifndef USE_VERTEX_SHADER_CONSTANT_CACHE
	CSkinMatrix aTempMatrix[iMaxBones];
#endif

	// upload only the matrices used by this mesh
	for(int iBone = 0; iBone < m_iNumberOfBonesUsed; ++iBone)
	{
		// get the matrix index
		int iMatrixIndex = m_pucBoneIndices[iBone];

		// get the matrix for the given index
		const CSkinMatrix* pobBone = &pobHierarchy->GetSkinMatrixArray()[iMatrixIndex];

#ifdef USE_VERTEX_SHADER_CONSTANT_CACHE
		NT_MEMCPY( m_fVertexConstantCache + ((iBaseRegister+(iBone*3))*4), 
					reinterpret_cast<const float*>(pobBone), 
					3*4*sizeof(float) );
#else
		aTempMatrix[iBone] = *pobBone;
#endif
	}

#ifdef USE_VERTEX_SHADER_CONSTANT_CACHE
	UNUSED(pVertexShader);
	ntAssert( m_iMinSkinRegister == 0xFFFF );
	m_iMinSkinRegister = iBaseRegister;
	m_iNumSkinRegisters = 3 * m_iNumberOfBonesUsed;
#else
	SetVSConst( pVertexShader, iBaseRegister, aTempMatrix, 3 * m_iNumberOfBonesUsed );
#endif

	CGatso::Stop("UploadSkinMatrices");

}

/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::LoadVertexProperty
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

void CMaterialInstance::LoadVertexProperty( Shader* pVertexShader,
											const SHADER_PROPERTY_BINDING* pstBinding, 
											const MATERIAL_DATA_CACHE& stCache ) const
{
	CGatso::Start("CMaterialInstance::LoadVertexProperty");

	// special case skinning
	if(pstBinding->eSemantic == PROPERTY_BLEND_TRANSFORMS )
	{
		// skin is cacheable
#ifdef USE_VERTEX_SHADER_CONSTANT_CACHE
		if( m_uiVertexCacheTick < RenderingContext::ms_uiRenderContextTick )		
#endif
		{
			UploadSkinMatrices(
				pVertexShader,
				stCache.pobTransform->GetParentHierarchy(), 
				pstBinding->iRegister 
			);
#ifdef USE_VERTEX_SHADER_CONSTANT_CACHE
			m_uiVertexCacheTick = RenderingContext::ms_uiRenderContextTick;
#endif
		}
		CGatso::Stop("CMaterialInstance::LoadVertexProperty");
		return;
	} else
	{
		ntAssert( pstBinding->iRegister < 64 );
	}


	switch( pstBinding->eSemantic )
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
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister, 
				temp);
		}
		break;

	case PROPERTY_POSITION_RECONST_MAT:
		SetVSConst(
			pVertexShader,
			pstBinding->iRegister, 
			stCache.obReconstructionMatrix, 
			pstBinding->iStorage
		);
		break;

	case PROPERTY_PROJECTION:
		SetVSConst(
			pVertexShader,
			pstBinding->iRegister, 
			stCache.obReconstructionMatrix * stCache.obObjectToWorld * RenderingContext::Get()->m_worldToScreen, 
			pstBinding->iStorage
		);
		break;

	case PROPERTY_PROJECTION_NORECONST_MAT:
		SetVSConst(
			pVertexShader,
			pstBinding->iRegister, 
			stCache.obObjectToWorld * RenderingContext::Get()->m_worldToScreen, 
			pstBinding->iStorage
		);
		break;


	case PROPERTY_WORLD_TRANSFORM:
	case PROPERTY_REFLECTANCE_MAP_TRANSFORM:
		SetVSConst( 
			pVertexShader,
			pstBinding->iRegister, 
			stCache.obObjectToWorld, 
			pstBinding->iStorage
		);
		break;

	case PROPERTY_FILL_SH_MATRICES:
		SetVSConst(
			pVertexShader,
			pstBinding->iRegister, 
			RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 
			pstBinding->iStorage
		);
		break;

	case PROPERTY_KEY_DIR_OBJECTSPACE:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister,
				RenderingContext::Get()->m_toKeyLight * stCache.obWorldToObject );
		}
		break;

	case PROPERTY_KEY_DIR_COLOUR:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister,
				RenderingContext::Get()->m_keyColour );
		}
		break;

	case PROPERTY_SHADOW_MAP_TRANSFORM:
		SetVSConst( 
			pVertexShader,
			pstBinding->iRegister, 
			stCache.obObjectToWorld * RenderingContext::Get()->m_shadowMapProjection[0], 
			pstBinding->iStorage
		);
		break;
	case PROPERTY_SHADOW_MAP_TRANSFORM1:
		SetVSConst( 
			pVertexShader,
			pstBinding->iRegister, 
			stCache.obObjectToWorld * RenderingContext::Get()->m_shadowMapProjection[1], 
			pstBinding->iStorage
		);
		break;
	case PROPERTY_SHADOW_MAP_TRANSFORM2:
		SetVSConst( 
			pVertexShader,
			pstBinding->iRegister, 
			stCache.obObjectToWorld * RenderingContext::Get()->m_shadowMapProjection[2], 
			pstBinding->iStorage
		);
		break;
	case PROPERTY_SHADOW_MAP_TRANSFORM3:
		SetVSConst( 
			pVertexShader,
			pstBinding->iRegister, 
			stCache.obObjectToWorld * RenderingContext::Get()->m_shadowMapProjection[3], 
			pstBinding->iStorage
		);
		break;
	case PROPERTY_VIEW_POSITION_OBJECTSPACE:
		{
			ntAssert( pstBinding->iStorage == 1 );			
			CPoint obWorldViewer = RenderingContext::Get()->GetEyePos();
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister,
				obWorldViewer * stCache.obWorldToObject );
		}
		break;

	case PROPERTY_GAME_TIME:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetVSConst( 
				pVertexShader,
				pstBinding->iRegister, 
				CVector( _R( CTimer::Get().GetGameTime() ), 0.0f, 0.0f, 0.0f ) );
		}
		break;
	case PROPERTY_VIEW_TRANSFORM:
		{
			SetVSConst( 
				pVertexShader,
				pstBinding->iRegister, 
				stCache.obObjectToWorld * RenderingContext::Get()->m_worldToView, 
				pstBinding->iStorage
			);
		}
		break;
	case PROPERTY_DEPTH_HAZE_CONSTS_A:
		{
			ntAssert( pstBinding->iStorage == 1 );
			const CVector obConstsA = CDepthHazeSetting::GetAConsts();
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister,
				obConstsA );
		}
		break;	
	case PROPERTY_DEPTH_HAZE_CONSTS_G:
		{
			ntAssert( pstBinding->iStorage == 1 );
			const CVector obConstsG = CDepthHazeSetting::GetGConsts();
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister, 
				obConstsG );
		}
		break;
	case PROPERTY_DEPTH_HAZE_BETA1PLUSBETA2:
		{
			ntAssert( pstBinding->iStorage == 1 );
			const CVector obBeta1PlusBeta2 = CDepthHazeSetting::GetBeta1PlusBeta2();
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister, 
				obBeta1PlusBeta2 );
		}
		break;

	case PROPERTY_DEPTH_HAZE_RECIP_BETA1PLUSBETA2:
		{
			ntAssert( pstBinding->iStorage == 1 );
			const CVector obRecipBeta1PlusBeta2 = CDepthHazeSetting::GetOneOverBeta1PlusBeta2();
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister, 
				obRecipBeta1PlusBeta2 );
		}
		break;
	case PROPERTY_DEPTH_HAZE_BETADASH1:
		{
			ntAssert( pstBinding->iStorage == 1 );
			const CVector obBetaDash1 = CDepthHazeSetting::GetBetaDash1();
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister, 
				obBetaDash1 );
		}
		break;
	case PROPERTY_DEPTH_HAZE_BETADASH2:
		{
			ntAssert( pstBinding->iStorage == 1 );
			const CVector obBetaDash2 = CDepthHazeSetting::GetBetaDash2();
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister,
				obBetaDash2 );
		}
		break;
	case PROPERTY_DEPTH_HAZE_KILL_POWER:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister,
				CVector(8.0f, 8.0f, 8.0f, 8.0f) );
		}
		break;
	case PROPERTY_SUN_DIRECTION_OBJECTSPACE:
		{
			ntAssert( pstBinding->iStorage == 1 );
			CDirection obLightInObject = CDirection(CDepthHazeSetting::GetSunDir()) * stCache.obWorldToObject;
			obLightInObject.Normalise();
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister,
				obLightInObject );
		}
		break;
	case PROPERTY_SUN_COLOUR:
		{
			ntAssert( pstBinding->iStorage == 1 );
			const CVector obSunSplit = CDepthHazeSetting::GetSunColour();
			SetVSConst(
				pVertexShader,
				pstBinding->iRegister,
				obSunSplit  );
		}
		break;
	case PROPERTY_HAZE_FRAGMENT_EXTINCTION:
		{	
			CVector vOpjPos = CVector(0.0f, 0.0f, 0.0f, 1.0f) * (stCache.obObjectToWorld);

			float s = (vOpjPos * RenderingContext::Get()->m_worldToView).Length();
			CVector Fex = -s * CDepthHazeSetting::GetBeta1PlusBeta2();
			Fex = CVector( expf(Fex.Y()), expf(Fex.Y()), expf(Fex.Z()), 0.0f );
		
			Fex.Max(CVector(0.0f, 0.0f, 0.0f, 0.0f));
			Fex.Min(CVector(1.0f, 1.0f, 1.0f, 0.0f));

			Fex = CVector(1.0f, 1.0f, 1.0f, 0.0f);
			SetVSConst(	pVertexShader,	pstBinding->iRegister,	Fex );				
			break;
		}
	case PROPERTY_HAZE_FRAGMENT_SCATTER:
		{	
			CVector vOpjPos = CVector(0.0f, 0.0f, 0.0f, 1.0f) * (stCache.obObjectToWorld);

			float s = (vOpjPos * RenderingContext::Get()->m_worldToView).Length();
				CVector Fex = -s * CDepthHazeSetting::GetBeta1PlusBeta2();
			Fex = CVector( expf(Fex.Y()), expf(Fex.Y()), expf(Fex.Z()), 0.0f );

			CDirection V = CDirection(RenderingContext::Get()->GetEyePos()) - CDirection(vOpjPos);
			V.Normalise();

			CVector sunDir = CVector(CDepthHazeSetting::GetSunDir()) * (stCache.obWorldToObject);
			float fCosTheta = V.Dot( CDirection(sunDir) );
			float fPhase1Theta = CDepthHazeSetting::GetAConsts().X() + fCosTheta  * fCosTheta;

			float fPhase2Theta = 0;
			if ( fCosTheta > 0.0f )
			{
				fPhase2Theta = powf( fCosTheta, CDepthHazeSetting::GetGConsts().Y() );
				fPhase2Theta = fPhase2Theta * CDepthHazeSetting::GetGConsts().X() * CDepthHazeSetting::GetGConsts().Z();
			}
			
			float ax = CDepthHazeSetting::GetAConsts().X();
			CVector I = ( CDepthHazeSetting::GetBetaDash1() * fPhase1Theta) + ( CDepthHazeSetting::GetBetaDash2() * fPhase2Theta );
			CVector Scattering = ( I * CDepthHazeSetting::GetOneOverBeta1PlusBeta2() ) * CDepthHazeSetting::GetAConsts().Z() * 
				( CVector( ax, ax, ax, 0.0f ) - Fex ) * CDepthHazeSetting::GetSunColour() * CDepthHazeSetting::GetSunColour().W();

			Scattering.Max(CVector(0.0f, 0.0f, 0.0f, 0.0f));
			SetVSConst(	pVertexShader,	pstBinding->iRegister,	Scattering );
			break;
		}
	case PROPERTY_SPEEDTREE_WINDMATRICES:
	case PROPERTY_SPEEDTREE_LEAFCLUSTERS:
	case PROPERTY_SPEEDTREE_LEAFANGLES:
	case PROPERTY_SPEEDTREE_ALPHATRESHOLD:
		{
			// nothing, this value are set by speedtree code...
		}
		break;

	case PROPERTY_ANISOTROPIC_FILTERING:
		{
			// nothing
		}
		break;

	default:
		ntError_p( false, ( "unknown material vertex property binding: %i", pstBinding->eSemantic ) );
	}
	CGatso::Stop("CMaterialInstance::LoadVertexProperty");

}

/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::LoadPixelProperty
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

void CMaterialInstance::LoadPixelProperty(	Shader* pPixelShader,
											const SHADER_PROPERTY_BINDING* pstBinding, 
											const MATERIAL_DATA_CACHE& stCache ) const
{
	UNUSED( stCache );
	switch( pstBinding->eSemantic )
	{

	case PROPERTY_EXPOSURE_AND_TONEMAP:
		{	
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_ExposureAndToneMapConsts );
			break;
		} 
	case PROPERTY_REFRACTION_WARP:
		{
			ntAssert( pstBinding->iStorage == 1 );
			float cameraWidth = RenderingContext::Get()->m_fScreenAspectRatio*tanf(0.5f*RenderingContext::Get()->m_pViewCamera->GetFOVAngle());
			float refractionWarp = 0.25f * cameraWidth;
			SetPSConst( pPixelShader, pstBinding->iRegister, CVector(refractionWarp, refractionWarp, refractionWarp, refractionWarp) );
		}
		break;

	case PROPERTY_DEPTH_HAZE_KILL_POWER:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, CVector(8.0f, 8.0f, 8.0f, 8.0f));
		}
		break;

	case PROPERTY_KEY_DIR_COLOUR:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_keyColour );
		}
		break;

	case PROPERTY_KEY_DIR_OBJECTSPACE:
		{
			ntAssert( pstBinding->iStorage == 1 );			
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_toKeyLight * stCache.obWorldToObject );
		}
		break;

	case PROPERTY_VIEW_POSITION_OBJECTSPACE:
		{
			ntAssert( pstBinding->iStorage == 1 );			
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->GetEyePos() * stCache.obWorldToObject );
		}
		break;

	case PROPERTY_KEY_DIR_WORLDSPACE:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_toKeyLight );
		}
		break;

	case PROPERTY_KEY_DIR_REFLECTANCESPACE:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_toKeyLight );
		}
		break;

	case PROPERTY_REFLECTANCE_MAP_COLOUR:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_reflectanceCol );
		}
		break;

	case PROPERTY_SHADOW_MAP_RESOLUTION:
		{
			ntAssert( pstBinding->iStorage == 1 );
			
			float fWidth = _R( RenderingContext::Get()->m_pShadowMap->GetWidth() );
			float fHeight = _R( RenderingContext::Get()->m_pShadowMap->GetHeight() );
			
			SetPSConst( pPixelShader, 
				pstBinding->iRegister, CVector( fWidth, fHeight, 1.0f / fWidth, 1.0f / fHeight ) );
		}
		break;
	case PROPERTY_SHADOW_PLANES:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, CVector( RenderingContext::Get()->m_shadowPlanes[1].W(),
																	  RenderingContext::Get()->m_shadowPlanes[2].W(),	
																	  RenderingContext::Get()->m_shadowPlanes[3].W(),
																	  0.0f )	);
		}
		break;
	case PROPERTY_SHADOW_PLANE0:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_shadowPlanes[0] );
		}
		break;
	case PROPERTY_SHADOW_PLANE1:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_shadowPlanes[1] );
		}
		break;
	case PROPERTY_SHADOW_PLANE2:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_shadowPlanes[2] );
		}
		break;
	case PROPERTY_SHADOW_PLANE3:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_shadowPlanes[3] );
		}
		break;
	case PROPERTY_SHADOW_PLANE4:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_shadowPlanes[4] );
		}
		break;
	case PROPERTY_SHADOW_RADII:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_shadowRadii[0] );
		}
		break;
	case PROPERTY_SHADOW_RADII1:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_shadowRadii[1] );
		}
		break;
	case PROPERTY_SHADOW_RADII2:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_shadowRadii[2] );
		}
		break;
	case PROPERTY_SHADOW_RADII3:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_shadowRadii[3] );
		}
		break;
	case PROPERTY_FILL_SH_MATRICES:
		SetPSConst( pPixelShader,
					pstBinding->iRegister, 
					RenderingContext::Get()->m_SHMatrices.m_aChannelMats, 
					pstBinding->iStorage );
		break;
	case PROPERTY_PARALLAX_SCALE_AND_BIAS:
		{
			ntAssert( pstBinding->iStorage == 1 );

			CVector scaleAndBias( CONSTRUCT_CLEAR );
			if (CRendererSettings::bUseParallaxMapping)
			{
				scaleAndBias.X() = 0.02f;
				scaleAndBias.Y() = -0.01f;
			}

			SetPSConst( pPixelShader, pstBinding->iRegister, scaleAndBias );
		}
		break;
	case PROPERTY_VPOS_TO_UV:
		{
			ntAssert( pstBinding->iStorage == 1 );

			CVector posToUV( CONSTRUCT_CLEAR );
			posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth()-1);
			posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight()-1);
			posToUV.Z() =  1.f / Renderer::Get().m_targetCache.GetWidth();
			posToUV.W() = -1.f / Renderer::Get().m_targetCache.GetHeight();

			SetPSConst( pPixelShader, pstBinding->iRegister, posToUV );
		}
		break;
	case PROPERTY_DEPTHOFFIELD_PARAMS:
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, RenderingContext::Get()->m_depthOfFieldParams );
		}
		break;
	case PROPERTY_SPEEDTREE_ALPHATRESHOLD:
		{
			// nothing, this value are set by speedtree code...
		}
		break;
	// hair badnesss, all this freq stuff should be removed
	// but I currently need to tweak them a bit...
	case PROPERTY_HAIR_BASETEX_FREQ:
		{
			ntAssert( pstBinding->iStorage == 1 );
			CVector tmp(5.0f,5.0f,0.0f,0.0f);
			SetPSConst( pPixelShader, pstBinding->iRegister, tmp );
			break;
		}
	case PROPERTY_HAIR_SHIFTTEX_FREQ:
		{
			ntAssert( pstBinding->iStorage == 1 );
			CVector tmp(5.0f,5.0f,0.0f,0.0f);
			SetPSConst( pPixelShader, pstBinding->iRegister, tmp );
			break;
		}
	case PROPERTY_HAIR_MASKTEX_FREQ:
		{
			ntAssert( pstBinding->iStorage == 1 );
			CVector tmp(5.0f,5.0f,0.0f,0.0f);
			SetPSConst( pPixelShader, pstBinding->iRegister, tmp );
			break;
		}


	// nothing. These are set later...
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
	
		break;

	case PROPERTY_ANISOTROPIC_FILTERING:
		ntAssert( pstBinding->iStorage == 1 );
		
		break;

	case PROPERTY_ALPHATEST_THRESHOLD:
		break;
	default:
		ntError_p( false, ( "unknown material pixel property binding: %i", pstBinding->eSemantic ) );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CMaterialInstance::GetMaterialProperty
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

const CMaterialProperty* const CMaterialInstance::GetMaterialProperty(int semantic) const
{
	ntstd::Map<int, const CMaterialProperty*>::const_iterator it = m_PropertiesMap.find(semantic);

	if (it != m_PropertiesMap.end())
	{
		return it->second;
	}
	
	return NULL;
}


/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::LoadTexture
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

void CMaterialInstance::LoadTexture( SHADER_TEXTURE_BINDING const* pstBinding, 
										 MATERIAL_DATA_CACHE const& stCache ) const
{
	UNUSED( stCache );
	switch( pstBinding->eSemantic )
	{
	case TEXTURE_SHADOW_MAP:
	case TEXTURE_SHADOW_MAP1:
	case TEXTURE_SHADOW_MAP2:
	case TEXTURE_SHADOW_MAP3:
		Renderer::Get().SetTexture( pstBinding->iStage, RenderingContext::Get()->m_pShadowMap );
		break;

	case TEXTURE_STENCIL_MAP:
		Renderer::Get().SetTexture( pstBinding->iStage, RenderingContext::Get()->m_pStencilTarget );
		break;

#ifdef PLATFORM_PS3
	case TEXTURE_IRRADIANCE_CACHE:
		Renderer::Get().SetTexture( pstBinding->iStage, RenderingContext::Get()->m_pIrradianceCache );
		break;
#endif

	case TEXTURE_REFLECTANCE_MAP:
#ifdef PLATFORM_PS3
		RenderingContext::Get()->m_reflectanceMap->m_Platform.GetTexture()->SetGammaCorrect( pstBinding->iGammaCorrection );
#endif
		Renderer::Get().SetTexture( pstBinding->iStage, RenderingContext::Get()->m_reflectanceMap );
		break;

#ifdef PLATFORM_PS3
	case TEXTURE_DIFFUSE1:
	case TEXTURE_DIFFUSE2:
		Renderer::Get().SetTexture( pstBinding->iStage, RenderingContext::Get()->m_pStencilTarget );
		break;
#endif


	default:
		ntError_p( false, ( "unknown material texture binding(%i)", pstBinding->eSemantic ) );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::UnBindProperties
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

void CMaterialInstance::UnBindProperties( Shader* pVertexShader, Shader* pPixelShader ) const
{
	UNUSED( pVertexShader );

	// clean up texture stages
	for(int iBinding = 0; iBinding < pPixelShader->GetNumTextureBindings(); ++iBinding)
	{
		const SHADER_TEXTURE_BINDING* pstBinding = pPixelShader->GetTextureBinding(iBinding);
		Renderer::Get().SetTexture( pstBinding->iStage, Texture::NONE );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::CGameMaterialInstance
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

CGameMaterialInstance::CGameMaterialInstance( CMaterial const* pobMaterial, 
											  CMeshVertexElement * pobVertexElements, 
											  int iVertexElementCount, 
											  CMaterialProperty const* pobProperties, 
											  int iPropertyCount )
  : CMaterialInstance( pobMaterial, pobVertexElements, iVertexElementCount ), 
	m_iTechniqueIndex( 0 )
{
	SetPropertyTable(pobProperties,iPropertyCount);
}

/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::CGameMaterialInstance
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

CGameMaterialInstance::CGameMaterialInstance( CMaterial const* pobMaterial )
	:	CMaterialInstance( pobMaterial ),
		m_iTechniqueIndex( 0 )
{

}

/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::GetTechniqueIndex
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

int CGameMaterialInstance::GetTechniqueIndex( bool bReceiveShadow )
{

	// compute the technique index
	int iTechnique = 0, iPower = 1;

	iTechnique += iPower*( ( CShadowSystemController::Get().IsShadowMapActive() &&  bReceiveShadow) ? 1 : 0 );
	iPower *= 2; // disabled/recieve screen aligned shadow map

	iTechnique += iPower * ( CRendererSettings::bEnableDepthHaze ? 1 : 0 );
	iPower *= 2; // enabled/disabled

	return iTechnique;
}
