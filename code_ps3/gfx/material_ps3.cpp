
#include "gfx/material.h"
#include "gfx/shader.h"
#include "gfx/renderer_debug_settings_ps3.h"
#include "gfx/renderer.h"
#include "heresy/heresy_capi.h"

/***************************************************************************************************
*
*	FUNCTION		CMaterial::CMaterial
*
*	DESCRIPTION		Creates an empty material.
*
***************************************************************************************************/

CMaterial::CMaterial():
	m_pcName(0),
	m_Flags(0)
{
	// nothing
}

/***************************************************************************************************
*
*	FUNCTION		CMaterial::Load
*
*	DESCRIPTION		Fixes up the offset pointers based on the location of the head of the 
*					dictionary it was loaded from.
*
***************************************************************************************************/
void CMaterial::Load(const HSMaterialResourceEntry* pEntry, const CShaderDictionary* dict)
{
	NT_MEMCPY((void*)m_pcTemplateName, pEntry->name, 32);
	hashName = pEntry->hash;
	unsigned int index = GRAPHSLOT_BASIC;

	if ( strstr(m_pcTemplateName, "_lerp" ) )
	{
		m_Flags |= MATERIAL_ALPHA_BLENDED;
	}

	if ( dict->GetDictionaryVersion() > 1 )
	{
		m_Flags |= MATERIAL_DEBUG_LAYER;
	}

	// this way of inserting graph into material's slot is not very elegant but I'm keeping it that way 'till all
	// the materials/dictionary code has not reached a stable configuration (Marco)
	for (int i = 0; i < 3*MAX_SLOT; i++)
	{
		if (pEntry->m_aoMetaData[i].IsBasicTemplate())
		{
			if (pEntry->m_aoMetaData[i].IsDepthHaze())
			{
				if (pEntry->m_aoMetaData[i].IsReceiveShadow())
				{
					pEntry->m_aoMetaData[i].IsSkinned() ? index = GRAPHSLOT_RECEIVESHADOW_DEPTHHAZE_SKINNED : index = GRAPHSLOT_RECEIVESHADOW_DEPTHHAZE;
				}
				else
				{
					pEntry->m_aoMetaData[i].IsSkinned() ? index = GRAPHSLOT_BASIC_DEPTHHAZE_SKINNED : index = GRAPHSLOT_BASIC_DEPTHHAZE;
				}
			}
			else
			{
				if (pEntry->m_aoMetaData[i].IsReceiveShadow())
				{
					pEntry->m_aoMetaData[i].IsSkinned() ? index = GRAPHSLOT_RECEIVESHADOW_SKINNED : index = GRAPHSLOT_RECEIVESHADOW ;
				}
				else
				{
					pEntry->m_aoMetaData[i].IsSkinned() ? index = GRAPHSLOT_BASIC_SKINNED : index = GRAPHSLOT_BASIC;
				}
			}
	
			m_aoGraphs[index].SetVertexShader(dict->FindVertexShader(pEntry->m_aoMetaData[i].GetEntryIndex()));
			m_aoGraphs[index].SetFragmentShader(dict->FindFragmentShader(pEntry->m_aoMetaData[i].GetEntryIndex()));

		}
		else if (pEntry->m_aoMetaData[i].IsShadowTemplate())
		{
			if (pEntry->m_aoMetaData[i].IsDepthWrite())
			{
				pEntry->m_aoMetaData[i].IsSkinned() ? index = GRAPHSLOT_SHADOW_DEPTHWRITE_SKINNED : index = GRAPHSLOT_SHADOW_DEPTHWRITE;
   			}
			// receive pass was removed, that's just an hack to store reused its ahader slots for debugging purposes
			else if (pEntry->m_aoMetaData[i].IsReceiveShadow())
			{
				pEntry->m_aoMetaData[i].IsSkinned() ? index = GRAPHSLOT_DEBUG_LAYER2 :  index = GRAPHSLOT_DEBUG_LAYER1;
    		}
			else
			{
				ntAssert("Error: Unknown material template!\n");
			}
			             
			m_aoGraphs[index].SetVertexShader(dict->FindVertexShader(pEntry->m_aoMetaData[i].GetEntryIndex()));
			m_aoGraphs[index].SetFragmentShader(dict->FindFragmentShader(pEntry->m_aoMetaData[i].GetEntryIndex()));

		}
		else
		{
			ntAssert("Error: Unknown material template!\n");
		}

#ifdef COLLECT_SHADER_STATS
        Shader* vs = m_aoGraphs[index].GetVertexShader();
        vs -> m_material = (CMaterial*)this;
        vs -> m_index = index;
        vs -> m_slot = i;

        Shader* ps = m_aoGraphs[index].GetFragmentShaderNoAlloc();
        ps -> m_material = (CMaterial*)this;
        ps -> m_index = index;
        ps -> m_slot = i;
#endif


	}
}

