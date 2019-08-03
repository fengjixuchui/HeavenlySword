
#include "gfx/shader.h"
#include "gfx/renderstates.h"
#include "gfx/renderer.h"

//-----------------------------------------------------
//!
//!	Shader::Shader
//! Ctor
//!
//-----------------------------------------------------
Shader::Shader() 
  : m_bNonHeresyFragmentCached( false ),
	m_eType(SHADERTYPE_UNKNOWN), 
	m_pcName(0), 
	m_iNumStreamBindings(0), 
	m_iNumPropertyBindings(0), 
	m_iNumTextureBindings(0)
{
}

//-----------------------------------------------------
//!
//!	Shader::Load
//! Fixes up the offset pointers based on the location of the head of the 
//!	dictionary it was loaded from, then allocates a shader based on the
//!	loaded shader code.
//!
//-----------------------------------------------------
void Shader::Load(const void* pMemory, unsigned int size, SHADERTYPE shaderType)
{
	// create the shader
	// if we use heresy, put our fragment shaders in main RAM this will slow down anything
	// that bypasses heresy (the blendshapes and possible particles)
	if( shaderType == SHADERTYPE_PIXEL )
	{
		// now in both heresy or normal mode we put shader in main RAM that we have to allocate and copy post this call..
		// note this will make non-heresy mode slightly slower at the moment.
		m_hShaderHandle = GcShader::Create(pMemory, static_cast<size_t>(size), Gc::kUserBuffer, Gc::kHostMemory);
	} else
	{
		m_hShaderHandle = GcShader::Create(pMemory, static_cast<size_t>(size));
	}
	ntAssert_p(m_hShaderHandle, ("Error: can't create a shader\n"));

	// set some constant values
    m_eType = shaderType;

	m_iNumStreamBindings = m_hShaderHandle->GetAttributeCount();
	m_iNumPropertyBindings = m_hShaderHandle->GetConstantCount();
	m_iNumTextureBindings = m_hShaderHandle->GetSamplerCount();
}

//-----------------------------------------------------
//!
//!	Shader::LoadInternal
//! allocates a shader based on the loaded shader code.
//! note: this may be deferred untill the shader is actually used, hencs the const mutable stuff.
//!
//-----------------------------------------------------
void Shader::LoadInternal( void ) const
{
/* FIXME_WIL not implemented yet
	// create the shader
	if(m_eType == SHADERTYPE_VERTEX)
	{
		CComPtr<IDirect3DVertexShader9> pobVS = Renderer::Get().m_Platform.CreateVertexShader(m_pdwFunction);
		m_pobVertexShader = pobVS;
	}
	else
	{
		CComPtr<IDirect3DPixelShader9> pobPS = Renderer::Get().m_Platform.CreatePixelShader(m_pdwFunction);
		m_pobPixelShader = pobPS;
	}
*/
}

//-----------------------------------------------------
//!
//!	Shader::Unload
//! Clear the shader handle
//!
//-----------------------------------------------------
void Shader::Unload()
{
	m_hShaderHandle.Reset();
}

//-----------------------------------------------------
//!
//!	Shader::SetVSConstant
//! Set vertex shader constant is now a method of GcShaderHandle
//!
//-----------------------------------------------------
void Shader::SetVSConstant( u32 index, const void* pvValue, int iNumRegisters )
{
	ntAssert_p( m_hShaderHandle, ("Shader is null, cannot set vertex shader constants") );
	ntAssert_p( m_hShaderHandle->GetType() == Gc::kVertexProgram, ("Shader not a vertex shader") );
	//ntAssert_p( index != 0xffffffff, ("Invalid constant index for shader") );
	if (index == 0xffffffff) return;

	GcKernel::SetShaderConstant( m_hShaderHandle, index, (float*)pvValue, 0, iNumRegisters );	
}

//-----------------------------------------------------
//!
//!	Shader::SetPSConstant
//! Set pixel shader constant is now a method of GcShaderHandle
//! NOTE! must be called before Renderer::SetPixelShader()

//-----------------------------------------------------
void Shader::SetPSConstant( u32 index, const void* pvValue, int iNumRegisters )
{
	ntAssert_p( m_hShaderHandle, ("Shader is null, cannot set pixel shader constants") );
	ntAssert_p( m_hShaderHandle->GetType() == Gc::kFragmentProgram, ("Shader not a pixel shader") );
	//ntAssert_p( index != 0xffffffff, ("Invalid constant index for shader\n") );
	if (index == 0xffffffff) return;

	GcKernel::SetShaderConstant( m_hShaderHandle, index, (float*)pvValue, 0, iNumRegisters );	
	Renderer::Get().m_Platform.ForcePSRefresh();
}

//-----------------------------------------------------
//!
//!	Shader::SetVSConstantByRegister
//! Set vertex shader constant is now a method of GcShaderHandle
//!
//-----------------------------------------------------
void Shader::SetVSConstantByRegister( u32 registerIndex, const void* pvValue, int iNumRegisters )
{
	ntAssert_p( m_hShaderHandle, ("Shader is null, cannot set vertex shader constants") );
	ntAssert_p( m_hShaderHandle->GetType() == Gc::kVertexProgram, ("Shader not a vertex shader") );

	GcKernel::SetVertexProgramConstant( registerIndex, (float*)pvValue, iNumRegisters );
}


