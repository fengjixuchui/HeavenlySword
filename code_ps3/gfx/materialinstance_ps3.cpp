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
#include "heresy/heresy_capi.h"
#include "anim/hierarchy.h" 

/***************************************************************************************************
*
*	FUNCTION		CMaterialInstance::BindShaders
*
*	DESCRIPTION		takes a type to use

***************************************************************************************************/

void CMaterialInstance::BindShaders(enum  VERTEXSHADER_TRANSFORM_TYPE type, unsigned int* puiStreamElements )
{
	UNUSED( type );
	UNUSED( puiStreamElements );
	// AFAIK this function is simple not needed on PS3 as the stream buffer hold the declerations as well...
}


/***************************************************************************************************
*
*	FUNCTION		GetAnisotropicFilterLevel
*
*	DESCRIPTION		do you need a description?

***************************************************************************************************/

int GetAnisotropicFilterLevel( const CMaterialInstance* pMaterial, int semantic )
{
	const CMaterialProperty *const pobProperty = pMaterial->GetMaterialProperty( PROPERTY_ANISOTROPIC_FILTERING ); 
	int filterLevel = 0;
	
	if ( pobProperty )
	{
		switch( semantic )
		{
			case TEXTURE_DIFFUSE0:
			case TEXTURE_DIFFUSE1:
			case TEXTURE_DIFFUSE2:
			case TEXTURE_HDR_DIFFUSE0:
			case TEXTURE_NORMAL_MAP:
			case TEXTURE_BSSKIN_BASE_NORMAL_OCCLUSION:
			case TEXTURE_BSSKIN_WRINKLE_NORMAL_OCCLUSION:
				filterLevel = int( pobProperty->GetFloatData().afFloats[0] );
		}
	}

	return filterLevel;
}

/***************************************************************************************************
//
// Setups all the properties moved to Material instance so that the shadow material can access it
//
***************************************************************************************************/
void CMaterialInstance::BindProperties( Shader* pVertexShader, Shader* pPixelShader, MATERIAL_DATA_CACHE const& stCache ) const
{
	CGatso::Start( "CMaterialInstance::UploadVSConsts" );
 
	// get the material properties
	//CMaterialPropertyIterator obMaterialProperties( GetPropertyTable(), GetPropertyTableSize());

	// upload vertex shader properties
	for(int iBinding = 0; iBinding < pVertexShader->GetNumPropertyBindings(); ++iBinding)
	{
		const SHADER_PROPERTY_BINDING* pstBinding = pVertexShader->GetPropertyBinding(iBinding);

		// get to the property for this binding
		//CMaterialProperty const* pobProperty = obMaterialProperties.NextProperty( pstBinding->eSemantic );
		const CMaterialProperty *const pobProperty = this->GetMaterialProperty(pstBinding->eSemantic);
		if( pobProperty )
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetVSConst( pVertexShader, pstBinding->iRegister, pobProperty->GetFloatData().afFloats, 1 );
		}
		else
			LoadVertexProperty( pVertexShader, pstBinding, stCache );
	}

	// reset the properties iterator for the pixel program data
	//obMaterialProperties.Reset();

	UploadVertexShaderConstants();
	CGatso::Stop( "CMaterialInstance::UploadVSConsts" );

	CGatso::Start( "CMaterialInstance::UploadPSConsts" );

	// upload pixel shader properties
	for(int iBinding = 0; iBinding < pPixelShader->GetNumPropertyBindings(); ++iBinding)
	{
		const SHADER_PROPERTY_BINDING* pstBinding = pPixelShader->GetPropertyBinding(iBinding);

		// get to the property for this binding
		//CMaterialProperty const* pobProperty = obMaterialProperties.NextProperty( pstBinding->eSemantic );
		CMaterialProperty const* pobProperty  = GetMaterialProperty(pstBinding->eSemantic);
		if( pobProperty )
		{
			ntAssert( pstBinding->iStorage == 1 );
			SetPSConst( pPixelShader, pstBinding->iRegister, pobProperty->GetFloatData().afFloats, 1 );
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
		//CMaterialProperty const* pobProperty = obMaterialProperties.NextProperty( pstBinding->eSemantic );
		CMaterialProperty const* pobProperty  = GetMaterialProperty(pstBinding->eSemantic);

		if( !pobProperty )
			LoadTexture( pstBinding, stCache );
		else
		{
			const GcTextureHandle& handle = pobProperty->GetTextureData().pobTexture->m_Platform.GetTexture();
			Gc::TexFormat textureFormat = handle->GetFormat();
			if (textureFormat == Gc::kTexFormatRGBA16F	||
				textureFormat == Gc::kTexFormatRGBA32F	||
				textureFormat == Gc::kTexFormatGR16F	||
				textureFormat == Gc::kTexFormatR32F	)
				handle->SetGammaCorrect( 0 );
			else
				handle->SetGammaCorrect( pstBinding->iGammaCorrection );
			Renderer::Get().SetTexture( pstBinding->iStage, pobProperty->GetTextureData().pobTexture );
		}
		
		// override desired anisotropic filtering level for this binding. Note that this is only a suggestion. It's up
		// to Renderer to set it
		int anisotropicFilterLevel = GetAnisotropicFilterLevel(this, pstBinding->eSemantic);

		// set the stage state
		// PS3_NOTE! do we need to convert pstBinding->iAddressMode or is the 
		// exporter changed to use the new enum lists ?
		Renderer::Get().SetSamplerAddressMode(pstBinding->iStage, pstBinding->iAddressMode);
		Renderer::Get().SetSamplerFilterMode(pstBinding->iStage, pstBinding->iFilterMode);
		
		// don't bother setting if not present
		if ( anisotropicFilterLevel )
            Renderer::Get().SetAnisotropicFilterLevel( pstBinding->iStage, anisotropicFilterLevel );
		
	}
	CGatso::Stop( "CMaterialInstance::BindTextures" );

}