/***************************************************************************************************
*
*	FUNCTION		CMaterial::GetBasicShaders
*
*	DESCRIPTION		Get vertex and fragment shaders from a basic graph
*					
*
***************************************************************************************************/
const CShaderGraph* CMaterial::GetBasicGraph(int iTechniqueIndex, VERTEXSHADER_TRANSFORM_TYPE type) const
{
	unsigned int slot;
	if ( (m_Flags & MATERIAL_DEBUG_LAYER) && (CRendererSettings::iDebugLayer > 0))
	{
		slot = static_cast<unsigned int>(GRAPHSLOT_DEBUG_LAYER1 + CRendererSettings::iDebugLayer - 1);
	}
	else
	{
		// get depth haze flag
		unsigned int iIndex = (iTechniqueIndex & 0x2)>>1; 
		// get skinning flag
		if (type == VSTT_SKINNED) iIndex += 2;
		// get shadop mapping flag
		iIndex += (iTechniqueIndex&0x1)<<2;

		slot = static_cast<unsigned int>(GRAPHSLOT_BASIC) + iIndex;
	}

	return &m_aoGraphs[slot];
}

/***************************************************************************************************
*
*	FUNCTION		CMaterial::GetDepthWriteShaders
*
*	DESCRIPTION		Get vertex and fragment shaders from a depth write graph
*					
*
***************************************************************************************************/
const CShaderGraph* CMaterial::GetDepthWriteGraph(VERTEXSHADER_TRANSFORM_TYPE type) const
{
	GRAPH_SLOT_TYPE slotType;
	switch (type)
	{
		case VSTT_BASIC:
			slotType = GRAPHSLOT_SHADOW_DEPTHWRITE;
		break;

		case VSTT_SKINNED:
			slotType = GRAPHSLOT_SHADOW_DEPTHWRITE_SKINNED;
		break;

		default:
			slotType = GRAPHSLOT_SHADOW_DEPTHWRITE;
		break;
	}
	unsigned int slot = static_cast<unsigned int>(slotType);
	
	return &m_aoGraphs[slot];
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
	ntAssert_p( File::Exists(pcFileName), ("Shader dictionary (%s) not present, exiting.\n", pcFileName) );

	uint8_t *dictionary_data = NULL;
	File dictionary_file;
	LoadFile_Chunk( pcFileName, File::FT_READ | File::FT_BINARY, Mem::MC_GFX, dictionary_file, &dictionary_data );
	return NT_NEW_CHUNK( Mem::MC_GFX ) CShaderDictionary( dictionary_data );
}


/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::Load
*
*	DESCRIPTION		Load a fake dictionary from debug shaders
*
***************************************************************************************************/

CShaderDictionary* CShaderDictionary::Create( DebugShader** pDebugShaders,  const char* pMaterialName )
{
	return NT_NEW_CHUNK( Mem::MC_GFX ) CShaderDictionary( pDebugShaders, pMaterialName );
}

