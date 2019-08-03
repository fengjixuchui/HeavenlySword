/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/materialinstance.h"
#include "core/gatso.h"
#include "gfx/rendercontext.h"
#include "gfx/renderer.h"
#include "anim/transform.h"

/***************************************************************************************************
*
*	FUNCTION		CMaterialInstance::BindShaders
*
*	DESCRIPTION		takes a type to use
*
***************************************************************************************************/

void CMaterialInstance::BindShaders(enum  VERTEXSHADER_TRANSFORM_TYPE type, unsigned int* puiStreamElements )
{
	// argument check
	ntError_p( m_pobMaterial, ( "material instance must have non-null material" ) );

	m_eBoundType = type;

	// bind each pass by computing a declaration for it
	m_pobDeclarations.Reset( NT_NEW_CHUNK(Mem::MC_GFX) CVertexDeclaration[m_pobMaterial->GetNumTechniques()] );

	// bind each technique
	for(int iBinding = 0; iBinding < m_pobMaterial->GetNumTechniques(); ++iBinding)
	{
		const TECHNIQUE* const pstTechnique = m_pobMaterial->GetTechnique(iBinding);
		ntAssert( pstTechnique->GetColourVertexShader(m_eBoundType) );
		ntAssert( pstTechnique->GetShadowVertexShader(m_eBoundType) );

		D3DVERTEXELEMENT9 astFormat[17];
		memset(&astFormat[0], 0, 17*sizeof(D3DVERTEXELEMENT9));
		
		// it is currently assumed that the stream binding for colour is a superset of what the shadows want
		if( BindStreams(
			pstTechnique->GetColourVertexShader(m_eBoundType)->GetStreamBinding(0), 
			pstTechnique->GetColourVertexShader(m_eBoundType)->GetNumStreamBindings(), 
			m_pVertexElements, 
			m_iVertexElementCount, 
			puiStreamElements,
			astFormat
		) )
		{
			m_pobDeclarations[iBinding] = CVertexDeclarationManager::Get().GetDeclaration( &astFormat[0] );
		}
		else
			ntError_p( false, ( __FUNCTION__": failed to bind material to mesh header" ) );
	}
}

/***************************************************************************************************
//
// Setups all the properties moved to Material instance so that the shadow material can access it
//
***************************************************************************************************/
void CMaterialInstance::BindProperties( Shader* pVertexShader, Shader* pPixelShader, MATERIAL_DATA_CACHE const& stCache ) const
{
	CGatso::Start( "CMaterialInstance::UploadVSConsts" );

	if( m_uiVertexCacheTick < RenderingContext::ms_uiRenderContextTick )		
		ResetVertexShaderConstantCache();

	// get the material properties
	CMaterialPropertyIterator obMaterialProperties( GetPropertyTable(), GetPropertyTableSize());

	// upload vertex shader properties
	for(int iBinding = 0; iBinding < pVertexShader->GetNumPropertyBindings(); ++iBinding)
	{
		const SHADER_PROPERTY_BINDING* pstBinding = pVertexShader->GetPropertyBinding(iBinding);

		// get to the property for this binding
		CMaterialProperty const* pobProperty = obMaterialProperties.NextProperty( pstBinding->eSemantic );
		if( pobProperty )
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetVSConst( pVertexShader, pstBinding->iRegister, pobProperty->m_uData.stFloatData.afFloats, 1 );
		}
		else
			LoadVertexProperty( pVertexShader, pstBinding, stCache );
	}

	// reset the properties iterator for the pixel program data
	obMaterialProperties.Reset();

	UploadVertexShaderConstants();
	CGatso::Stop( "CMaterialInstance::UploadVSConsts" );

	CGatso::Start( "CMaterialInstance::UploadPSConsts" );

	// upload pixel shader properties
	for(int iBinding = 0; iBinding < pPixelShader->GetNumPropertyBindings(); ++iBinding)
	{
		const SHADER_PROPERTY_BINDING* pstBinding = pPixelShader->GetPropertyBinding(iBinding);

		// get to the property for this binding
		CMaterialProperty const* pobProperty = obMaterialProperties.NextProperty( pstBinding->eSemantic );
		if( pobProperty )
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, pobProperty->m_uData.stFloatData.afFloats, 1 );
		}
		else
			LoadPixelProperty( pPixelShader, pstBinding, stCache );
	}
	
	CGatso::Stop( "CMaterialInstance::UploadPSConsts" );

	CGatso::Start( "CMaterialInstance::BindTextures" );

	// bind the textures
	for(int iBinding = 0; iBinding < pPixelShader->GetNumTextureBindings(); ++iBinding)
	{
		const SHADER_TEXTURE_BINDING* pstBinding = pPixelShader->GetTextureBinding(iBinding);

		// get to the property for this binding
		CMaterialProperty const* pobProperty = obMaterialProperties.NextProperty( pstBinding->eSemantic );
		if( !pobProperty )
			LoadTexture( pstBinding, stCache );
		else
			Renderer::Get().m_Platform.SetTexture( pstBinding->iStage, pobProperty->m_uData.stTextureData.pobTexture, false );

		// set the stage state
		Renderer::Get().SetSamplerAddressMode(pstBinding->iStage, pstBinding->iAddressMode);
		Renderer::Get().SetSamplerFilterMode(pstBinding->iStage, pstBinding->iFilterMode);
	}
	CGatso::Stop( "CMaterialInstance::BindTextures" );

}