int MatchBindingsNames(CHashedString guess, const CHashedString* bindings,  int bindcount)
{
	int i = 0;
	while (i < bindcount)
	{
		if (guess == bindings[i]) return i;
		i++;
	}

	return -1;
}

//-----------------------------------------------------
//!
//!	Shader::GenerateStreamBindings
//! Generate stream bindings
//!
//-----------------------------------------------------


void Shader::GenerateStreamBindings(SHADER_STREAM_BINDING* pStreamBind)
{
	if (m_iNumStreamBindings > 0)
	{
		int semanticTypesCount = sizeof(aiLinkCgStreamWithStreamSemantic)/(2*sizeof(unsigned int));
		for (int i=0; i < m_iNumStreamBindings; i++)
		{
			unsigned int bindIndex = 0xffffffff;
			unsigned int attributeHash = m_hShaderHandle->GetAttributeHash(i).Get();
			for (int k=0; k < semanticTypesCount; k++)
			{
				if (aiLinkCgStreamWithStreamSemantic[2*k] == attributeHash)
				{
					bindIndex = aiLinkCgStreamWithStreamSemantic[2*k+1];
					break;
				}
			}
			
			if (bindIndex != 0xffffffff)
			{
				pStreamBind[i].eSemantic = static_cast<STREAM_SEMANTIC_TYPE>(bindIndex);
				pStreamBind[i].iUsage = 0;
				pStreamBind[i].iUsageIndex = static_cast<int>(m_hShaderHandle->GetAttributeResourceIndex(i)); 

			}
			else
			{
				pStreamBind[i].eSemantic = STREAM_UNKNOWN;
				pStreamBind[i].iUsage = 0;
				pStreamBind[i].iUsageIndex = 0;
			}
		}

		m_pstStreamBindings = pStreamBind;
	}
}

//-----------------------------------------------------
//!
//!	Shader::GeneratePropertyBindings
//! Generate property bindings
//!
//-----------------------------------------------------
void Shader::GeneratePropertyBindings(SHADER_PROPERTY_BINDING* pPropBind)
{
	if (m_iNumPropertyBindings > 0)
	{
		int propertyTypesCount = sizeof(aiLinkCgUniformWithPropertySemantic)/(3*sizeof(unsigned int));
		for (int i=0; i < m_iNumPropertyBindings; i++)
		{
			unsigned int bindIndex = 0xffffffff;
			unsigned int constantHash = m_hShaderHandle->GetConstantHash(i).Get();
			unsigned int storage = 0;
			for (int k=0; k < propertyTypesCount; k++)
			{
				if (aiLinkCgUniformWithPropertySemantic[3*k] == constantHash)
				{
					bindIndex = aiLinkCgUniformWithPropertySemantic[3*k+1];
					storage = aiLinkCgUniformWithPropertySemantic[3*k+2];
					break;
				}
			}			
			if (bindIndex != 0xffffffff)
			{
				pPropBind[i].eSemantic = static_cast<PROPERTY_SEMANTIC_TYPE>(bindIndex);
				pPropBind[i].iRegister = i;
				pPropBind[i].iStorage = storage; // we can't have this info from a GcShader (oh well..a complete constants description is included in the object, but it's not exposed)
			}
			else
			{
				pPropBind[i].eSemantic = PROPERTY_UNKNOWN;
				pPropBind[i].iRegister = 0;
				pPropBind[i].iStorage = 0;
			}
		}

		m_pstPropertyBindings = pPropBind;
	}

}

//-----------------------------------------------------
//!
//!	Shader::GenerateTextureBindings
//! Generate texture bindings
//!
//-----------------------------------------------------
int QsortTextureBindingComparator( const void* a, const void* b )
{
	const SHADER_TEXTURE_BINDING* bindA = static_cast<const SHADER_TEXTURE_BINDING*>(a);
	const SHADER_TEXTURE_BINDING* bindB = static_cast<const SHADER_TEXTURE_BINDING*>(b);

	return bindA->eSemantic > bindB->eSemantic;
}

