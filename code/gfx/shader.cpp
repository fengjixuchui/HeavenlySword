/***************************************************************************************************
*
*	$Header:: /game/shader.cpp 17    13/08/03 10:39 Simonb                                         $
*
*
*
*	CHANGES
*
*	9/5/2003	SimonB	Created
*
***************************************************************************************************/

#include "gfx/shader.h"
#include "gfx/material.h"
#include "gfx/hardwarecaps.h"

/***************************************************************************************************
*
*	FUNCTION		ShaderManager::ShaderManager
*
*	DESCRIPTION		Loads the default dictionary.
*
***************************************************************************************************/


 const static char* BSSKIN_DICTIONARY = "data/bsskin.dict";

ShaderManager::ShaderManager():m_bForceShadersReload(false)
{
#if defined(PLATFORM_PC)
	char acFileName[ MAX_PATH ];
	Util::GetFiosFilePath( "data/shaders.dict", acFileName );
	LoadDictionary(acFileName);
#else
	GfxStringList m_dicName;


	////////////////////////////////////////////////
	////////////////////////////////////////////////
//	m_dicName.push_back("data/test.dict");
//	m_dicName.push_back("data/lambert.dict");
//	m_dicName.push_back("data/metallic.dict");
//	m_dicName.push_back("data/phong.dict");
//	m_dicName.push_back("data/uber.dict");
//	m_dicName.push_back("data/unlit.dict");
	m_dicName.push_back("data/glass.dict");
	m_dicName.push_back("data/debugNEW.dict");
	m_dicName.push_back("data/doublesided.dict");
//	m_dicName.push_back("data/hair.dict");
#ifdef _SPEEDTREE
//	m_dicName.push_back("data/speedtree.dict");
#endif // _SPEEDTREE

	m_dicName.push_back( BSSKIN_DICTIONARY );
	////////////////////////////////////////////////
	////////////////////////////////////////////////
	
	char acFileName[ MAX_PATH ];
	for(GfxStringList::iterator it = m_dicName.begin();
		it != m_dicName.end();
		++it)
	{
		Util::GetFiosFilePath_Platform(it->c_str(), acFileName );
		LoadDictionary(acFileName);
	}


	


#endif
}

/***************************************************************************************************
*
*	FUNCTION		ShaderManager::ReloadShaders
*
*	DESCRIPTION		Unloads and reload dictionaries.
*
***************************************************************************************************/
void ShaderManager::ReloadShaders(void)
{
	// unload all dictionaries
	while(!m_obDictionaries.empty())
	{
		CShaderDictionary::Destroy( m_obDictionaries.back() );
		m_obDictionaries.pop_back();
	}

	GfxStringList dummyList = m_obDictFileName;
	m_obDictFileName.clear();

	while(!dummyList.empty())
	{
		LoadDictionary( dummyList.back().c_str() );
		dummyList.pop_back();
	}
}

/***************************************************************************************************
*
*	FUNCTION		ShaderManager::~ShaderManager
*
*	DESCRIPTION		Unloads all loaded dictionaries.
*
***************************************************************************************************/

ShaderManager::~ShaderManager()
{
	// unload all dictionaries
	while(!m_obDictionaries.empty())
	{
		CShaderDictionary::Destroy( m_obDictionaries.back() );
		m_obDictionaries.pop_back();
	}
}

/***************************************************************************************************
*
*	FUNCTION		ShaderManager::LoadDictionary
*
*	DESCRIPTION		Loads a dictionary from a file on disk. This dictionary is added to the end of
*					the list, and as such shaders are resolved in this dictionary first.
*
***************************************************************************************************/

void ShaderManager::LoadDictionary(const char* pcFileName)
{
	ntPrintf(" \"%s\" ", pcFileName);
	CShaderDictionary* pobDictionary = CShaderDictionary::Create(pcFileName);
	m_obDictionaries.push_back(pobDictionary);
	m_obDictFileName.push_back(ntstd::String(pcFileName));
}




/***************************************************************************************************
*
*	FUNCTION		ShaderManager::LoadDictionary
*
*	DESCRIPTION		Loads a fake dictionary
*
***************************************************************************************************/
#ifdef PLATFORM_PS3
void ShaderManager::LoadDictionary(DebugShader** pDebugShaders, const char* pcMaterialName)
{
	CShaderDictionary* pobDictionary = CShaderDictionary::Create(pDebugShaders, pcMaterialName);
	m_obDictionaries.push_back(pobDictionary);
//	m_obDictFileName.push_back(ntstd::String(pcMaterialName));
}
#endif

/***************************************************************************************************
*
*	FUNCTION		ShaderManager::GetNumShaders
*
*	DESCRIPTION		Returns the total number of shaders in the manager.
*
***************************************************************************************************/

int ShaderManager::GetNumShaders() const
{
	int iCount = 0;
	for(ShaderDictionaryList::const_iterator obIter = m_obDictionaries.begin(); obIter != m_obDictionaries.end(); ++obIter)
		iCount += (*obIter)->GetNumShaders();
	return iCount;
}

/***************************************************************************************************
*
*	FUNCTION		ShaderManager::GetNumMaterials
*
*	DESCRIPTION		Returns the total number of materials shaders in the manager.
*
***************************************************************************************************/

int ShaderManager::GetNumMaterials() const
{
	int iCount = 0;
	for(ShaderDictionaryList::const_iterator obIter = m_obDictionaries.begin(); obIter != m_obDictionaries.end(); ++obIter)
		iCount += (*obIter)->GetNumMaterials();
	return iCount;
}

/***************************************************************************************************
*
*	FUNCTION		ShaderManager::FindShader
*
*	DESCRIPTION		Finds a shader by name. Dictionaries loaded later are searched first.
*
***************************************************************************************************/
Shader* ShaderManager::FindShader(const char* pcName) const
{
	for(ShaderDictionaryList::const_reverse_iterator obIter = m_obDictionaries.rbegin();; ++obIter)
	{
		if(obIter == m_obDictionaries.rend())
			return 0;
		Shader* pobShader = (*obIter)->FindShader(pcName);
		if(pobShader != 0)
			return pobShader;
	}
}

/***************************************************************************************************
*
*	FUNCTION		ShaderManager::FindMaterial
*
*	DESCRIPTION		Finds a material by hash. Dictionaries loaded later are searched first.
*
***************************************************************************************************/

const CMaterial* ShaderManager::FindMaterial(CHashedString const& obHash) const
{
	for(ShaderDictionaryList::const_reverse_iterator obIter = m_obDictionaries.rbegin();; ++obIter)
	{
		if(obIter == m_obDictionaries.rend())
		{
			return 0;
		}
		const CMaterial* pobMaterial = (*obIter)->FindMaterial(obHash);
		if(pobMaterial != 0)
			return pobMaterial;
	}
}

