/***************************************************************************************************
*
*	CLASS			CDebugVertexShader
*
*	DESCRIPTION		A debug vertex shader compiled on the fly.
*
***************************************************************************************************/

//! Debug vertex shader wrapper.

#ifndef GFX_DEBUGSHADER_PC_H
#define GFX_DEBUGSHADER_PC_H

#include "gfx/shader.h"

class DebugShader : public Shader
{
public:
	DebugShader(bool bExitGameOnFailure = true);
	void SetName( const char* pcName ) { m_pcName = pcName; }

	void SetASMFunction(SHADERTYPE eType, const char* pcSource);
	void SetHLSLFunction(SHADERTYPE eType, const char* pcSource, const char* pcFuncName, const char* pcProfile );
	bool SetFILEFunction(SHADERTYPE eType, const char* pcSource, const char* pcFuncName, const char* pcProfile );

	void SetStreamBindings(const SHADER_STREAM_BINDING* pstBindings, int iNumBindings);
	void SetPropertyBindings(const SHADER_PROPERTY_BINDING* pstBindings, int iNumBindings);
	void SetTextureBindings(const SHADER_TEXTURE_BINDING* pstBindings, int iNumBindings);
	
	bool IsValid();
private:
	CScopedArray<SHADER_STREAM_BINDING> m_aobStreamBindings;
	CScopedArray<SHADER_PROPERTY_BINDING> m_aobPropertyBindings;
	CScopedArray<SHADER_TEXTURE_BINDING> m_aobTextureBindings;
	
	/// if false, the shader will try not to crash on compilation failure
	bool m_bExitGameOnFailure;
};

//------------------------------------------------------------------------------------------
//!
//!	DebugShaderCache
//! Cache singleton for Debug shaders
//!
//------------------------------------------------------------------------------------------
class DebugShaderCache : public Singleton<DebugShaderCache>
{
public:
	~DebugShaderCache();
	DebugShader* LoadShaderFILE(	SHADERTYPE eType,
									const char* pName,
									const char* pFuncName, 
									const char* pProfile,
									bool bExitGameOnFailure = true );
private:
	typedef ntstd::Map<u_int,DebugShader*> ShaderMap;
	ShaderMap	m_shaderMap;

#ifndef _RELEASE
	struct Validation
	{
		SHADERTYPE eType;
		u_int iFuncNameHash;
		u_int iProfileHash;
	};

	typedef ntstd::Map<u_int,Validation*> ValidationMap;
	ValidationMap	m_ValidationMap;
#endif
};

#endif // GFX_DEBUGSHADER_PC_H