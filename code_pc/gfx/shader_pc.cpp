/***************************************************************************************************
*
*	DESCRIPTION		This file is intentionally left full of useless comments.
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/material.h"
#include "gfx/hardwarecaps_pc.h"

// use the plain text version
//#define USE_XENON_PLAIN_TEXT
/***************************************************************************************************
*
*	FUNCTION		BindStreams
*
*	DESCRIPTION		Loads simpler with the new vertex buffer format. Winnar!
*		puiStream = which stream each element is in, must be iNumElements long if null assumes all element 
					in stream 0
***************************************************************************************************/

bool BindStreams( SHADER_STREAM_BINDING const* pstShaderBindings, int iNumShaderBindings, 
				  CMeshVertexElement const* pobElements, int iNumElements, 
				  unsigned int* puiStream, 
				  D3DVERTEXELEMENT9* pstDeclaration )
{
	// sanity check
	ntAssert( pstShaderBindings && pobElements && pstDeclaration );
	ntAssert( iNumShaderBindings > 0 && iNumElements > 0 );

	// loop through the streams and buffer elements, linking stuff up
	for( int iBinding = 0, iElement = 0; iBinding < iNumShaderBindings; ++iBinding )
	{
		// currently reset every iteration due to batched render patching things, makes this
		// quadratic rather than linear big O
		iElement = 0;
		// skip forward to the element for this binding
		while( pobElements[iElement].m_eStreamSemanticTag != pstShaderBindings[iBinding].eSemantic )
		{
			if( ++iElement == iNumElements )
				return false;
		}

		// connect up to the stream binding
		pstDeclaration[iBinding].Method		= D3DDECLMETHOD_DEFAULT;
		pstDeclaration[iBinding].Offset		= static_cast<WORD>( pobElements[iElement].m_iOffset );
		if( puiStream )
		{
			pstDeclaration[iBinding].Stream	= static_cast<WORD>( puiStream[iElement] );
		} else
		{
			pstDeclaration[iBinding].Stream	= 0;
		}

		pstDeclaration[iBinding].Type		= static_cast<uint8_t>( pobElements[iElement].m_eType );
		pstDeclaration[iBinding].Usage		= static_cast<uint8_t>( pstShaderBindings[iBinding].iUsage );
		pstDeclaration[iBinding].UsageIndex = static_cast<uint8_t>( pstShaderBindings[iBinding].iUsageIndex );
	}

	// close the declaration
	static const D3DVERTEXELEMENT9 stEnd = D3DDECL_END();
	NT_MEMCPY( &pstDeclaration[iNumShaderBindings], &stEnd, sizeof( D3DVERTEXELEMENT9 ) );

	// done
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CVertexShader::CVertexShader
*
*	DESCRIPTION		Creates an empty vertex shader.
*
***************************************************************************************************/

Shader::Shader() 
  : m_eType(SHADERTYPE_UNKNOWN), 
	m_pcName(0), 
	m_pdwFunction(0), 
	m_iNumPropertyBindings(0), 
	m_iNumStreamBindings(0), 
	m_iNumTextureBindings(0)
{
}

/***************************************************************************************************
*
*	FUNCTION		Shader::Load
*
*	DESCRIPTION		Fixes up the offset pointers based on the location of the head of the 
*					dictionary it was loaded from, then allocates a DirectX shader based on the
*					loaded shader code.
*
***************************************************************************************************/
void Shader::Load(const CShaderDictionary* pobHead)
{
	// fix up the pointers
	ResolveOffset(m_pcName, pobHead);
	ResolveOffset(m_pdwFunction, pobHead);
	ResolveOffset(m_pstStreamBindings, pobHead);
	ResolveOffset(m_pstPropertyBindings, pobHead);
	ResolveOffset(m_pstTextureBindings, pobHead);

#ifndef DELAY_SHADER_CREATION
	LoadInternal();
#endif
}

/***************************************************************************************************
*
*	FUNCTION		Shader::LoadInternal
*
*	DESCRIPTION		allocates a DirectX shader based on the loaded shader code.
*
*	NOTES			this may be deferred untill the shader is actually used, hencs the const mutable stuff.
*
***************************************************************************************************/
void Shader::LoadInternal( void ) const
{
#if !defined(USE_XENON_PLAIN_TEXT)
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
#else
	//Alternate plain text in shader dict version for Xenon
	// found out how big the plain text is and restore newline
	char* iPlainText = const_cast<char*>(reinterpret_cast<const char*>(m_pdwFunction));
	char* iEndPlainText = iPlainText;
	while( *iEndPlainText != 0 )
	{
		// EVIL Sorry but binify and D3DXAssembleShader disagree on newlines
		// so I need to get the original back
		if( *iEndPlainText == '~' )
		{
			if( *(iEndPlainText+1) == '¬')
			{
				// an encoded new line
				*iEndPlainText = 0x0D;
				*(iEndPlainText+1) = 0x0A;
			}
		}
		iEndPlainText++;
	}

	LPD3DXBUFFER pShader = 0;

	// xenon shaderdict currently stores shader in ASM plain text so D3DXAssembleshader
	D3DXAssembleShader( iPlainText, iEndPlainText - iPlainText, 0, 0, 0, &pShader, 0 );

	ntAssert( pShader != 0 );

	// create the shader
	if(m_eType == SHADERTYPE_VERTEX)
	{
		CComPtr<IDirect3DVertexShader9> pobVS = 
				Renderer::Get().CreateVertexShader( (uint32_t*) pShader->GetBufferPointer() );
		m_pobVertexShader = pobVS;
	}
	else
	{
		CComPtr<IDirect3DPixelShader9> pobPS = 
				Renderer::Get().CreatePixelShader( (uint32_t*) pShader->GetBufferPointer() );
		m_pobPixelShader = pobPS;
	}

	pShader->Release();
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CVertexShader::Unload
*
*	DESCRIPTION		Deallocates the DirectX shader.
*
***************************************************************************************************/

void Shader::Unload()
{
	if(m_eType == SHADERTYPE_VERTEX)
	{
		m_pobVertexShader.SafeRelease();
	}
	else
	{
		m_pobPixelShader.SafeRelease();
	}
}

Shader* TECHNIQUE::GetColourVertexShader( VERTEXSHADER_TRANSFORM_TYPE type ) const
{
	if( HardwareCapabilities::Get().SupportsShaderModel3() )
	{
		return apobShaders[ SHADERSLOT_BASICVERTEX_SM3 + type ];
	} else
	{
		return apobShaders[ SHADERSLOT_BASICVERTEX_SM2 + type ];
	}
}

Shader* TECHNIQUE::GetColourPixelShader() const 
{
	if( HardwareCapabilities::Get().SupportsShaderModel3() )
	{
		return apobShaders[ SHADERSLOT_PIXEL_SM3 ];
	} else
	{
		return apobShaders[ SHADERSLOT_PIXEL_SM2 ];
	}
}

Shader* TECHNIQUE::GetDepthVertexShader( VERTEXSHADER_TRANSFORM_TYPE type ) const
{
	return apobShaders[ SHADERSLOT_BASICVERTEX_DEPTH + type ];
}


Shader* TECHNIQUE::GetShadowVertexShader( VERTEXSHADER_TRANSFORM_TYPE type ) const
{
	if( HardwareCapabilities::Get().SupportsShaderModel3() && HardwareCapabilities::Get().SupportsHardwareShadowMaps() )
	{
		return apobShaders[ SHADERSLOT_BASICVERTEX_SHADOWSM3 + type ];
	} else
	{
		return apobShaders[ SHADERSLOT_BASICVERTEX_SHADOWSM2 + type ];
	}
}

Shader* TECHNIQUE::GetShadowPixelShader() const
{
	// note I currently assume (incorrectly) that SM3 also has shadowmap hardwware (SMH)...
	if( HardwareCapabilities::Get().SupportsShaderModel3() && HardwareCapabilities::Get().SupportsHardwareShadowMaps() )
	{
		return apobShaders[ SHADERSLOT_RECEIVESHADOW_SMH_PIXEL ];
	} else
	{
		return apobShaders[ SHADERSLOT_RECEIVESHADOW_PIXEL ];
	}
}

