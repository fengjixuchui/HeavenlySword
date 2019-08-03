/***************************************************************************************************
*
*	DESCRIPTION		This file is intentionally left full of useless comments.
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/debugshader_pc.h"
#include "gfx/renderer.h"
#include "gfx/material.h"
#include "gfx/hardwarecaps_pc.h"

/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::ctor
*
*	DESCRIPTION		
*
***************************************************************************************************/

DebugShader::DebugShader(bool bExitGameOnFailure):
		m_bExitGameOnFailure(bExitGameOnFailure)
{
	// null init, so that it's possible to check if we have a null shader
	m_pobPixelShader = CComPtr<IDirect3DPixelShader9>(0);
	m_pobVertexShader = CComPtr<IDirect3DVertexShader9>(0);
}


/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::IsValid
*
*	DESCRIPTION		true if the shader inside is valid
*
***************************************************************************************************/

bool DebugShader::IsValid()
{
	switch(m_eType)
	{
		case SHADERTYPE_PIXEL:
		{
			return m_pobPixelShader.Get() != 0;
			break;
		}
		case SHADERTYPE_VERTEX:
		{
			return m_pobVertexShader.Get() != 0;
			break;
		}
		default:
		{
			ntAssert(false);
			return false;
			break;
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::SetASMFunction
*
*	DESCRIPTION		Sets the shader function from assembler source.
*
***************************************************************************************************/

void DebugShader::SetASMFunction(SHADERTYPE eType, const char* pcSource)
{
	m_eType = eType;
	Unload();

	if(m_eType == SHADERTYPE_VERTEX)
	{
		CComPtr<IDirect3DVertexShader9> pobVS = Renderer::Get().m_Platform.AssembleVertexShader(pcSource);
		m_pobVertexShader = pobVS;
	}
	else
	{
		CComPtr<IDirect3DPixelShader9> pobPS = Renderer::Get().m_Platform.AssemblePixelShader(pcSource);
		m_pobPixelShader = pobPS;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::SetHLSLFunction
*
*	DESCRIPTION		Sets the shader function from HLSL source.
*
***************************************************************************************************/

void DebugShader::SetHLSLFunction(SHADERTYPE eType, const char* pcSource, const char* pcFuncName, const char* pcProfile )
{
	m_eType = eType;
	Unload();

	if(m_eType == SHADERTYPE_VERTEX)
	{
		CComPtr<IDirect3DVertexShader9> pobVS = Renderer::Get().m_Platform.CompileVertexShader(pcSource,pcFuncName,pcProfile);
		m_pobVertexShader = pobVS;
	}
	else
	{
		CComPtr<IDirect3DPixelShader9> pobPS = Renderer::Get().m_Platform.CompilePixelShader(pcSource,pcFuncName,pcProfile);
		m_pobPixelShader = pobPS;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::SetFILEFunction
*
*	DESCRIPTION		Sets the shader function from HLSL file.
*
***************************************************************************************************/

bool DebugShader::SetFILEFunction(SHADERTYPE eType, const char* pcFileName, const char* pcFuncName, const char* pcProfile )
{
	m_eType = eType;
	Unload();

	static char acFileName[MAX_PATH];
	Util::GetFiosFilePath( pcFileName, acFileName );

	CComPtr<ID3DXBuffer> pobCompiledSource, pobErrors;
	HRESULT hr;

	if(m_eType == SHADERTYPE_VERTEX)
	{
		hr = D3DXCompileShaderFromFile( acFileName, 0, 0, pcFuncName, pcProfile, 0, pobCompiledSource.AddressOf(), pobErrors.AddressOf(), 0);
		if(FAILED(hr))
		{
			if(pobErrors)
			{
				ntPrintf("Error compiling vertex-DebugShader %s: %s", pcFileName, pobErrors->GetBufferPointer() );
			}
			else
			{
				ntPrintf("Error compiling vertex-DebugShader %s (no ntError message, sorry)\n", pcFileName );
			}

			ntAssert((!m_bExitGameOnFailure )|| (hr == S_OK));
			return false;
		}
		else
		{
			CComPtr<IDirect3DVertexShader9> pobVS = Renderer::Get().m_Platform.CreateVertexShader(reinterpret_cast<const uint32_t*>(pobCompiledSource->GetBufferPointer()));
			m_pobVertexShader = pobVS;
			return true;
		}
	}
	else
	{
		hr = D3DXCompileShaderFromFile( acFileName, 0, 0, pcFuncName, pcProfile, 0, pobCompiledSource.AddressOf(), pobErrors.AddressOf(), 0);
		if(FAILED(hr))
		{
			if(pobErrors)
			{
				ntPrintf("Error compiling pixel-DebugShader %s: %s", pcFileName, pobErrors->GetBufferPointer() );
			}
			else
			{
				ntPrintf("Error compiling pixel-DebugShader %s (no ntError message, sorry)\n", pcFileName );
			}
			
			ntAssert((!m_bExitGameOnFailure )|| (hr == S_OK));
			return false;
		}
		else
		{
			CComPtr<IDirect3DPixelShader9> pobPS = Renderer::Get().m_Platform.CreatePixelShader(reinterpret_cast<const uint32_t*>(pobCompiledSource->GetBufferPointer()));
			m_pobPixelShader = pobPS;
			return true;
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::SetStreamBindings
*
*	DESCRIPTION		Sets the stream bindings for this shader. The bindings are copied locally.
*
***************************************************************************************************/

void DebugShader::SetStreamBindings(const SHADER_STREAM_BINDING* pstBindings, int iNumBindings)
{
	// cache the bindings
	if(iNumBindings > 0)
	{
		m_aobStreamBindings.Reset(NT_NEW_CHUNK(Mem::MC_GFX) SHADER_STREAM_BINDING[iNumBindings]);
		memcpy(m_aobStreamBindings.Get(), pstBindings, iNumBindings*sizeof(SHADER_STREAM_BINDING));
	}
	else
		m_aobStreamBindings.Reset();

	// set up the base class to use them
	m_pstStreamBindings = m_aobStreamBindings.Get();
	m_iNumStreamBindings = iNumBindings;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::SetPropertyBindings
*
*	DESCRIPTION		Sets the property bindings for this shader. The bindings are copied locally.
*
***************************************************************************************************/

void DebugShader::SetPropertyBindings(const SHADER_PROPERTY_BINDING* pstBindings, int iNumBindings)
{
	// cache the bindings
	if(iNumBindings > 0)
	{
		m_aobPropertyBindings.Reset(NT_NEW_CHUNK(Mem::MC_GFX) SHADER_PROPERTY_BINDING[iNumBindings]);
		memcpy(m_aobPropertyBindings.Get(), pstBindings, iNumBindings*sizeof(SHADER_PROPERTY_BINDING));
	}
	else
		m_aobPropertyBindings.Reset();
	
	// set up the base class to use them
	m_pstPropertyBindings = m_aobPropertyBindings.Get();
	m_iNumPropertyBindings = iNumBindings;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::SetTextureBindings
*
*	DESCRIPTION		Sets the texture bindings for this shader. The bindings are copied locally.
*
***************************************************************************************************/

void DebugShader::SetTextureBindings(const SHADER_TEXTURE_BINDING* pstBindings, int iNumBindings)
{
	// cache the bindings
	if(iNumBindings > 0)
	{
		m_aobTextureBindings.Reset(NT_NEW_CHUNK(Mem::MC_GFX) SHADER_TEXTURE_BINDING[iNumBindings]);
		memcpy(m_aobTextureBindings.Get(), pstBindings, iNumBindings*sizeof(SHADER_TEXTURE_BINDING));
	}
	else
		m_aobTextureBindings.Reset();

	// set up the base class to use them
	m_pstTextureBindings = m_aobTextureBindings.Get();
	m_iNumTextureBindings = iNumBindings;
}


//------------------------------------------------------------------------------------------
//!
//!	DebugShaderCache
//! Cache singleton for Debug shaders
//!
//------------------------------------------------------------------------------------------
DebugShaderCache::~DebugShaderCache()
{
	for( ShaderMap::iterator curr = m_shaderMap.begin(); curr != m_shaderMap.end(); )
	{
		NT_DELETE_CHUNK(Mem::MC_GFX,  curr->second );
		curr = m_shaderMap.erase( curr );
	}

#ifndef _RELEASE
	for( ValidationMap::iterator curr = m_ValidationMap.begin(); curr != m_ValidationMap.end(); )
	{
		NT_DELETE_CHUNK(Mem::MC_GFX,  curr->second );
		curr = m_ValidationMap.erase( curr );
	}
#endif
}

//------------------------------------------------------------------------------------------
//!
//!	DebugShaderCache
//! Cache singleton for Debug shaders
//!
//------------------------------------------------------------------------------------------
DebugShader* DebugShaderCache::LoadShaderFILE(	SHADERTYPE eType,
												const char* pName,
												const char* pFuncName, 
												const char* pProfile, 
												bool bExitGameOnFailure )
{
	uint32_t key = CHashedString( pName ).GetValue();

	ShaderMap::iterator it = m_shaderMap.find( key );
	if (it != m_shaderMap.end())
	{
#ifndef _RELEASE
		ValidationMap::iterator checkIt = m_ValidationMap.find( key );
		ntError_p(	checkIt != m_ValidationMap.end(),
					("Debug Shader valdiation not found") );

		Validation* pValid = checkIt->second;
		ntError_p(	pValid->eType == eType, 
					("Debug Shader is not mapped to the same type") );
			
		ntError_p(	pValid->iFuncNameHash == CHashedString(pFuncName).GetValue(), 
					("Debug Shader is not mapped to the same function name") );
			
		ntError_p(	pValid->iProfileHash == CHashedString(pProfile).GetValue(), 
					("Debug Shader is not mapped to the same profile") );
#endif
		return it->second;
	}

	// need to load and cache it
	DebugShader* pResult = NT_NEW_CHUNK(Mem::MC_GFX) DebugShader( bExitGameOnFailure );
	ntAssert_p( pResult, ("Failed to create new DebugShader") );

	// we dont care if the actual load failed, as this is valid behavior. just
	// stick it in the cache and return it.
	pResult->SetFILEFunction( eType, pName, pFuncName, pProfile );
	m_shaderMap[key] = pResult;

#ifndef _RELEASE
	// record a validation entry too
	Validation* pCheck = NT_NEW_CHUNK(Mem::MC_GFX) Validation;
	pCheck->eType = eType;
	pCheck->iFuncNameHash = CHashedString(pFuncName).GetValue();
	pCheck->iProfileHash = CHashedString(pProfile).GetValue();
	m_ValidationMap[key] = pCheck;
#endif

	return pResult;
}