/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::Destroy
*
*	DESCRIPTION		Unload the shader dictionary
*
***************************************************************************************************/

void CShaderDictionary::Destroy(CShaderDictionary* pDict)
{
	if ( pDict != NULL )
		NT_DELETE_CHUNK(Mem::MC_GFX, pDict );
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
CShaderDictionary::CShaderDictionary( uint8_t *dictionary_data )
{
	m_pDictionaryData = dictionary_data;

	// perform a check on the dictionary header
	HSDictionaryHeader const* pHeader = reinterpret_cast< HSDictionaryHeader const* >( m_pDictionaryData );
	ntError_p( memcmp( pHeader, "HSD", 3 ) == 0, ( "This resource doesn't look like a HS shader dictionary!" ) );

	// get version ID
	m_uiVersionID = pHeader->magic;

	// get graphs count
	m_iNumGraphs = (pHeader->graphcount)&0xffff;
	// get shaders count
	m_iNumShaders = pHeader->uniqueshadercount;
	// get materials count
	m_iNumMaterials = pHeader->materialcount;


	// allocate some memory
	m_pEntries.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) HSDictionaryEntry[m_iNumGraphs] );
	m_pobShaders.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) Shader[m_iNumShaders] );
	m_pobMaterials.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) CMaterial[m_iNumMaterials] );
	
	int iSize = m_iNumGraphs;
	int iShaderCounter = 0;
	int iStreamBindCount = 0;
	int iPropertyBindCount = 0;
	int iTextureBindCount = 0;

	ntPrintf("Dictionary loaded\n");
	ntPrintf("Graph count: %i - Shader count: %i\n", m_iNumGraphs, m_iNumGraphs*2);
	ntPrintf("Unique shader count: %i\n", m_iNumShaders);


	// load all the shaders
	u8 const* pBytes = reinterpret_cast< u8 const* >( m_pDictionaryData );
	HSDictionaryResourceEntry const* pEntries = reinterpret_cast< HSDictionaryResourceEntry const* >( pBytes + sizeof( HSDictionaryHeader ) );
	for( int i = 0; i < iSize; ++i )
	{
		// store the entry name
		m_pEntries[i].hash = pEntries[i].hash;

		// create the shaders
		if (pEntries[i].vSharedIndex == 0xffffffff)
		{
			// load this shader
			m_pobShaders[iShaderCounter].Load(pBytes + pEntries[i].vpOffset, pEntries[i].vpSize, SHADERTYPE_VERTEX);

			// get some facts about this shader
			iStreamBindCount += m_pobShaders[iShaderCounter].GetNumStreamBindings();
			iPropertyBindCount += m_pobShaders[iShaderCounter].GetNumPropertyBindings();
			iTextureBindCount += m_pobShaders[iShaderCounter].GetNumTextureBindings();

			m_pEntries[i].vpIndex = iShaderCounter;

			//  update shader count
			iShaderCounter++;
		}
		else
		{
			m_pEntries[i].vpIndex = pEntries[i].vSharedIndex;
		}


		if (pEntries[i].fSharedIndex == 0xffffffff)
		{
			// load this shader
			m_pobShaders[iShaderCounter].Load(pBytes + pEntries[i].fpOffset, pEntries[i].fpSize, SHADERTYPE_PIXEL);
	
			// get some facts about this shader
			iStreamBindCount += m_pobShaders[iShaderCounter].GetNumStreamBindings();
			iPropertyBindCount += m_pobShaders[iShaderCounter].GetNumPropertyBindings();
			iTextureBindCount += m_pobShaders[iShaderCounter].GetNumTextureBindings();

			m_pEntries[i].fpIndex = iShaderCounter;

			// NOTE: These shader aren't ready to go! You must call GetFragmentShader if your usign non heresy
			//		 or GetFragmentShaderNoAlloc() if you are using heresy (actually heresy doesn't care by NoAlloc if faster and less ram)

			//  update shader count
			iShaderCounter++;
		}
		else
		{
			m_pEntries[i].fpIndex = pEntries[i].fSharedIndex;
		}
		
		// store the graph metadata too!
		//m_pEntries[i].oMetadata = pEntries[i].oMetadata;
	}

	ntPrintf("Allocated shader count: %i\n", iShaderCounter);

	// get some mem for all the bindings structures
	m_pstStreamBindings.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) SHADER_STREAM_BINDING[iStreamBindCount] );
	m_pstPropertyBindings.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) SHADER_PROPERTY_BINDING[iPropertyBindCount] );
	m_pstTextureBindings.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) SHADER_TEXTURE_BINDING[iTextureBindCount] );

	iStreamBindCount = 0;
	iPropertyBindCount = 0;
	iTextureBindCount = 0;

	// generate shader bindings for this dictionary
	for (int i = 0; i < m_iNumShaders; i++)
	{
		m_pobShaders[i].GenerateStreamBindings(&m_pstStreamBindings[iStreamBindCount]);
		m_pobShaders[i].GeneratePropertyBindings(&m_pstPropertyBindings[iPropertyBindCount]);
		m_pobShaders[i].GenerateTextureBindings(&m_pstTextureBindings[iTextureBindCount]);

		// update counters
		iStreamBindCount += m_pobShaders[i].GetNumStreamBindings();
		iPropertyBindCount += m_pobShaders[i].GetNumPropertyBindings();
		iTextureBindCount += m_pobShaders[i].GetNumTextureBindings();
	}

	// Load Materials
	HSMaterialResourceEntry const* pMatEntries = reinterpret_cast< HSMaterialResourceEntry const* >( pBytes + sizeof( HSDictionaryHeader )
		 + m_iNumGraphs*sizeof(HSDictionaryResourceEntry) );

	for (int i=0; i < m_iNumMaterials; i++)
	{
		m_pobMaterials[i].Load(&pMatEntries[i], this);
	}
}


