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

#ifndef _MATERIAL_PS3_H
#define _MATERIAL_PS3_H

// forward declarations
class Shader;
class DebugShader;
class CShaderDictionary;
class SHADER_STREAM_BINDING;
class SHADER_PROPERTY_BINDING;
class SHADER_TEXTURE_BINDING;

// Each material has a number of fixed graph slots, every graph has got a vertex and a fragment shader
enum GRAPH_SLOT_TYPE
{
	// basic rendering slots
	GRAPHSLOT_BASIC = 0,
	GRAPHSLOT_BASIC_DEPTHHAZE,
	GRAPHSLOT_BASIC_SKINNED,
	GRAPHSLOT_BASIC_DEPTHHAZE_SKINNED,

	// receive shadow rendering slots
	GRAPHSLOT_RECEIVESHADOW,
	GRAPHSLOT_RECEIVESHADOW_DEPTHHAZE,
	GRAPHSLOT_RECEIVESHADOW_SKINNED,
	GRAPHSLOT_RECEIVESHADOW_DEPTHHAZE_SKINNED,

	// depth write rendering slots
	GRAPHSLOT_SHADOW_DEPTHWRITE,
	GRAPHSLOT_SHADOW_DEPTHWRITE_SKINNED,

	// shadow receive rendering slots
	GRAPHSLOT_DEBUG_LAYER1,
	GRAPHSLOT_DEBUG_LAYER2,

	GRAPHSLOT_MAX
};


enum VERTEXSHADER_TRANSFORM_TYPE
{
	VSTT_BASIC = 0,
	VSTT_SKINNED = 1,
	VSTT_BATCHED = 2
};


enum MATERIAL_FLAG_TYPE
{
	MATERIAL_ALPHA_BLENDED	= (1 << 0),
	MATERIAL_ALPHA_TESTED	= (1 << 1),
	MATERIAL_DEBUG_LAYER	= (1 << 2),
};


/***************************************************************************************************
*
*	CLASS			CShaderGraph
*
*	DESCRIPTION		A very simple class used to map a graph and its metadata
*
***************************************************************************************************/


class CShaderGraph
{

public:

	Shader* GetVertexShader(void) const;
	Shader* GetFragmentShader(void) const;

	//! normally we alloc a RSX_MAIN copy of the fragment shader microcode (for alignment) when we first ask for it via GetFragmentShader
	Shader* GetFragmentShaderNoAlloc(void) const;

	void SetVertexShader(Shader* shader) { m_pVertexShader = shader; }
	void SetFragmentShader(Shader* shader){ m_pFragmentShader = shader; }

private:
	Shader* m_pVertexShader;
	Shader* m_pFragmentShader;
	
	//Maybe graph metadata will be used at some point in the future..
	//HSGraphMetadata oMetadata;
};


//////////////////////////////////////////////////////////////////////////////////////////////////
// DICTIONARY STUFF
//////////////////////////////////////////////////////////////////////////////////////////////////


/***************************************************************************************************
*
*	CLASS			HSDictionaryHeader
*
*	DESCRIPTION		This is just the dictionary file header.
*
***************************************************************************************************/

class HSDictionaryHeader
{
public:
	unsigned int magic;
	unsigned int graphcount;
	unsigned int uniqueshadercount;
	unsigned int materialcount;
};

/***************************************************************************************************
*
*	CLASS			HSGraphMetadata
*
*	DESCRIPTION		A class to hold a graph description
*
***************************************************************************************************/

class HSGraphMetadata
{
public:
	
	enum eMetaDataFlags
	{
		GRAPHFLAGS_BASIC = 0,
		GRAPHFLAGS_SHADOW,
		GRAPHFLAGS_DEPTHHAZE,
		GRAPHFLAGS_DEPTHWRITE,
		GRAPHFLAGS_RECEIVESHADOW,
		GRAPHFLAGS_ALPHABLEND,
		GRAPHFLAGS_ALPHATEST,
		GRAPHFLAGS_SKINNED,
		GRAPHFLAGS_UNUSED7,
		GRAPHFLAGS_UNUSED6,
		GRAPHFLAGS_UNUSED5,
		GRAPHFLAGS_UNUSED4,
		GRAPHFLAGS_UNUSED3,
		GRAPHFLAGS_UNUSED2,
		GRAPHFLAGS_UNUSED1,
		GRAPHFLAGS_UNUSED0,
		GRAPHFLAGS_ENTRYINDEX,
	};