void CGameMaterialInstance::PreRender( Transform const* pobTransform, bool bRecieveShadows, void const* pDecompMatrix ) const
{
	CGatso::Start( "CGameMaterialInstance::BindShaders" );
	UNUSED(pDecompMatrix);

	// get the technique index and bind it
	m_iTechniqueIndex = GetTechniqueIndex( bRecieveShadows );

	// cache some properties
	MATERIAL_DATA_CACHE stCache;
	if( pobTransform != 0 )
	{
		stCache.pobTransform = pobTransform;
		stCache.obObjectToWorld = pobTransform->GetWorldMatrixFast();
		stCache.obWorldToObject = stCache.obObjectToWorld.GetAffineInverse();
	} else
	{
		stCache.pobTransform = 0;
		stCache.obObjectToWorld.SetIdentity();
		stCache.obWorldToObject.SetIdentity();
	}
	stCache.obReconstructionMatrix.SetIdentity();

	// find the shaders
	Shader* pVertexShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->GetColourVertexShader(m_eBoundType);
	Shader* pPixelShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->GetColourPixelShader();

#if defined(_DEBUG)
	if( 
		((pVertexShader->GetModel() == SHADERMODEL_3_0) && (pPixelShader->GetModel() != SHADERMODEL_3_0)) ||
		((pVertexShader->GetModel() != SHADERMODEL_3_0) && (pPixelShader->GetModel() == SHADERMODEL_3_0))
		)
	{
		ntAssert_p(0, ("Both shaders must be version 3.0 if any one is...") );
	}
#endif

	// bind the shaders and declaration
	Renderer::Get().SetVertexShader( pVertexShader );
	Renderer::Get().SetPixelShader( pPixelShader );

#ifdef PLATFORM_PC
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pobDeclarations[m_iTechniqueIndex] );
#endif

	CGatso::Stop( "CGameMaterialInstance::BindShaders" );

	BindProperties( pVertexShader, pPixelShader, stCache );
}



void CGameMaterialInstance::PostRender() const
{
	// find the shaders
	Shader* pVertexShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->GetColourVertexShader(m_eBoundType);
	Shader* pPixelShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->GetColourPixelShader();

	UnBindProperties( pVertexShader, pPixelShader );
}

/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::PreRenderDepths
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

