#include "gfx/material.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/hardwarecaps.h"
#include "core/file.h"


/***************************************************************************************************
*
*	FUNCTION		CMaterial::CMaterial
*
*	DESCRIPTION		Creates an empty material.
*
***************************************************************************************************/

CMaterial::CMaterial():
	m_pcName(0),
	m_pcTemplateName(0),
	m_Flags(0),
	m_iNumTechniques(0),
	m_aobTechniques(0)

{
	// nothing
}

/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::~CShaderDictionary
*
*	DESCRIPTION		Unloads the shaders in this dictionary.
*
***************************************************************************************************/

CShaderDictionary::~CShaderDictionary()
{
	// unload the shaders
//	for(int iShader = 0; iShader < m_iNumShaders; ++iShader)
  //      m_pobShaders[iShader].Unload();
}


/***************************************************************************************************
*
*	FUNCTION		CMaterial::Load
*
*	DESCRIPTION		Fixes up the offset pointers based on the location of the head of the 
*					dictionary it was loaded from.
*
***************************************************************************************************/
void CMaterial::Load(const CShaderDictionary* pobHead)
{
	ResolveOffset(m_pcName, pobHead);
	ResolveOffset(m_pcTemplateName, pobHead);
	ResolveOffset(m_aobTechniques, pobHead);
	for(int iSlot = 0; iSlot < m_iNumTechniques; ++iSlot)
	{ 
		for(int iShader = 0; iShader < SHADERSLOT_MAX; ++iShader)
			ResolveOffset(m_aobTechniques[iSlot].apobShaders[iShader], pobHead);
	}

/*
	for(int iSlot = 0; iSlot < m_iNumTechniques; ++iSlot)
	{
		if ( strcmp( m_aobTechniques[iSlot].apobShaders[1]->GetName(), "lambert_vs_false_false_false_true_false_false_false_false_false_jambert1") == 0 )
		{
			ntPrintf("Overiding: lambert_vs_false_false_false_true_false_false_false_false_false_jambert1\n");

			Shader* pOldShader = m_aobTechniques[iSlot].apobShaders[1];
			CDebugShader* pNewShader = NT_NEW_CHUNK(Mem::MC_GFX) CDebugShader;
			pNewShader->SetFILEFunction( SHADERTYPE_VERTEX, "shaders\\jambert_overides.hlsl", "lambert_vs_000100000_jambert1", "vs_2_0" );	
			pNewShader->SetName("lambert_vs_000100000_jambert1");

			pNewShader->SetStreamBindings( pOldShader->GetStreamBinding( 0 ), pOldShader->GetNumStreamBindings() );
			pNewShader->SetPropertyBindings( pOldShader->GetPropertyBinding( 0 ), pOldShader->GetNumPropertyBindings() );
			pNewShader->SetTextureBindings( pOldShader->GetTextureBinding( 0 ), pOldShader->GetNumTextureBindings() );

			m_aobTechniques[iSlot].apobShaders[1] = pNewShader;
		}

		if ( strcmp( m_aobTechniques[iSlot].apobShaders[1]->GetName(), "lambert_vs_false_true_true_true_false_false_false_false_false_jambert1") == 0 )
		{
			ntPrintf("Overiding: lambert_vs_false_true_true_true_false_false_false_false_false_jambert1\n");

			Shader* pOldShader = m_aobTechniques[iSlot].apobShaders[1];
			CDebugShader* pNewShader = NT_NEW_CHUNK(Mem::MC_GFX) CDebugShader;
			pNewShader->SetFILEFunction( SHADERTYPE_VERTEX, "shaders\\jambert_overides.hlsl", "lambert_vs_011100000_jambert1", "vs_2_0" );	
			pNewShader->SetName("lambert_vs_011100000_jambert1");

			pNewShader->SetStreamBindings( pOldShader->GetStreamBinding( 0 ), pOldShader->GetNumStreamBindings() );
			pNewShader->SetPropertyBindings( pOldShader->GetPropertyBinding( 0 ), pOldShader->GetNumPropertyBindings() );
			pNewShader->SetTextureBindings( pOldShader->GetTextureBinding( 0 ), pOldShader->GetNumTextureBindings() );

			m_aobTechniques[iSlot].apobShaders[1] = pNewShader;
		}

		if ( strcmp( m_aobTechniques[iSlot].apobShaders[1]->GetName(), "phong_vs_false_true_false_true_false_false_false_false_false_jambert1") == 0 )
		{
			ntPrintf("Overiding: phong_vs_false_true_false_true_false_false_false_false_false_jambert1\n");

			Shader* pOldShader = m_aobTechniques[iSlot].apobShaders[1];
			CDebugShader* pNewShader = NT_NEW_CHUNK(Mem::MC_GFX) CDebugShader;
			pNewShader->SetFILEFunction( SHADERTYPE_VERTEX, "shaders\\phong_overides.hlsl", "phong_vs_010100000_jambert1", "vs_2_0" );
			pNewShader->SetName("phong_vs_010100000_jambert1");

			pNewShader->SetStreamBindings( pOldShader->GetStreamBinding( 0 ), pOldShader->GetNumStreamBindings() );
			pNewShader->SetPropertyBindings( pOldShader->GetPropertyBinding( 0 ), pOldShader->GetNumPropertyBindings() );
			pNewShader->SetTextureBindings( pOldShader->GetTextureBinding( 0 ), pOldShader->GetNumTextureBindings() );

			m_aobTechniques[iSlot].apobShaders[1] = pNewShader;
		}
	}
*/
}