/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::CShaderDictionary
*
*	DESCRIPTION		Create a fake dictionary, get shader from debug shaders
*
***************************************************************************************************/
CShaderDictionary::CShaderDictionary( DebugShader** pDebugShaders, const char* pMaterialName )
{
	m_pDictionaryData = NULL;

	// get version ID
	m_uiVersionID = 2;

	// get graphs count
	m_iNumGraphs = 12;
	// get shaders count
	m_iNumShaders = 24;
	// get materials count
	m_iNumMaterials = 1;

	// allocate some memory
	m_pEntries.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) HSDictionaryEntry[m_iNumGraphs] );
	m_pobShaders.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) Shader[m_iNumShaders] );
	m_pobMaterials.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) CMaterial[m_iNumMaterials] );
	
	int iStreamBindCount = 0;
	int iPropertyBindCount = 0;
	int iTextureBindCount = 0;

	ntPrintf("Dictionary loaded\n");
	ntPrintf("Graph count: %i - Shader count: %i\n", m_iNumGraphs, m_iNumGraphs*2);
	ntPrintf("Unique shader count: %i\n", m_iNumShaders);

	
	// load all the shaders
	for(  int i = 0; i < m_iNumGraphs; ++i )
	{
		unsigned int index = i<<1;

		ntError_p(  pDebugShaders[index]->GetType() == SHADERTYPE_VERTEX  , ( "FakeDictionaryCreation: This shader is not a vertex shader!" ) );
		// store shader name
		m_pEntries[index].hash = CHashedString( pDebugShaders[index]->GetName() );

		// load this shader
		m_aobDebugShader[index] = pDebugShaders[index];

		// get some facts about this shader
		iStreamBindCount += m_aobDebugShader[index]->GetNumStreamBindings();
		iPropertyBindCount += m_aobDebugShader[index]->GetNumPropertyBindings();
		iTextureBindCount += m_aobDebugShader[index]->GetNumTextureBindings();

		m_pEntries[i].vpIndex = index;

		index++;
		ntError_p(  pDebugShaders[index]->GetType() == SHADERTYPE_PIXEL  , ( "FakeDictionaryCreation: This shader is not a vertex shader!" ) );
		// store shader name
		m_pEntries[index].hash = CHashedString( pDebugShaders[index]->GetName() );

		// load this shader
		m_aobDebugShader[index] = pDebugShaders[index];

		// get some facts about this shader
		iStreamBindCount += m_aobDebugShader[index]->GetNumStreamBindings();
		iPropertyBindCount += m_aobDebugShader[index]->GetNumPropertyBindings();
		iTextureBindCount += m_aobDebugShader[index]->GetNumTextureBindings();

		m_pEntries[i].fpIndex = index;

	}

