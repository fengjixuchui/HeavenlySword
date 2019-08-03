//------------------------------------------------------------------------------------------
//!
//!	\file debugshader_ps3.h
//!
//------------------------------------------------------------------------------------------

#ifndef GFX_DEBUGSHADER_PS3_H
#define GFX_DEBUGSHADER_PS3_H 

#ifndef GFX_SHADER_H
#include "gfx/shader.h"
#endif

#ifndef EFFECT_RESOURCEMAN_H
#include "effect/effect_resourceman.h"
#endif

class DebugShaderCache;

//------------------------------------------------------------------------------------------
//!
//!	DebugShader
//! shader sourced from a compiles SHO file
//!
//------------------------------------------------------------------------------------------
class DebugShader : public Shader, public EffectResource
{
public:
	void SetName( const char* pcName ) { m_pcName = pcName; }

	void SetStreamBindings(const SHADER_STREAM_BINDING* pstBindings, int iNumBindings);
	void SetPropertyBindings(const SHADER_PROPERTY_BINDING* pstBindings, int iNumBindings);
	void SetTextureBindings(const SHADER_TEXTURE_BINDING* pstBindings, int iNumBindings);
	
	bool IsValid();

	// Effect Resource stuff
	virtual void GenerateResources	();
	virtual bool ResourcesOutOfDate	() const;

private:
	friend class DebugShaderCache;
	DebugShader(bool bExitGameOnFailure = true);
	~DebugShader();
	bool SetCGFile(const char* pcSourceFile);

	CScopedArray<SHADER_STREAM_BINDING> m_aobStreamBindings;
	CScopedArray<SHADER_PROPERTY_BINDING> m_aobPropertyBindings;
	CScopedArray<SHADER_TEXTURE_BINDING> m_aobTextureBindings;
	CScopedArray< uint8_t > m_ShaderMemory;

	/// if false, the shader will try not to crash on compilation failure
	bool m_bExitGameOnFailure;

#ifdef _DEBUG
	// I need this to store an ATGBinary shader filename in order to check its timestamp
	ntstd::String m_ShaderFileNameFull;
	ntstd::String m_ShaderFileName;
	time_t m_iShaderTimestamp;
#endif
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
	DebugShaderCache();
	~DebugShaderCache();
	DebugShader* LoadShader( const char* pName, bool bExitGameOnFailure = true );

private:
	typedef ntstd::Map<u_int,DebugShader*> ShaderMap;
	ShaderMap	m_shaderMap;
};


#endif // _DEBUGSHADER_PS3_H