void CGameMaterialInstance::PreRenderDepth( Transform const* pobTransform, bool bShadowProject, void const* pDecompMatrix ) const
{
	CGatso::Start( "CGameMaterialInstance::BindShaders" );
	UNUSED(pDecompMatrix);

	// get the technique index and bind it
	m_iTechniqueIndex = GetTechniqueIndex( true );

	// cache some properties
	MATERIAL_DATA_CACHE stCache;
	if( pobTransform != 0 )
	{
		stCache.pobTransform = pobTransform;
		stCache.obObjectToWorld = pobTransform->GetWorldMatrixFast();
		stCache.obWorldToObject = stCache.obObjectToWorld.GetAffineInverse();
	} else
	{
		stCache.pobTransform = 0;
		stCache.obObjectToWorld.SetIdentity();
		stCache.obWorldToObject.SetIdentity();
	}
	stCache.obReconstructionMatrix.SetIdentity();

	// find the shaders
	Shader* pVertexShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->GetDepthVertexShader(m_eBoundType);
	Shader* pPixelShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->apobShaders[ SHADERSLOT_OUTPUTDEPTH_PIXEL ];

	// bind the shaders and declaration
	Renderer::Get().SetVertexShader( pVertexShader );
	Renderer::Get().SetPixelShader( pPixelShader );

#ifdef PLATFORM_PC
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pobDeclarations[m_iTechniqueIndex] );
#endif

	CGatso::Stop( "CGameMaterialInstance::BindShaders" );

	// my really nasty hack has moved but this isn't exactly nice either...
	if( bShadowProject )
	{
		if( GetMaterial()->IsAlphaBlended() || GetMaterial()->IsAlphaTested() )
		{
			Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );
		}
	}

	BindProperties( pVertexShader, pPixelShader, stCache );
}

void CGameMaterialInstance::PostRenderDepth( bool bShadowProject ) const
{
	// find the shaders
	Shader* pVertexShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->GetDepthVertexShader(m_eBoundType);
	Shader* pPixelShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->apobShaders[ SHADERSLOT_OUTPUTDEPTH_PIXEL ];

	UnBindProperties( pVertexShader, pPixelShader );

	if( bShadowProject )
	{
		if( GetMaterial()->IsAlphaBlended() || GetMaterial()->IsAlphaTested() )
		{
//			Renderer::Get().SetCullMode( GFX_CULLMODE_REVERSED );
			Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
		}
	}
}



/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::RenderShadow
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

void CGameMaterialInstance::PreRenderShadowRecieve( Transform const* pobTransform, void const* pDecompMatrix ) const
{
	CGatso::Start( "CGameMaterialInstance::BindShaders" );
	UNUSED(pDecompMatrix);

	// get the technique index and bind it
	m_iTechniqueIndex = GetTechniqueIndex( true );

	// cache some properties
	MATERIAL_DATA_CACHE stCache;
	if( pobTransform != 0 )
	{
		stCache.pobTransform = pobTransform;
		stCache.obObjectToWorld = pobTransform->GetWorldMatrixFast();
		stCache.obWorldToObject = stCache.obObjectToWorld.GetAffineInverse();
	} else
	{
		stCache.pobTransform = 0;
		stCache.obObjectToWorld.SetIdentity();
		stCache.obWorldToObject.SetIdentity();
	}
	stCache.obReconstructionMatrix.SetIdentity();

	// find the shaders
	Shader* pVertexShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->GetShadowVertexShader(m_eBoundType);
	Shader* pPixelShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->GetShadowPixelShader();

	// bind the shaders and declaration
	Renderer::Get().SetVertexShader( pVertexShader );
	Renderer::Get().SetPixelShader( pPixelShader );

#ifdef PLATFORM_PC
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pobDeclarations[m_iTechniqueIndex] );
#endif

	CGatso::Stop( "CGameMaterialInstance::BindShaders" );

	BindProperties( pVertexShader, pPixelShader, stCache );
}

void CGameMaterialInstance::PostRenderShadowRecieve() const
{
	// find the shaders
	Shader* pVertexShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->GetShadowVertexShader(m_eBoundType);
	Shader* pPixelShader = GetMaterial()->GetTechnique( m_iTechniqueIndex )->GetShadowPixelShader();
	UnBindProperties( pVertexShader, pPixelShader );
}