/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::BuildPreRenderPB
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/
bool CGameMaterialInstance::BuildPreRenderPB( restrict Heresy_PushBuffer* pPB, const VBHandle* pVB, uint32_t uiStreamCount, bool bRecieveShadows ) const
{
	// get the technique index and bind it
	m_iTechniqueIndex = GetTechniqueIndex( bRecieveShadows );

	// find the shaders
	const CShaderGraph* graph = GetMaterial()->GetBasicGraph(m_iTechniqueIndex, m_eBoundType);

	Heresy_VertexShader* pVertexProgram = (Heresy_VertexShader*) graph->GetVertexShader()->GetVertexHandle()->GetResource()->GetVertexProgram();
	Heresy_PixelShader* pPixelProgram = (Heresy_PixelShader*) graph->GetFragmentShaderNoAlloc()->GetPixelHandle()->GetResource()->GetFragmentProgram();
	uint32_t* pPixelShaderLocation = Heresy_SetPixelShaderScratch( pPB, pPixelProgram );

	// bind the shaders and declaration
	BuildPropertiesPB( pPB, pPixelShaderLocation, graph );

	uint32_t* instanceJumpStart = pPB->m_pCurrent;

	Heresy_SetVertexShader( pPB, pVertexProgram );


	uint32_t* jumpDestination = pPB->m_pCurrent;
	Heresy_SetJumpFixUp( pPB, instanceJumpStart, jumpDestination, HPBP_INSTANCE_JUMP_PRE_DRAW_FIXUP );

	GcShader* pGCVShader = graph->GetVertexShader()->GetVertexHandle().Get();


	for( uint32_t uiStreamIndex = 0; uiStreamIndex < uiStreamCount; uiStreamIndex++ )
	{
		if ( pVB[uiStreamIndex] )
		{
			// crazy arse stream vertex stream name machines... my kingdom for constants...
			GcStreamBuffer* pStreamBuffer = pVB[uiStreamIndex].Get();

			for( int i=0;i < pStreamBuffer->GetNumFields();++i )
			{
				FwHashedString hash = pStreamBuffer->GetField(i).GetName();

				for( uint32_t j=0;j < pGCVShader->GetAttributeCount();++j )
				{
					if( hash == pGCVShader->GetAttributeHash(j) )
					{
						uint16_t iReg = pGCVShader->GetAttributeResourceIndex(j);
						uint32_t iType;
						const GcStreamField& field = pStreamBuffer->GetField(i);
						switch( field.GetType() )
						{
						case Gc::kFloat: iType = HVF_FLOAT; break;
						case Gc::kHalf: iType = HVF_FLOAT16; break;
						case Gc::kPacked: iType = HVF_11_11_10; break;
						case Gc::kShort:
							if( field.GetNormalised() )
								iType = HVF_NORMALISED_INT16;
							else
								iType = HVF_INT16;
							break;
						case Gc::kUByte: 
							if( field.GetNormalised() )
								iType = HVF_NORMALISED_UNSIGNED_BYTE;
							else
								iType = HVF_UNSIGNED_BYTE;
							break;
						default:
							ntAssert_p (0, ("Invalid GC Stream type ") );
							iType = 0;
						}
						UNUSED(iReg);
						// currently all vertex streams are in vram so
						Heresy_SetVertexStreamFormat( pPB, iReg, Heresy_VertexStreamFormatReg( iType, field.GetSize(), pStreamBuffer->GetStride(), 0 ) );
						Heresy_SetVertexStreamOffset( pPB, iReg, pStreamBuffer->GetDataOffset() + field.GetOffset() );
					}
				}
			}
		}
	}



	return true;
}

bool CGameMaterialInstance::BuildPostRenderPB( Heresy_PushBuffer* pPB, const VBHandle* pVB, uint32_t uiStreamCount) const
{

	uint32_t* instanceJumpStart = pPB->m_pCurrent;

	// find the shaders
	const CShaderGraph* graph = GetMaterial()->GetBasicGraph(m_iTechniqueIndex, m_eBoundType);


	// get the shaders
	Shader* pPixelShader = graph->GetFragmentShader();

	// crazy arse stream vertex stream name machines... my kingdom for constants...
	GcShader* pGCVShader = graph->GetVertexShader()->GetVertexHandle().Get();

	for( uint32_t uiStreamIndex = 0; uiStreamIndex < uiStreamCount; uiStreamIndex++ )
	{
		if ( pVB[uiStreamIndex] )
		{
			// crazy arse stream vertex stream name machines... my kingdom for constants...
			GcStreamBuffer* pStreamBuffer = pVB[uiStreamIndex].Get();

			for( int i=0;i < pStreamBuffer->GetNumFields();++i )
			{
				FwHashedString hash = pStreamBuffer->GetField(i).GetName();

				for( uint32_t j=0;j < pGCVShader->GetAttributeCount();++j )
				{
					if( hash == pGCVShader->GetAttributeHash(j) )
					{
						uint16_t iReg = pGCVShader->GetAttributeResourceIndex(j);
						Heresy_DisableVertexStream( pPB, iReg );
					}
				}
			}
		}
	}


	for(int iBinding = 0; iBinding < pPixelShader->GetNumTextureBindings(); ++iBinding)
	{
		const SHADER_TEXTURE_BINDING* pstBinding = pPixelShader->GetTextureBinding(iBinding);
		Heresy_DisableTexture( pPB, pstBinding->iStage );
	}


	uint32_t* jumpDestination = pPB->m_pCurrent;
	Heresy_SetJumpFixUp( pPB, instanceJumpStart, jumpDestination, HPBP_INSTANCE_JUMP_POST_DRAW_FIXUP );

	return true;
}