//	ntPrintf("Allocated shader count: %i\n", iShaderCounter);

	// get some mem for all the bindings structures
	m_pstStreamBindings.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) SHADER_STREAM_BINDING[iStreamBindCount] );
	m_pstPropertyBindings.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) SHADER_PROPERTY_BINDING[iPropertyBindCount] );
	m_pstTextureBindings.Reset( NT_NEW_CHUNK( Mem::MC_GFX ) SHADER_TEXTURE_BINDING[iTextureBindCount] );

	iStreamBindCount = 0;
	iPropertyBindCount = 0;
	iTextureBindCount = 0;

	// generate shader bindings for this dictionary
	for (int i = 0; i < m_iNumShaders; i++)
	{
		m_pobShaders[i].GenerateStreamBindings(&m_pstStreamBindings[iStreamBindCount]);
		m_pobShaders[i].GeneratePropertyBindings(&m_pstPropertyBindings[iPropertyBindCount]);
		m_pobShaders[i].GenerateTextureBindings(&m_pstTextureBindings[iTextureBindCount]);

		// update counters
		iStreamBindCount += m_pobShaders[i].GetNumStreamBindings();
		iPropertyBindCount += m_pobShaders[i].GetNumPropertyBindings();
		iTextureBindCount += m_pobShaders[i].GetNumTextureBindings();
	}


	HSMaterialResourceEntry fakeEntry;

	fakeEntry.hash = FwHashedString( pMaterialName );
	
	unsigned int nameLength = strlen(pMaterialName)+1;
	if (nameLength > 32)
		nameLength = 32;

	memset( &fakeEntry.m_aoMetaData, 0x0, sizeof(fakeEntry.m_aoMetaData) );
	
	for (unsigned int i = 0; i < 8; i++ )
		fakeEntry.m_aoMetaData[i].iMetaData |= 1<<HSGraphMetadata::GRAPHFLAGS_BASIC;
	for (unsigned int i = 8; i < 12; i++ )
		fakeEntry.m_aoMetaData[i].iMetaData |= 1<<HSGraphMetadata::GRAPHFLAGS_SHADOW;
	for (unsigned int i = 4; i < 8; i++ )
		fakeEntry.m_aoMetaData[i].iMetaData |= 1<<HSGraphMetadata::GRAPHFLAGS_RECEIVESHADOW;

	for (unsigned int i = 0; i < 3; i++ )
	{
		fakeEntry.m_aoMetaData[1+4*i].iMetaData |= 1<<HSGraphMetadata::GRAPHFLAGS_DEPTHHAZE;
		fakeEntry.m_aoMetaData[2+4*i].iMetaData |= 1<<HSGraphMetadata::GRAPHFLAGS_SKINNED;
		fakeEntry.m_aoMetaData[3+4*i].iMetaData |= (1<<HSGraphMetadata::GRAPHFLAGS_SKINNED) |  (1<<HSGraphMetadata::GRAPHFLAGS_DEPTHHAZE);
	}

	memcpy( &fakeEntry.name, pMaterialName, nameLength  );
	m_pobMaterials[0].Load(&fakeEntry, this);

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
	if ( m_pDictionaryData != NULL )
		NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, m_pDictionaryData );
}

