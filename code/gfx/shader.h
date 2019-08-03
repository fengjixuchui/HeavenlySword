/***************************************************************************************************
*
*	$Header:: /game/shader.h 29    13/08/03 10:39 Simonb                                           $
*
*	Shader classes and the shader dictionary.
*
*	CHANGES
*
*	18/2/2003	SimonB	Created
*
***************************************************************************************************/

#ifndef GFX_SHADER_H
#define GFX_SHADER_H

#include "core/semantics.h"
#include "core/exportstruct_clump.h"

#define DELAY_SHADER_CREATION

// forward declarations
class CShaderDictionary;
class CMaterial;
class DebugShader;

/***************************************************************************************************
*
*	CLASS			SHADER_STREAM_BINDING
*
*	DESCRIPTION		A vertex shader binding to a stream. This maps a single element to a shader
*					input.
*
***************************************************************************************************/

struct SHADER_STREAM_BINDING
{
	STREAM_SEMANTIC_TYPE eSemantic;
	int iUsage;
	int iUsageIndex;
};

/***************************************************************************************************
*
*	CLASS			SHADER_PROPERTY_BINDING
*
*	DESCRIPTION		A property binding. A property is a constant value to be loaded into either
*					vertex or pixel shader registers.
*
***************************************************************************************************/

struct SHADER_PROPERTY_BINDING
{
	PROPERTY_SEMANTIC_TYPE eSemantic;
	int iRegister;
	int iStorage;
};

/***************************************************************************************************
*
*	CLASS			SHADER_TEXTURE_BINDING
*
*	DESCRIPTION		A texture binding to a particular texture stage.
*
***************************************************************************************************/

struct SHADER_TEXTURE_BINDING
{
	TEXTURE_SEMANTIC_TYPE eSemantic;
	int iStage;
	int iAddressMode;
	int iFilterMode;

	// we cannot change the size of this struct on PC, as these are exported data.
#ifdef PLATFORM_PS3
	int iGammaCorrection;
#endif
};

/***************************************************************************************************
*
*	ENUM			SHADERTYPE
*
*	DESCRIPTION		The type of a given shader.
*
***************************************************************************************************/

enum SHADERTYPE
{
	SHADERTYPE_UNKNOWN = -1, 
	SHADERTYPE_VERTEX, 
	SHADERTYPE_PIXEL, 
};



//----------------------------------
// These include must happen here and only here
// They are the platform dependent bits
//-----------------------------------
#if defined( PLATFORM_PC )
#include "gfx/shader_pc.h"
#include "gfx/debugshader_pc.h"
#elif defined( PLATFORM_PS3 )
#include "gfx/shader_ps3.h"
#include "gfx/debugshader_ps3.h"
#include "gfx/graphicsdevice_ps3.h"
#endif


/***************************************************************************************************
*
*	CLASS			ShaderManager
*
*	DESCRIPTION		Manages a set of shader dictionaries.
*
*	NOTES			To allow shaders to be overridden in later content, dictionaries are searched
*					in the reverse order from which they are loaded.
*
***************************************************************************************************/

class ShaderManager : public Singleton<ShaderManager>
{
public:
	//! Loads the default shader dictionary.
	ShaderManager();

	//! Unloads all loaded dictionaries (this will unload every shader pair in the game).
	~ShaderManager();

	//! Loads a shader dictionary from disk.
	void LoadDictionary(const char* pcFileName);

#ifdef PLATFORM_PS3
	//! Loads a shader dictionary from disk.
	void LoadDictionary(DebugShader** pDebugShaders, const char* pcMaterialName);
#endif

	//! Returns the number of vertex shaders over all loaded dictionaries.
	int GetNumShaders() const;

	//! Returns the number of materials loaded over all dictionaries.
	int GetNumMaterials() const;

	//! Finds a shader by name... There's some const badness going on here !!! (this method should not be used on the PS3)
	Shader* FindShader(const char* pcName) const;

	//! Finds a material by hash.
	const CMaterial* FindMaterial(CHashedString const& obHash) const;

	void ReloadShaders(void);

	bool m_bForceShadersReload;

private:
	typedef ntstd::List<CShaderDictionary*, Mem::MC_GFX> ShaderDictionaryList;
	typedef ntstd::List<ntstd::String, Mem::MC_GFX> GfxStringList;

	ShaderDictionaryList m_obDictionaries;	//!< The currently loaded dictionaries.
	GfxStringList m_obDictFileName;		//!< The currently loaded dictionaries full path file names
};

#endif // _SHADER_H