void CGameMaterialInstance::PreRender( Transform const* pobTransform, bool bRecieveShadows, void const* pDecompMatrix ) const
{
	CGatso::Start( "CGameMaterialInstance::BindShaders" );

	// get the technique index and bind it
	m_iTechniqueIndex = GetTechniqueIndex( bRecieveShadows );

	// cache some properties
	MATERIAL_DATA_CACHE stCache;
	if( pobTransform != 0 )
	{
		stCache.pobTransform = pobTransform;
		stCache.obObjectToWorld = pobTransform->GetWorldMatrixFast();
		stCache.obWorldToObject = stCache.obObjectToWorld.GetAffineInverse();

		if (pDecompMatrix != NULL)
			// I can't use the overloaded assignment operator cause the source matrix from CMeshHeader is not aligned
			NT_MEMCPY (&stCache.obReconstructionMatrix, pDecompMatrix, sizeof(CMatrix));
		else
			stCache.obReconstructionMatrix.SetIdentity();
		
	} else
	{
		stCache.pobTransform = 0;
		stCache.obObjectToWorld.SetIdentity();
		stCache.obWorldToObject.SetIdentity();
		stCache.obReconstructionMatrix.SetIdentity();
	}

	// find the shaders
	const CShaderGraph* graph = GetMaterial()->GetBasicGraph(m_iTechniqueIndex, m_eBoundType);
	// get the shaders
	Shader* pVertexShader = graph->GetVertexShader();
	Shader* pPixelShader = graph->GetFragmentShader();

	// bind the shaders and declaration
	Renderer::Get().SetVertexShader( pVertexShader );


	CGatso::Stop( "CGameMaterialInstance::BindShaders" );

	BindProperties( pVertexShader, pPixelShader, stCache );

	// intentionally set pixel shader after constants are bound
	Renderer::Get().SetPixelShader( pPixelShader );
}

void CGameMaterialInstance::PostRender() const
{
	// find the shaders
	const CShaderGraph* graph = GetMaterial()->GetBasicGraph(m_iTechniqueIndex, m_eBoundType);
	// get the shaders
	Shader* pVertexShader = graph->GetVertexShader();
	Shader* pPixelShader = graph->GetFragmentShader();

	UnBindProperties( pVertexShader, pPixelShader );
}

/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::PreRenderDepths
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

void CGameMaterialInstance::PreRenderDepth( Transform const* pobTransform, bool bShadowProject, void const* pDecompMatrix  ) const
{
	CGatso::Start( "CGameMaterialInstance::BindShaders" );

	// get the technique index and bind it
	m_iTechniqueIndex = GetTechniqueIndex( true );

	// cache some properties
	MATERIAL_DATA_CACHE stCache;
	if( pobTransform != 0 )
	{
		stCache.pobTransform = pobTransform;
		stCache.obObjectToWorld = pobTransform->GetWorldMatrixFast();
		stCache.obWorldToObject = stCache.obObjectToWorld.GetAffineInverse();
		if (pDecompMatrix != NULL)
		{
			// I can't use the overloaded assignment operator cause the source matrix from CMeshHeader is not aligned
			NT_MEMCPY (&stCache.obReconstructionMatrix, pDecompMatrix, sizeof(CMatrix));
		}
		else
			stCache.obReconstructionMatrix.SetIdentity();

	} else
	{
		stCache.pobTransform = 0;
		stCache.obObjectToWorld.SetIdentity();
		stCache.obWorldToObject.SetIdentity();
		stCache.obReconstructionMatrix.SetIdentity();
	}

	// find the shaders
	const CShaderGraph* graph = GetMaterial()->GetDepthWriteGraph(m_eBoundType);
	// get the shaders
	Shader* pVertexShader = graph->GetVertexShader();
	Shader* pPixelShader = graph->GetFragmentShader();

	// bind the shaders and declaration
	Renderer::Get().SetVertexShader( pVertexShader, true );

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

	// intentionally set pixel shader after constants are bound
	Renderer::Get().SetPixelShader( pPixelShader, true );
}

void CGameMaterialInstance::PostRenderDepth( bool bShadowProject ) const
{
	// find the shaders
	const CShaderGraph* graph = GetMaterial()->GetDepthWriteGraph(m_eBoundType);
	// get the shaders
	Shader* pVertexShader = graph->GetVertexShader();
	Shader* pPixelShader = graph->GetFragmentShader();

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
*	FUNCTION		PatchSkinMatrices
***************************************************************************************************/
void CMaterialInstance::PatchSkinMatrices( restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, CHierarchy* pobHierarchy ) const
{
	CGatso::Start("UploadSkinMatrices");
	// ensure the transforms are up to date on the hierarchy
	pobHierarchy->UpdateSkinMatrices();
	ntError_p( m_pucBoneIndices, ( "no bone indices set on skinned material instance" ) );

	static const int iMaxBones = 200;
	ntAssert( m_iNumberOfBonesUsed < iMaxBones );

	CSkinMatrix aTempMatrix[iMaxBones];

	// upload only the matrices used by this mesh
	for(int iBone = 0; iBone < m_iNumberOfBonesUsed; ++iBone)
	{
		// get the matrix index
		int iMatrixIndex = m_pucBoneIndices[iBone];

		// get the matrix for the given index
		const CSkinMatrix* pobBone = &pobHierarchy->GetSkinMatrixArray()[iMatrixIndex];

		aTempMatrix[iBone] = *pobBone;
	}

//	SetVSConst( pVertexShader, iBaseRegister, aTempMatrix, 3 * m_iNumberOfBonesUsed );

	Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, (float*)&aTempMatrix, 3 * m_iNumberOfBonesUsed );

	CGatso::Stop("UploadSkinMatrices");

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

	// get the technique index and bind it
	m_iTechniqueIndex = GetTechniqueIndex( true );

	// cache some properties
	MATERIAL_DATA_CACHE stCache;
	if( pobTransform != 0 )
	{
		stCache.pobTransform = pobTransform;
		stCache.obObjectToWorld = pobTransform->GetWorldMatrixFast();
		stCache.obWorldToObject = stCache.obObjectToWorld.GetAffineInverse();
		if (pDecompMatrix != NULL)
		{
			// I can't use the overloaded assignment operator cause the source matric from CMeshHeader is not aligned
			NT_MEMCPY (&stCache.obReconstructionMatrix, pDecompMatrix, sizeof(CMatrix));
		}
		else
			stCache.obReconstructionMatrix.SetIdentity();

	} else
	{
		stCache.pobTransform = 0;
		stCache.obObjectToWorld.SetIdentity();
		stCache.obWorldToObject.SetIdentity();
		stCache.obReconstructionMatrix.SetIdentity();
	}

	// find the shaders
	const CShaderGraph* graph = GetMaterial()->GetBasicGraph(m_iTechniqueIndex, m_eBoundType);
	// get the shaders
	Shader* pVertexShader = graph->GetVertexShader();
	Shader* pPixelShader = graph->GetFragmentShader();

	// bind the shaders and declaration
	Renderer::Get().SetVertexShader( pVertexShader );

#ifdef PLATFORM_PC
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pobDeclarations[m_iTechniqueIndex] );
#endif

	CGatso::Stop( "CGameMaterialInstance::BindShaders" );

	BindProperties( pVertexShader, pPixelShader, stCache );

	// intentionally set pixel shader after constants are bound
	Renderer::Get().SetPixelShader( pPixelShader );
}

