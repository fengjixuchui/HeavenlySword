/***************************************************************************************************
*
*	$Header:: /game/material.h 3     13/08/03 10:39 Simonb                                         $
*
*	The material and technique class, and the material instance class.
*
*	CHANGES
*
*	2/7/2003	SimonB	Created
*
***************************************************************************************************/

#ifndef _MATERIAL_PC_H
#define _MATERIAL_PC_H

// forward declarations
class Shader;
class CShaderDictionary;

/***************************************************************************************************
*
*	ENUM			SHADERSLOT_TYPE
*
*	DESCRIPTION		The type of shader in a technique shader collection.
*
***************************************************************************************************/

enum SHADERSLOT_TYPE
{
	SHADERSLOT_BASICVERTEX_SM2, 
	SHADERSLOT_SKINNEDVERTEX_SM2, 
	SHADERSLOT_BATCHEDVERTEX_SM2,
	SHADERSLOT_PIXEL_SM2,

	SHADERSLOT_BASICVERTEX_SM3, 
	SHADERSLOT_SKINNEDVERTEX_SM3, 
	SHADERSLOT_BATCHEDVERTEX_SM3,
	SHADERSLOT_PIXEL_SM3,

	SHADERSLOT_BASICVERTEX_DEPTH, 
	SHADERSLOT_SKINNEDVERTEX_DEPTH, 
	SHADERSLOT_BATCHEDVERTEX_DEPTH,
	SHADERSLOT_OUTPUTDEPTH_PIXEL,

	SHADERSLOT_BASICVERTEX_SHADOWSM2, 
	SHADERSLOT_SKINNEDVERTEX_SHADOWSM2, 
	SHADERSLOT_BATCHEDVERTEX_SHADOWSM2,

	SHADERSLOT_BASICVERTEX_SHADOWSM3, 
	SHADERSLOT_SKINNEDVERTEX_SHADOWSM3, 
	SHADERSLOT_BATCHEDVERTEX_SHADOWSM3,

	SHADERSLOT_RECEIVESHADOW_PIXEL,
	SHADERSLOT_RECEIVESHADOW_SMH_PIXEL,

	SHADERSLOT_MAX,
};

enum VERTEXSHADER_TRANSFORM_TYPE
{
	VSTT_BASIC = 0,
	VSTT_SKINNED = 1,
	VSTT_BATCHED = 2,
};


enum MATERIAL_FLAG_TYPE
{
	MATERIAL_ALPHA_BLENDED	= (1 << 0),
	MATERIAL_ALPHA_TESTED	= (1 << 1)
};

/***************************************************************************************************
*
*	CLASS			TECHNIQUE
*
*	DESCRIPTION		A set of passes that render a mesh.
*
***************************************************************************************************/

struct TECHNIQUE
{
	// helpers to work around shader model 2 and 3 issues and skinned batched etc. complexities
	Shader* GetColourVertexShader( VERTEXSHADER_TRANSFORM_TYPE type ) const;
	Shader* GetDepthVertexShader( VERTEXSHADER_TRANSFORM_TYPE type ) const;
	Shader* GetShadowVertexShader( VERTEXSHADER_TRANSFORM_TYPE type ) const;
	Shader* GetColourPixelShader() const;
	Shader* GetShadowPixelShader() const;

	Shader* apobShaders[SHADERSLOT_MAX];
};

/***************************************************************************************************
*
*	CLASS			CMaterial
*
*	DESCRIPTION		A logic block that calls techniques based on the local environment.
*
***************************************************************************************************/

class CMaterial
{
public:
	//! Resolves pointer offsets after loading and loads the shader itself.
	void Load(const CShaderDictionary* pobHead);


	//! Gets the material name.
	const char* GetName() const { return m_pcName; }

	//! Gets the template name.
	const char* GetTemplateName() const { return m_pcTemplateName; }

	// see if this material is supposed to be alpha blended
	bool IsAlphaBlended() const { return !!(m_Flags & MATERIAL_ALPHA_BLENDED); }

	// see if this material is supposed to be alpha tested
	bool IsAlphaTested() const { return !!(m_Flags & MATERIAL_ALPHA_TESTED); }

	//! Gets the number of bindings needed to bind this material to a mesh.
	int GetNumTechniques() const { return m_iNumTechniques; }

	//! Debug: Gets the single technique for this material.
	const TECHNIQUE* GetTechnique(int iIndex) const { return &m_aobTechniques[iIndex]; }


private:
	CMaterial();					//!< Constructor for the basic shaders class.

	const char* m_pcName;			//!< The material name.
	const char* m_pcTemplateName;	//!< The implementation template to use.
	unsigned int m_Flags;			//!< flags MATERIAL_FLAG_TYPE

	int m_iNumTechniques;			//!< The number of techniques in the material.
	TECHNIQUE* m_aobTechniques;		//!< The technique array for this material.

	friend class CBasicShaders;		//!< Allows the fallback shaders to be created directly.
};

/***************************************************************************************************
*
*	CLASS			CShaderDictionary
*
*	DESCRIPTION		A shader dictionary.
*
***************************************************************************************************/

class CShaderDictionary
{
public:
	//! Loads a dictionary from a binary file.
	static CShaderDictionary* Create(const char* pcFileName);

	//! Deallocate shader dictionary
	static void Destroy(CShaderDictionary* pDict);

	//! Returns the number of vertex shaders in the dictionary.
	int GetNumShaders() const { return m_iNumShaders; }

	//! Returns the number of materias in the dictionary.
	int GetNumMaterials() const { return m_iNumMaterials; }

	//! Finds a vertex shader by name.
	Shader* FindShader(const char* pcName) const;

	//! Finds a material by name.
	const CMaterial* FindMaterial(CHashedString const& obHash) const;

private:
	CShaderDictionary();			//!< Private to force construction using the Create function.
	~CShaderDictionary();			//!< Private to force destruction using Destroy function.

	u_int m_uiVersionID;			//!< The dictionary version ID.
//	CHashedString m_obNameHash;		//!< The name hash, filled in from the filename on load.
	FwHashedString m_newNameHash;	//!< The name hash, filled in from the filename on load.
	
	int m_iNumShaders;				//!< The number of pixel shaders in the dictionary.
	Shader* m_pobShaders;			//!< The pixel shaders.

	int m_iNumMaterials;			//!< The number of materials in the dictionary.
	CMaterial* m_pobMaterials;		//!< The materials.
};

#endif // ndef _MATERIAL_PC_H