	HSGraphMetadata() { Reset(); }
	void Reset(void) { iMetaData = 0; }

	bool IsBasicTemplate(void) const { return (iMetaData&(1<<GRAPHFLAGS_BASIC) ? true : false); }
	bool IsShadowTemplate(void) const  { return (iMetaData&(1<<GRAPHFLAGS_SHADOW) ? true : false); }
	bool IsDepthHaze(void) const  { return (iMetaData&(1<<GRAPHFLAGS_DEPTHHAZE)? true : false); }
	bool IsDepthWrite(void) const  { return (iMetaData&(1<<GRAPHFLAGS_DEPTHWRITE) ? true : false); }
	bool IsReceiveShadow(void) const  { return (iMetaData&(1<<GRAPHFLAGS_RECEIVESHADOW) ? true : false); }
	bool IsAlphaBlend(void) const  { return (iMetaData&(1<<GRAPHFLAGS_ALPHABLEND) ? true : false); }
	bool IsAlphaTest(void) const  { return (iMetaData&(1<<GRAPHFLAGS_ALPHATEST) ? true : false); }
	bool IsSkinned(void) const  { return (iMetaData&(1<<GRAPHFLAGS_SKINNED) ? true : false); }

	unsigned int GetEntryIndex(void) const  { return  (iMetaData&(0xffffffff<<GRAPHFLAGS_ENTRYINDEX))>>GRAPHFLAGS_ENTRYINDEX; }
	unsigned int GetMetaData(void) const  { return iMetaData; }

	unsigned int iMetaData;						//!<  graph packed metadata
};

/***************************************************************************************************
*
*	CLASS			HSDictionaryResourceEntry
*
*	DESCRIPTION		This class represents a graph entry in the dictionary file
*
***************************************************************************************************/

class HSDictionaryResourceEntry
{
public:		
	FwHashedString  hash;						//!<  graph hash name
	unsigned int vSharedIndex;					//!<  if this vertex shader is shared with another graph this index is != -1
	unsigned int vpOffset;						//!<  vertex shader offset from the beginning of the dictionary
	unsigned int vpSize;						//!<  vertex shader size (in bytes)
	unsigned int fSharedIndex;					//!<  if this fragment shader is shared with another graph this index is != -1
	unsigned int fpOffset;						//!<  fragment shader offset from the beginning of the dictionary
	unsigned int fpSize;						//!<  fragment shader size (in bytes)
	HSGraphMetadata oMetadata;					//!<  graph metadata
};


// max number of slots per material template 
#define MAX_SLOT 4


/***************************************************************************************************
*
*	CLASS			HSMaterialResourceEntry
*
*	DESCRIPTION		This class represents a material entry in the dictionary file
*
***************************************************************************************************/

class HSMaterialResourceEntry
{
public:
	unsigned char name[32];						//!< material name
	FwHashedString hash;							//!< material hash name
	HSGraphMetadata m_aoMetaData[3*MAX_SLOT];	//!< graphs metadata (one graph per material slot)
};

/***************************************************************************************************
*
*	CLASS			HSMaterialResourceEntry
*
*	DESCRIPTION		This class represents a graph entry in the runtime dictionary
*
***************************************************************************************************/

class HSDictionaryEntry
{
public:
	CHashedString hash;							//!< graph name
	unsigned int vpIndex;						//!< vertex shader index in the runtime dictionary
	unsigned int fpIndex;						//!< fragment shader index in the runtime dictionary
	HSGraphMetadata oMetadata;					//!< graph metadata
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

	CMaterial();

	//! Resolves pointer offsets after loading and loads the shader itself.
	void Load(const HSMaterialResourceEntry* pEntry, const CShaderDictionary* dict);

	//! Gets the material name.
	const char* GetName() const { return m_pcName; }