void CGameMaterialInstance::PostRenderShadowRecieve() const
{
	// find the shaders
	const CShaderGraph* graph = GetMaterial()->GetBasicGraph(m_iTechniqueIndex, m_eBoundType);
	// get the shaders
	Shader* pVertexShader = graph->GetVertexShader();
	Shader* pPixelShader = graph->GetFragmentShader();

	UnBindProperties( pVertexShader, pPixelShader );
}


/***************************************************************************************************
//
// Setups all the properties moved to Material instance so that the shadow material can access it
//
***************************************************************************************************/
restrict uint32_t* CMaterialInstance::BuildPropertiesPB( restrict Heresy_PushBuffer* pPB, restrict uint32_t* pPixelShaderLocation, const CShaderGraph* graph ) const
{
	Shader* pVertexShader = graph->GetVertexShader();
	Shader* pPixelShader = graph->GetFragmentShaderNoAlloc();
	Heresy_PixelShader* pPixelProgram = (Heresy_PixelShader*) graph->GetFragmentShaderNoAlloc()->GetPixelHandle()->GetResource()->GetFragmentProgram();
			UNUSED( pPixelProgram );

	// get the material properties
	CMaterialPropertyIterator obMaterialProperties( GetPropertyTable(), GetPropertyTableSize());

	// TODO block optimise constant setting
	// upload vertex shader properties
	for(int iBinding = 0; iBinding < pVertexShader->GetNumPropertyBindings(); ++iBinding)
	{
		const SHADER_PROPERTY_BINDING* pstBinding = pVertexShader->GetPropertyBinding(iBinding);

		// get to the property for this binding
		CMaterialProperty const* pobProperty = GetMaterialProperty( pstBinding->eSemantic );

		const GcShaderResource::Constant* pConstant = pVertexShader->m_hShaderHandle->GetResource()->GetConstants() + pstBinding->iRegister;
		ntAssert( pConstant->IsContiguous() );

		if( pobProperty )
		{
			ntAssert( pstBinding->iStorage == 1 );

			Heresy_SetVertexShaderConstant4F( pPB, pConstant->m_resourceStart, pobProperty->GetFloatData().afFloats );
		}
		else
		{
			Heresy_SetVertexShaderConstantFixupSpace( pPB, pConstant->m_resourceStart, pstBinding->eSemantic, pstBinding->iStorage );
		}
	}

	// reset the properties iterator for the pixel program data
	obMaterialProperties.Reset();

	// upload pixel shader properties
	for(int iBinding = 0; iBinding < pPixelShader->GetNumPropertyBindings(); ++iBinding)
	{
		const SHADER_PROPERTY_BINDING* pstBinding = pPixelShader->GetPropertyBinding(iBinding);

		// get to the property for this binding
		CMaterialProperty const* pobProperty = GetMaterialProperty( pstBinding->eSemantic );
		const GcShaderResource::Constant* pConstant = pPixelShader->m_hShaderHandle->GetResource()->GetConstants() + pstBinding->iRegister;
		ntAssert( !pConstant->IsContiguous() );
		const u16* resourceArray = pPixelShader->m_hShaderHandle->GetResource()->GetResourceArray( pConstant->m_resourceArrayOffset );

		if( pobProperty )
		{
			ntAssert( pstBinding->iStorage == 1 );
			if( resourceArray[0] != 0xFFFF )
			{
				Heresy_SetPixelShaderScratchConstant4F(  pPB, pPixelProgram, pPixelShaderLocation, resourceArray[0], pobProperty->GetFloatData().afFloats );
			}
		}
		else
		{
			Heresy_SetPixelShaderScratchConstantInlineFixupSpace( pPB, pPixelProgram, pPixelShaderLocation, (uint16_t*)resourceArray, pstBinding->eSemantic, pstBinding->iStorage );
		}
	}

	uint32_t* pTextureSettings = pPB->m_pCurrent;

	// bind the textures
	for(int iBinding = 0; iBinding < pPixelShader->GetNumTextureBindings(); ++iBinding)
	{
		const SHADER_TEXTURE_BINDING* pstBinding = pPixelShader->GetTextureBinding(iBinding);

		CMaterialProperty const* pobProperty  = GetMaterialProperty(pstBinding->eSemantic);
		uint32_t* pTexPB;
		if( !pobProperty )
		{
			pTexPB = Heresy_SetTextureFixup( pPB, pstBinding->iStage, pstBinding->eSemantic );
		}
		else
		{
			const GcTextureHandle& handle = pobProperty->GetTextureData().pobTexture->m_Platform.GetTexture();
			Gc::TexFormat textureFormat = handle->GetFormat();
			if (textureFormat == Gc::kTexFormatRGBA16F	||
				textureFormat == Gc::kTexFormatRGBA32F	||
				textureFormat == Gc::kTexFormatGR16F	||
				textureFormat == Gc::kTexFormatR32F	)
				handle->SetGammaCorrect( 0 );
			else
				handle->SetGammaCorrect( pstBinding->iGammaCorrection );

			// set anisotropic filtering
			int anisotropicFilterLevel = GetAnisotropicFilterLevel(this, pstBinding->eSemantic);
			if ( anisotropicFilterLevel )
				handle->SetMaxAnisotropy( (Gc::AnisotropyLevel)anisotropicFilterLevel );

			Heresy_Texture* pTexture = (Heresy_Texture*) &handle->GetIceTexture();
			pTexPB = Heresy_SetTexture( pPB, pstBinding->iStage, pTexture );
		}

		// set the stage state 
		//! note this override the texture remap bit and the zfunc as well...
		//Renderer::Get().SetSamplerAddressMode(pstBinding->iStage, pstBinding->iAddressMode);
		switch( (TEXTUREADDRESS_TYPE)pstBinding->iAddressMode  )
		{
		case TEXTUREADDRESS_CLAMPALL:
			Heresy_OverrideTextureAddressOnly( pTexPB, Heresy_TextureAddressReg( HTA_CLAMP_TO_EDGE, HTA_CLAMP_TO_EDGE, HTA_CLAMP_TO_EDGE, 0, 0 ) );
			break;
		case TEXTUREADDRESS_WRAPALL:
			Heresy_OverrideTextureAddressOnly( pTexPB, Heresy_TextureAddressReg( HTA_WRAP, HTA_WRAP, HTA_WRAP, 0, 0 ) );
			break;
		case TEXTUREADDRESS_BORDERALL:
			Heresy_OverrideTextureAddressOnly( pTexPB, Heresy_TextureAddressReg( HTA_BORDER, HTA_BORDER, HTA_BORDER, 0, 0 ) );
			break;
		case TEXTUREADDRESS_WCC:
			Heresy_OverrideTextureAddressOnly( pTexPB, Heresy_TextureAddressReg( HTA_WRAP, HTA_CLAMP_TO_EDGE, HTA_CLAMP_TO_EDGE, 0, 0 ) );
			break;
		default:
			ntAssert_p(0,("Unrecognised texture address mode: %x\n", (TEXTUREFILTER_TYPE)pstBinding->iAddressMode));
			break;
		}
		//Renderer::Get().SetSamplerFilterMode(pstBinding->iStage, pstBinding->iFilterMode);
		switch( (TEXTUREFILTER_TYPE)pstBinding->iFilterMode  )
		{
		case TEXTUREFILTER_POINT:
			Heresy_OverrideTextureFilterOnly( pTexPB, Heresy_TextureFilterReg( HTF_NEAREST, HTF_NEAREST, 0 ) );
			break;
		case TEXTUREFILTER_BILINEAR:
			Heresy_OverrideTextureFilterOnly( pTexPB, Heresy_TextureFilterReg( HTF_LINEAR, HTF_LINEAR_NEAREST, 0 ) );
			break;
		case TEXTUREFILTER_TRILINEAR:
			Heresy_OverrideTextureFilterOnly( pTexPB, Heresy_TextureFilterReg( HTF_LINEAR, HTF_LINEAR_LINEAR, 0 ) );
			break;
		default:
			ntAssert_p(0,("Unrecognised texture filter mode: %x\n", (TEXTUREFILTER_TYPE)pstBinding->iFilterMode));
			break;
		}
	}

	return pTextureSettings;
	
}