void Shader::GenerateTextureBindings(SHADER_TEXTURE_BINDING* pTextureBind)
{
	if (m_iNumTextureBindings > 0)
	{
		int semanticTypesCount = sizeof(aiLinkCgSamplerWithTextureSemantic)/(2*sizeof(unsigned int));
		for (int i=0; i < m_iNumTextureBindings; i++)
		{
			// find the semantic type of this sampler
			uint32_t semanticType = 0xffffffff;
			uint32_t samplerHash = m_hShaderHandle->GetSamplerHash(i).Get();

			for (int k=0; k < semanticTypesCount; k++)
			{
				if (aiLinkCgSamplerWithTextureSemantic[2*k] == samplerHash)
				{
					semanticType = aiLinkCgSamplerWithTextureSemantic[2*k+1];
					break;
				}
			}

			ntAssert_p( semanticType != 0xffffffff, ("Sampler type not found in GenerateTextureBindings()"));

			// set semantic and texture stage
			pTextureBind[i].eSemantic = static_cast<TEXTURE_SEMANTIC_TYPE>(semanticType);
			pTextureBind[i].iStage = static_cast<int>(m_hShaderHandle->GetSamplerResourceIndex(i));

			// assign filter and addressing modes
			switch(pTextureBind[i].eSemantic)
			{
			case TEXTURE_CUSTOM_REFLECTANCE_MAP:
			case TEXTURE_REFLECTANCE_MAP:
				pTextureBind[i].iAddressMode = TEXTUREADDRESS_CLAMPALL;
				pTextureBind[i].iFilterMode = TEXTUREFILTER_TRILINEAR;
				pTextureBind[i].iGammaCorrection = Gc::kGammaCorrectSrgb;
				break;

			case TEXTURE_IRRADIANCE_CACHE:
				pTextureBind[i].iAddressMode = TEXTUREADDRESS_CLAMPALL;
				pTextureBind[i].iFilterMode = TEXTUREFILTER_BILINEAR;
				pTextureBind[i].iGammaCorrection = 0;
				break;

			case TEXTURE_DIFFUSE0:
			case TEXTURE_DIFFUSE1:
			case TEXTURE_DIFFUSE2:
			case TEXTURE_SPECULAR_COLOUR_METALLICITY:
			case TEXTURE_EMISSIVITY_MAP:
			case TEXTURE_GLITTERING_MAP:
			case TEXTURE_VELVET_FILL_SPECULARITY:
			case TEXTURE_VELVET_SILHOUETTE_SPECULARITY:
				pTextureBind[i].iAddressMode = TEXTUREADDRESS_WRAPALL;
				pTextureBind[i].iFilterMode = TEXTUREFILTER_TRILINEAR;
				pTextureBind[i].iGammaCorrection = Gc::kGammaCorrectSrgb;
				break;

			case TEXTURE_MASK:
				pTextureBind[i].iAddressMode = TEXTUREADDRESS_WRAPALL;
				pTextureBind[i].iFilterMode = TEXTUREFILTER_TRILINEAR;
				pTextureBind[i].iGammaCorrection = 0;
				break;

			case TEXTURE_HDR_DIFFUSE0:
				pTextureBind[i].iAddressMode = TEXTUREADDRESS_WRAPALL;
				pTextureBind[i].iFilterMode = TEXTUREFILTER_TRILINEAR;
				pTextureBind[i].iGammaCorrection = 0;
				break;

			case TEXTURE_BSSKIN_WRINKLE_REGIONS0:
			case TEXTURE_BSSKIN_WRINKLE_REGIONS1:
				pTextureBind[i].iAddressMode = TEXTUREADDRESS_WRAPALL;
				pTextureBind[i].iFilterMode = TEXTUREFILTER_TRILINEAR;
				pTextureBind[i].iGammaCorrection = 0;
				break;

			case TEXTURE_NORMAL_MAP:
			case TEXTURE_BSSKIN_BASE_NORMAL_OCCLUSION:
			case TEXTURE_BSSKIN_WRINKLE_NORMAL_OCCLUSION:
				pTextureBind[i].iAddressMode = TEXTUREADDRESS_WRAPALL;
				pTextureBind[i].iFilterMode = TEXTUREFILTER_TRILINEAR;
				pTextureBind[i].iGammaCorrection = 0;
				break;

			case TEXTURE_SHADOW_MAP:
			case TEXTURE_SHADOW_MAP1:
			case TEXTURE_SHADOW_MAP2:
			case TEXTURE_SHADOW_MAP3:
				pTextureBind[i].iAddressMode = TEXTUREADDRESS_CLAMPALL;
				pTextureBind[i].iFilterMode = TEXTUREFILTER_BILINEAR;
				pTextureBind[i].iGammaCorrection = 0;
				break;

			case TEXTURE_BSSKIN_SPECULAR_GAIN:
			case TEXTURE_BSSKIN_DIFFUSE_GAIN:
				pTextureBind[i].iAddressMode = TEXTUREADDRESS_CLAMPALL;
				pTextureBind[i].iFilterMode = TEXTUREFILTER_BILINEAR;
				pTextureBind[i].iGammaCorrection = 0;
				break;

			case TEXTURE_STENCIL_MAP:
				pTextureBind[i].iAddressMode = TEXTUREADDRESS_CLAMPALL;
				pTextureBind[i].iFilterMode = TEXTUREFILTER_POINT;
				pTextureBind[i].iGammaCorrection = 0;
				break;

			default:
				ntAssert_p( 0, ("Unrecognised semantic texture type in GenerateTextureBindings()") );
			}
		}

		m_pstTextureBindings = pTextureBind;

		// sort texture bindings
		qsort( static_cast<void*>(&m_pstTextureBindings), m_iNumTextureBindings, sizeof(SHADER_TEXTURE_BINDING), 
			QsortTextureBindingComparator );
	}
}