	//! Gets hash name
	CHashedString GetHashName() const;
	

	//! Gets the template name.
	char* GetTemplateName() { return m_pcTemplateName; }

	// see if this material is supposed to be alpha blended
	bool IsAlphaBlended() const { return !!(m_Flags & MATERIAL_ALPHA_BLENDED); }

	// see if this material is supposed to be alpha tested
	bool IsAlphaTested() const { return !!(m_Flags & MATERIAL_ALPHA_TESTED); }

	//! Gets the number of bindings needed to bind this material to a mesh.
	int GetNumGraphs() const { return m_iNumGraphs; }

	// Techniques don't make sense anymore but I'm keeping the technique index anyway so we don't have to change
	// a lot of code..
	const CShaderGraph* GetBasicGraph(int iTechniqueIndex, VERTEXSHADER_TRANSFORM_TYPE type) const;
	const CShaderGraph* GetDepthWriteGraph(VERTEXSHADER_TRANSFORM_TYPE type) const;

private:

	CHashedString hashName;						//!< The material hash name.
	const char* m_pcName;						//!< The material name.
	char m_pcTemplateName[32];					//!< The implementation template to use.
	unsigned int m_Flags;						//!< flags MATERIAL_FLAG_TYPE
		
	int m_iNumGraphs;							//!< The number of graphs in the material.
	CShaderGraph m_aoGraphs[GRAPHSLOT_MAX];		//!< The graphs array for this material.

	friend class CBasicShaders;					//!< Allows the fallback shaders to be created directly.
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

	//! Create a fake dictionary from some debug shaders
	static CShaderDictionary* Create( DebugShader** pDebugShaders, const char* pMaterialName );

	//! Deallocate shader dictionary
	static void Destroy(CShaderDictionary* pDict);

	//! Returns the number of vertex and pixel shaders in the dictionary.
	int GetNumShaders() const { return 2*m_iNumGraphs; }

	//! Returns the number of materias in the dictionary.
	int GetNumMaterials() const { return m_iNumMaterials; }

	unsigned int GetDictionaryVersion(void) const { return (m_uiVersionID & 0xFF); }

	//! Finds a vertex shader by name.
	Shader* FindShader(const char* pcName) const;

	//! Finds a vertex shader by graph index
	Shader* FindVertexShader(unsigned int index) const;

	//! Finds a fragment shader by graph index
	Shader* FindFragmentShader(unsigned int index) const;

	//! Finds a material by name.
	const CMaterial* FindMaterial(CHashedString const& obHash) const;

private:
	CShaderDictionary( uint8_t *dictionary_data );			//!< Private to force construction using the Load function.
	CShaderDictionary( DebugShader** pDebugShaders, const char* pMaterialName ); //!< Private to force construction using the Load function.
	~CShaderDictionary();			//!< Private to force destruction using Destroy function.

	u_int m_uiVersionID;			//!< The dictionary version ID.
	CHashedString m_obNameHash;	//!< The name hash, filled in from the filename on load.
	
	uint8_t *m_pDictionaryData;
	
	int m_iNumGraphs;				//!< The number of graphs in the dictionary.
	int	m_iNumShaders;				//!< The number of shaders in the dictionary.
    
	CScopedArray< HSDictionaryEntry > m_pEntries;					//!< An array of dictionary entries.
	CScopedArray< SHADER_STREAM_BINDING > m_pstStreamBindings;		//!< Dictionary stream bindings..
	CScopedArray< SHADER_PROPERTY_BINDING > m_pstPropertyBindings; //!< Dictionary property bindings..
	CScopedArray< SHADER_TEXTURE_BINDING > m_pstTextureBindings;	//!< Dictionary texture bindings..
	CScopedArray< Shader > m_pobShaders;							//!< Shaders..
	CScopedArray< CMaterial > m_pobMaterials;						//!< Shaders..

	DebugShader*	m_aobDebugShader[24];							//!< This array is filled only we create a fake dictionary

	int m_iNumMaterials;			//!< The number of materials in the dictionary.
};


#endif // ndef _MATERIAL_PS3_H