/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::PreRenderDepths
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/
bool CGameMaterialInstance::BuildPreRenderDepthPB( restrict Heresy_PushBuffer* pPB, const VBHandle* pVB, bool bShadowProject ) const
{
	// don't support skinned yet
//	if( m_eBoundType != VSTT_BASIC )
//		return false;

	// find the shaders
	const CShaderGraph* graph = GetMaterial()->GetDepthWriteGraph(m_eBoundType);

	Heresy_VertexShader* pVertexProgram = (Heresy_VertexShader*) graph->GetVertexShader()->GetVertexHandle()->GetResource()->GetVertexProgram();
	Heresy_PixelShader* pPixelProgram = (Heresy_PixelShader*) graph->GetFragmentShaderNoAlloc()->GetPixelHandle()->GetResource()->GetFragmentProgram();
	uint32_t* pPixelShaderLocation = Heresy_SetPixelShaderScratch( pPB, pPixelProgram );
	// bind the shaders and declaration
	BuildPropertiesPB( pPB, pPixelShaderLocation, graph );

	uint32_t* instanceJumpStart = pPB->m_pCurrent;

	// my really nasty hack has moved but this isn't exactly nice either...
	if( bShadowProject )
	{
		if( GetMaterial()->IsAlphaBlended() || GetMaterial()->IsAlphaTested() )
		{
			Heresy_SetCullMode( pPB, HCF_FRONT_AND_BACK );
		}
	}

	Heresy_SetVertexShader( pPB, pVertexProgram );
 

	uint32_t* jumpDestination = pPB->m_pCurrent;
	Heresy_SetJumpFixUp( pPB, instanceJumpStart, jumpDestination, HPBP_INSTANCE_JUMP_PRE_DRAW_FIXUP );

	// crazy arse stream vertex stream name machines... my kingdom for constants...
	GcShader* pGCVShader = graph->GetVertexShader()->GetVertexHandle().Get();
	for (int j = 0; j < 2; j++)
	{
		GcStreamBuffer* pStreamBuffer = pVB[j].Get();
		if ( pStreamBuffer != NULL )
		{
			for( int i=0;i < pStreamBuffer->GetNumFields();++i )
			{
				FwHashedString hash = pStreamBuffer->GetField(i).GetName();

				for( uint32_t j=0;j < pGCVShader->GetAttributeCount();++j )
				{
					if( hash == pGCVShader->GetAttributeHash(j) )
					{
						uint16_t iReg = pGCVShader->GetAttributeResourceIndex(j);
						uint32_t iType;
						const GcStreamField& field = pStreamBuffer->GetField(i);
						switch( field.GetType() )
						{
						case Gc::kFloat: iType = HVF_FLOAT; break;
						case Gc::kHalf: iType = HVF_FLOAT16; break;
						case Gc::kPacked: iType = HVF_11_11_10; break;
						case Gc::kShort:
							if( field.GetNormalised() )
								iType = HVF_NORMALISED_INT16;
							else
								iType = HVF_INT16;
							break;
						case Gc::kUByte: 
							if( field.GetNormalised() )
								iType = HVF_NORMALISED_UNSIGNED_BYTE;
							else
								iType = HVF_UNSIGNED_BYTE;
							break;
						default:
							ntAssert_p (0, ("Invalid GC Stream type ") );
							iType = 0;
						}
						UNUSED(iReg);
						// currently all vertex streams are in vram so
						Heresy_SetVertexStreamFormat( pPB, iReg, Heresy_VertexStreamFormatReg( iType, field.GetSize(), pStreamBuffer->GetStride(), 0 ) );
						Heresy_SetVertexStreamOffset( pPB, iReg, pStreamBuffer->GetDataOffset() + field.GetOffset() );
					}
				}
			}
		}
	}

	return true;
}