/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::FindVertexShader
*
*	DESCRIPTION		
*
***************************************************************************************************/

Shader* CShaderDictionary::FindVertexShader(unsigned int index) const
{
	if ( m_pDictionaryData != NULL )
	{
		if (index < static_cast<unsigned int>(m_iNumGraphs))
		{
			return &m_pobShaders[m_pEntries[index].vpIndex];
		}
		
		ntAssert("Error: graph index is out of bound");
		return NULL;
	}
	else
	{
		if (index < static_cast<unsigned int>(m_iNumGraphs))
		{
			return static_cast<Shader*>(m_aobDebugShader[m_pEntries[index].vpIndex]);
		}
		
		ntAssert("Error: graph index is out of bound");
		return NULL;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::FindFragmentShader
*
*	DESCRIPTION		
*
***************************************************************************************************/

Shader* CShaderDictionary::FindFragmentShader(unsigned int index) const
{
	if ( m_pDictionaryData != NULL )
	{
		if (index < static_cast<unsigned int>(m_iNumGraphs))
		{
			return &m_pobShaders[m_pEntries[index].fpIndex];
		}
		
		ntAssert("Error: graph index is out of bound");
		return NULL;
	}
	else
	{
		if (index < static_cast<unsigned int>(m_iNumGraphs))
		{
			return static_cast<Shader*>(m_aobDebugShader[m_pEntries[index].fpIndex]);
		}
		
		ntAssert("Error: graph index is out of bound");
		return NULL;
	}
}



/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::GetHashName
*
*	DESCRIPTION		
*
***************************************************************************************************/

CHashedString CMaterial::GetHashName() const
{
	return hashName;
}	

/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::FindMaterial
*
*	DESCRIPTION		
*
***************************************************************************************************/

const CMaterial* CShaderDictionary::FindMaterial(CHashedString const& obHash) const
{
	for (int i=0; i < m_iNumMaterials; i++)
	{
		if (m_pobMaterials[i].GetHashName() == obHash) return &m_pobMaterials[i];
	}
	
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CShaderGraph stuff
///////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************************
*
*	FUNCTION		CShaderGraph::GetVertexShader
*
*	DESCRIPTION		< > 
*
***************************************************************************************************/

Shader* CShaderGraph::GetVertexShader(void) const
{
	return m_pVertexShader;
}

/***************************************************************************************************
*2
*	FUNCTION		CShaderGraph::GetVertexShaderNoAlloc
*
*	DESCRIPTION		< > 
*
***************************************************************************************************/

Shader* CShaderGraph::GetFragmentShaderNoAlloc(void) const
{
	return m_pFragmentShader;
}


/***************************************************************************************************
*2
*	FUNCTION		CShaderGraph::GetVertexShader
*
*	DESCRIPTION		< > 
*
***************************************************************************************************/

Shader* CShaderGraph::GetFragmentShader(void) const
{
	const GcShaderHandle pShader = m_pFragmentShader->GetPixelHandle();
	Heresy_PixelShader* pPixelProgram = (Heresy_PixelShader*) pShader->GetResource()->GetFragmentProgram();
	if( m_pFragmentShader->m_bNonHeresyFragmentCached == false)
	{
		// use our allocator instead of Ice's for user mo
		void* pShaderMemPtr = (void*) Renderer::Get().m_Platform.m_PixelShaderMainSpace.MemAlign( pPixelProgram->m_microcodeSize, 64 );
		NT_MEMCPY( pShaderMemPtr, ((const char*)pPixelProgram) + pPixelProgram->m_microcodeOffset, pPixelProgram->m_microcodeSize );
		const_cast<GcShader*>(pShader.Get())->SetDataAddress( pShaderMemPtr );
		m_pFragmentShader->m_bNonHeresyFragmentCached = true;
	}

	return m_pFragmentShader;
}