/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::CShaderDictionary
*
*	DESCRIPTION		This can only be called via Load and hence by placement new. So the function
*					assumes the memory behind this class is valid and loads all the shaders
*					from it.
*
***************************************************************************************************/

CShaderDictionary::CShaderDictionary()
{
	// check the version
	ntAssert_p(m_uiVersionID == MAKE_ID('s', 'd', '0', '5'), ("not current shader dictionary version"));

	// fix up the pointers
	ResolveOffset(m_pobShaders, this);
	ResolveOffset(m_pobMaterials, this);

	// load the shaders
	for(int iShader = 0; iShader < m_iNumShaders; ++iShader)
        m_pobShaders[iShader].Load(this);
	for(int iMaterial = 0; iMaterial < m_iNumMaterials; ++iMaterial)
		m_pobMaterials[iMaterial].Load(this);
}
/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::Load
*
*	DESCRIPTION		Loads a shader dictionary directly off disk into memory, then placement
*					news a dictionary object over that memory.
*
***************************************************************************************************/

CShaderDictionary* CShaderDictionary::Create(const char* pcFileName)
{
	File dict( pcFileName, File::FT_READ | File::FT_BINARY );
	ntAssert_p( dict.IsValid(), ("Shader dictionary (%s) not present, exiting.\n", pcFileName) );

	char* acStorage;
	dict.AllocateRead( &acStorage );

	// return a new dictionary using this storage
	return NT_PLACEMENT_NEW(acStorage) CShaderDictionary;
}

/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::Destroy
*
*	DESCRIPTION		Static destroy that matches our create
*
***************************************************************************************************/
void	CShaderDictionary::Destroy( CShaderDictionary* pDict )
{
	ntAssert_p( pDict, ("Must provide a vaild CShaderDictionary ptr here") );
	pDict->~CShaderDictionary();

	char* pData = (char*) pDict;
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, pData );
}

/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::FindMaterial
*
*	DESCRIPTION		Finds a material by hash. If no shader is found we return 0.
*
***************************************************************************************************/

const CMaterial* CShaderDictionary::FindMaterial(CHashedString const& obHash) const
{
	// do a linear search for the material
	for(int iMaterial = 0; iMaterial < m_iNumMaterials; ++iMaterial)
	{
		if(CHashedString(m_pobMaterials[iMaterial].GetName()) == obHash)
			return &m_pobMaterials[iMaterial];
	}
	// material not found
	return 0;
}