bool CGameMaterialInstance::BuildPostRenderDepthPB( Heresy_PushBuffer* pPB, const VBHandle* pVB, bool bShadowProject ) const
{

	uint32_t* instanceJumpStart = pPB->m_pCurrent;


	// find the shaders
	const CShaderGraph* graph = GetMaterial()->GetDepthWriteGraph(m_eBoundType);
	// get the shaders
	Shader* pPixelShader = graph->GetFragmentShaderNoAlloc();


	// crazy arse stream vertex stream name machines... my kingdom for constants...
	GcShader* pGCVShader = graph->GetVertexShader()->GetVertexHandle().Get();
	for (int j = 0; j < 2; j++)
	{
		GcStreamBuffer* pStreamBuffer = pVB[j].Get();
		if ( pStreamBuffer != NULL )
		{
			for( int i=0;i < pStreamBuffer->GetNumFields();++i )
			{
				FwHashedString hash = pStreamBuffer->GetField(i).GetName();

				for( uint32_t j=0;j < pGCVShader->GetAttributeCount();++j )
				{
					if( hash == pGCVShader->GetAttributeHash(j) )
					{
						uint16_t iReg = pGCVShader->GetAttributeResourceIndex(j);
						Heresy_DisableVertexStream( pPB, iReg );
					}
				}
			}
		}
	}

	for(int iBinding = 0; iBinding < pPixelShader->GetNumTextureBindings(); ++iBinding)
	{
		const SHADER_TEXTURE_BINDING* pstBinding = pPixelShader->GetTextureBinding(iBinding);
		Heresy_DisableTexture( pPB, pstBinding->iStage );
	}

	if( bShadowProject )
	{
		if( GetMaterial()->IsAlphaBlended() || GetMaterial()->IsAlphaTested() )
		{
			Heresy_SetCullMode( pPB, HCF_FRONT );
		}
	}
	
	uint32_t* jumpDestination = pPB->m_pCurrent;
	Heresy_SetJumpFixUp( pPB, instanceJumpStart, jumpDestination, HPBP_INSTANCE_JUMP_POST_DRAW_FIXUP );

	return true;
}

void FillSHMatricesInPixelShader( restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch )
{
	switch( pPatch->m_offset )
	{
	case 0:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[0][0].Quadword()  );
		break;
	case 1:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[0][1].Quadword()  );
		break;
	case 2:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[0][2].Quadword()  );
		break;
	case 3:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[0][3].Quadword()  );
		break;

	case 4:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[1][0].Quadword()  );
		break;
	case 5:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[1][1].Quadword()  );
		break;
	case 6:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[1][2].Quadword()  );
		break;
	case 7:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[1][3].Quadword()  );
		break;

	case 8:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[2][0].Quadword()  );
		break;
	case 9:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[2][1].Quadword()  );
		break;
	case 10:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[2][2].Quadword()  );
		break;
	case 11:
		Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_SHMatrices.m_aChannelMats[2][3].Quadword()  );
		break;
	}
}

void CMaterialInstance::FixupPixelShaderConstant(restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, const MATERIAL_DATA_CACHE& stCache ) const
{
	switch( pPatch->m_Semantic )
	{
	// properties that could be on vertex or pixel shaders...
	case PROPERTY_ALPHATEST_THRESHOLD:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &CVector(0.5f,0.5f,0.5f,0.5f).Quadword() );
		}
		break;

	case PROPERTY_FILL_SH_MATRICES:
		{
			FillSHMatricesInPixelShader( pPB, pPatch );
		}
		break;
	case PROPERTY_KEY_DIR_COLOUR:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_keyColour.Quadword() );
		}
		break;
	case PROPERTY_KEY_DIR_OBJECTSPACE:
		{
			CDirection temp( RenderingContext::Get()->m_toKeyLight * stCache.obWorldToObject );
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &temp.Quadword() );
		}
		break;
	case PROPERTY_SPEEDTREE_ALPHATRESHOLD:
		{
			// nothing, this value are set by speedtree code...
		}
		break;

	case PROPERTY_VIEW_POSITION_OBJECTSPACE:
		{
			CPoint temp( RenderingContext::Get()->GetEyePos() * stCache.obWorldToObject );
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &temp.Quadword() );
			break;
		}
	//----------------------
	// Pixel shader properties follow
	//----------------------
	case PROPERTY_REFRACTION_WARP:
		{
			float cameraWidth = RenderingContext::Get()->m_fScreenAspectRatio*tanf(0.5f*RenderingContext::Get()->m_pViewCamera->GetFOVAngle());
			float refractionWarp = 0.25f * cameraWidth;
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &CVector(refractionWarp, refractionWarp, refractionWarp, refractionWarp).Quadword() );
			break;
		}
	case PROPERTY_DEPTH_HAZE_KILL_POWER:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &CVector(8.0f, 8.0f, 8.0f, 8.0f).Quadword() );
			break;
		}
	case PROPERTY_KEY_DIR_REFLECTANCESPACE:
	case PROPERTY_KEY_DIR_WORLDSPACE:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_toKeyLight.Quadword() );
		}
		break;
	case PROPERTY_REFLECTANCE_MAP_COLOUR:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_reflectanceCol.Quadword() );
		}
		break;

	case PROPERTY_SHADOW_MAP_RESOLUTION:
		{
			float fWidth = _R( RenderingContext::Get()->m_pShadowMap->GetWidth() );
			float fHeight = _R( RenderingContext::Get()->m_pShadowMap->GetHeight() );
			CVector temp( fWidth, fHeight, 1.0f / fWidth, 1.0f / fHeight );
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &temp.Quadword() );
		}
		break;
	case PROPERTY_SHADOW_PLANES:
		{
			CVector temp(	RenderingContext::Get()->m_shadowPlanes[1].W(),
							RenderingContext::Get()->m_shadowPlanes[2].W(),	
							RenderingContext::Get()->m_shadowPlanes[3].W(),
							0.0f );
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &temp.Quadword() );
		}
		break;
	case PROPERTY_SHADOW_PLANE0:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_shadowPlanes[0].Quadword() );
		}
		break;
	case PROPERTY_SHADOW_PLANE1:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_shadowPlanes[1].Quadword() );
		}
		break;
	case PROPERTY_SHADOW_PLANE2:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_shadowPlanes[2].Quadword() );
		}
		break;
	case PROPERTY_SHADOW_PLANE3:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_shadowPlanes[3].Quadword() );
		}
		break;
	case PROPERTY_SHADOW_PLANE4:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_shadowPlanes[4].Quadword() );
		}
		break;
	case PROPERTY_SHADOW_RADII:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_shadowRadii[0].Quadword() );
		}
		break;
	case PROPERTY_SHADOW_RADII1:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_shadowRadii[1].Quadword() );
		}
		break;
	case PROPERTY_SHADOW_RADII2:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_shadowRadii[2].Quadword() );
		}
		break;
	case PROPERTY_SHADOW_RADII3:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_shadowRadii[3].Quadword() );
		}
		break;
	case PROPERTY_PARALLAX_SCALE_AND_BIAS:
		{
			CVector scaleAndBias( CONSTRUCT_CLEAR );
			if (CRendererSettings::bUseParallaxMapping)
			{
				scaleAndBias.X() = 0.02f;
				scaleAndBias.Y() = -0.01f;
			}
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &scaleAndBias.Quadword() );
		}
		break;
	case PROPERTY_VPOS_TO_UV:
		{
			CVector posToUV( CONSTRUCT_CLEAR );
			posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth()-1);
			posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight()-1);
			posToUV.Z() =  1.f / Renderer::Get().m_targetCache.GetWidth();
			posToUV.W() = -1.f / Renderer::Get().m_targetCache.GetHeight();
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &posToUV.Quadword() );
		}
		break;
	case PROPERTY_DEPTHOFFIELD_PARAMS:
		{
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict) &RenderingContext::Get()->m_depthOfFieldParams.Quadword() );
		}
		break;
	// hair badnesss, all this freq stuff should be removed
	// but I currently need to tweak them a bit...
	case PROPERTY_HAIR_BASETEX_FREQ:
		{
			CVector tmp(5.0f,5.0f,0.0f,0.0f);
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&tmp.Quadword() );
			break;
		}
	case PROPERTY_HAIR_SHIFTTEX_FREQ:
		{
			CVector tmp(5.0f,5.0f,0.0f,0.0f);
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&tmp.Quadword() );
			break;
		}
	case PROPERTY_HAIR_MASKTEX_FREQ:
		{
			CVector tmp(5.0f,5.0f,0.0f,0.0f);
			Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&tmp.Quadword() );
			break;
		}

	case PROPERTY_ANISOTROPIC_FILTERING:
		break;

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

	default:
		ntError_p( false, ( "unknown material pixel property binding" ) );
	};
}


void CMaterialInstance::FixupTexture(restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch ) const 
{
	switch( pPatch->m_Semantic )
	{
	// texture properties
	case TEXTURE_SHADOW_MAP:
	case TEXTURE_SHADOW_MAP1:
	case TEXTURE_SHADOW_MAP2:
	case TEXTURE_SHADOW_MAP3:
		{
			Heresy_Texture* pTexture = (Heresy_Texture*) &RenderingContext::Get()->m_pShadowMap->m_Platform.GetTexture()->GetIceTexture();
			Heresy_FixupTextureLeaveOverrides( pPB, pTexture, pPatch );
		}
		break;

	case TEXTURE_DIFFUSE1:
	case TEXTURE_DIFFUSE2:
	case TEXTURE_STENCIL_MAP:
		{
			Heresy_Texture* pTexture = (Heresy_Texture*) &RenderingContext::Get()->m_pStencilTarget->m_Platform.GetTexture()->GetIceTexture();
			Heresy_FixupTextureLeaveOverrides( pPB, pTexture, pPatch );
		}
		break;
		
	case TEXTURE_IRRADIANCE_CACHE:
		{
			Heresy_Texture* pTexture = (Heresy_Texture*) &RenderingContext::Get()->m_pIrradianceCache->m_Platform.GetTexture()->GetIceTexture();
			Heresy_FixupTextureLeaveOverrides( pPB, pTexture, pPatch );
		}
		break; 

	case TEXTURE_REFLECTANCE_MAP:
		{
			Heresy_Texture* pTexture = (Heresy_Texture*) &RenderingContext::Get()->m_reflectanceMap->m_Platform.GetTexture()->GetIceTexture();
			Heresy_FixupTextureLeaveOverrides( pPB, pTexture, pPatch );
		}
		break;

/*
	case TEXTURE_CUSTOM_REFLECTANCE_MAP:
		{
			Heresy_Texture* pTexture = (Heresy_Texture*) &RenderingContext::Get()->m_reflectanceMap->m_Platform.GetTexture()->GetIceTexture();
			Heresy_FixupTextureLeaveOverrides( pPB, pTexture, pPatch );
		}
		break; */
	}
}
void CMaterialInstance::FixupVertexShaderConstant(restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, const MATERIAL_DATA_CACHE& stCache ) const
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
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, temp.Quadword() );
		}
		break;
	// properties that could be on vertex or pixel shaders...
	case PROPERTY_FILL_SH_MATRICES:
		{
			uint128_t temp[12];
			NT_MEMCPY( temp, RenderingContext::Get()->m_SHMatrices.m_aChannelMats[0].operator const float *(), sizeof(uint128_t)*4 );
			NT_MEMCPY( temp+4, RenderingContext::Get()->m_SHMatrices.m_aChannelMats[1].operator const float *(), sizeof(uint128_t)*4 );
			NT_MEMCPY( temp+8, RenderingContext::Get()->m_SHMatrices.m_aChannelMats[2].operator const float *(), sizeof(uint128_t)*4 );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, temp, 4*3 );
		}
		break;
	case PROPERTY_KEY_DIR_COLOUR:
		{
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, RenderingContext::Get()->m_keyColour.Quadword() );
		}
		break;
	case PROPERTY_KEY_DIR_OBJECTSPACE:
		{
			CDirection temp( RenderingContext::Get()->m_toKeyLight * stCache.obWorldToObject );
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, temp.Quadword() );
		}
		break;
	case PROPERTY_SPEEDTREE_ALPHATRESHOLD:
		{
			// nothing, this value are set by speedtree code...
		}
		break;
	// properties that could be on vertex shaders
	case PROPERTY_BLEND_TRANSFORMS:
		{
			PatchSkinMatrices( pPB, pPatch, stCache.pobTransform->GetParentHierarchy() );
			break;
		}
	case PROPERTY_POSITION_RECONST_MAT:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp( stCache.obReconstructionMatrix );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, temp.operator const float *(), 4 );
			break;
		}
	case PROPERTY_PROJECTION_NORECONST_MAT:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp( stCache.obObjectToWorld * RenderingContext::Get()->m_worldToScreen );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, temp.operator const float *(), 4 );
			break;
		}
	case PROPERTY_PROJECTION:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp( stCache.obObjectToWorld * RenderingContext::Get()->m_worldToScreen );
			if ( stCache.bApplyPosReconstMatrix )
				temp = stCache.obReconstructionMatrix * temp;
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, temp.operator const float *(), 4 );
			break;
		}
	case PROPERTY_WORLD_TRANSFORM:
	case PROPERTY_REFLECTANCE_MAP_TRANSFORM:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, &stCache.obObjectToWorld, 4 );
			break;
		}
	case PROPERTY_SHADOW_MAP_TRANSFORM:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp( stCache.obObjectToWorld * RenderingContext::Get()->m_shadowMapProjection[0] );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, temp.operator const float *(), 4 );
			break;
		}
	case PROPERTY_SHADOW_MAP_TRANSFORM1:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp( stCache.obObjectToWorld * RenderingContext::Get()->m_shadowMapProjection[1] );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, temp.operator const float *(), 4 );
			break;
		}
	case PROPERTY_SHADOW_MAP_TRANSFORM2:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp( stCache.obObjectToWorld * RenderingContext::Get()->m_shadowMapProjection[2] );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, temp.operator const float *(), 4 );
			break;
		}
	case PROPERTY_SHADOW_MAP_TRANSFORM3:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp( stCache.obObjectToWorld * RenderingContext::Get()->m_shadowMapProjection[3] );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, temp.operator const float *(), 4 );
			break;
		}
	case PROPERTY_VIEW_POSITION_OBJECTSPACE:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CPoint temp( RenderingContext::Get()->GetEyePos() * stCache.obWorldToObject );
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, temp.Quadword() );
			break;
		}
	case PROPERTY_GAME_TIME:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CVector temp( CTimer::Get().GetGameTime(), 0.0f, 0.0f, 0.0f );
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, temp.Quadword() );
			break;
		}
	case PROPERTY_VIEW_TRANSFORM:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CMatrix temp( stCache.obObjectToWorld * RenderingContext::Get()->m_worldToView );
			Heresy_FixupCopyN128bitsVertexShaderConstant( pPB, pPatch, temp.operator const float *(), 4 );
			break;
		}
	case PROPERTY_DEPTH_HAZE_CONSTS_A:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, CDepthHazeSetting::GetAConsts().Quadword() );
			break;	
		}
	case PROPERTY_DEPTH_HAZE_CONSTS_G:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, CDepthHazeSetting::GetGConsts().Quadword() );
			break;
		}
	case PROPERTY_DEPTH_HAZE_BETA1PLUSBETA2:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, CDepthHazeSetting::GetBeta1PlusBeta2().Quadword() );
			break;
		}
	case PROPERTY_DEPTH_HAZE_RECIP_BETA1PLUSBETA2:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, CDepthHazeSetting::GetOneOverBeta1PlusBeta2().Quadword() );
			break;
		}
	case PROPERTY_DEPTH_HAZE_BETADASH1:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, CDepthHazeSetting::GetBetaDash1().Quadword() );
			break;
		}
	case PROPERTY_DEPTH_HAZE_BETADASH2:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, CDepthHazeSetting::GetBetaDash2().Quadword() );
			break;
		}
	case PROPERTY_SUN_DIRECTION_OBJECTSPACE:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			CDirection obLightInObject = CDepthHazeSetting::GetSunDir() * stCache.obWorldToObject;
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, obLightInObject.Quadword() );
			break;
		}
	case PROPERTY_SUN_COLOUR:
		{
			ntAssert((pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) ==  HPBP_VERTEX_CONSTANT_FIXUP );
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, CDepthHazeSetting::GetSunColour().Quadword() );
			break;
		}

	case PROPERTY_HAZE_FRAGMENT_EXTINCTION:
		{	
			CVector vOpjPos = CVector(0.0f, 0.0f, 0.0f, 1.0f) * (stCache.obObjectToWorld);

			float s = (vOpjPos * RenderingContext::Get()->m_worldToView).Length();
			CVector Fex = -s * CDepthHazeSetting::GetBeta1PlusBeta2();
			Fex = CVector( expf(Fex.Y()), expf(Fex.Y()), expf(Fex.Z()), 0.0f );
		
			Fex.Max(CVector(0.0f, 0.0f, 0.0f, 0.0f));
			Fex.Min(CVector(1.0f, 1.0f, 1.0f, 0.0f));

			Fex = CVector(1.0f, 1.0f, 1.0f, 0.0f);
					
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, Fex.Quadword() );	
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
	
			Heresy_Set128bit( pPB->m_pStart + pPatch->m_offset, Scattering.Quadword() );	
			break;
		}

	case PROPERTY_SPEEDTREE_WINDMATRICES:
	case PROPERTY_SPEEDTREE_LEAFCLUSTERS:
	case PROPERTY_SPEEDTREE_LEAFANGLES:
		{
			// nothing, this value are set by speedtree code...
		}
		break;


	}
}

/***************************************************************************************************
FUNCTION		CGameMaterialInstance::PatchVertexProperties
***************************************************************************************************/
void CMaterialInstance::PatchProperties(	restrict Heresy_PushBuffer* pPB,
											const MATERIAL_DATA_CACHE& stCache ) const
{
	restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (pPB+1);

	for( uint16_t i = 0; i < pPB->m_iNumPatches;i++, pPatch++ )
	{
		switch( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) )
		{
		case HPBP_PIXEL_CONSTANT_FIXUP:
			FixupPixelShaderConstant( pPB, pPatch, stCache );
			break;
		case HPBP_TEXTURE_FIXUP:
			FixupTexture( pPB, pPatch );
			break;
		case HPBP_VERTEX_CONSTANT_FIXUP:
			FixupVertexShaderConstant( pPB, pPatch, stCache );
			break;
		default:
			// probably a pixel shader fixup
			break;
		}
	}
}
