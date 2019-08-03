
#include "gfx/materialbase.h"
#include "anim/hierarchy.h"
#include "core/exportstruct_clump.h"
#include "gfx/renderersettings.h"

MaterialInstanceBase::MaterialInstanceBase(bool bIsFX ):
	// properties
	m_iPropertyCount( 0 ),
	m_pDefaultProperties(0),
	m_iPropertyOverideTableRefCount(0),

	m_eBoundType(VSTT_BASIC),
	
	// bones
	m_pucBoneIndices( 0 ), 
	m_iNumberOfBonesUsed( 0 ),

	//flag
	m_bIsFX(bIsFX),
	m_bIsAlphaBlended(false),

	// vertex
	m_pVertexElements( 0 ), 
	m_iVertexElementCount( 0 ),
	// debug
	m_debugIndices(-1,-1,-1,-1)

{
	// nothing
}



MaterialInstanceBase::~MaterialInstanceBase()
{
	
}

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
void MaterialInstanceBase::SetBoneIndices( u_char const* pucBoneIndices, int iNumberOfBonesUsed )
{
	m_pucBoneIndices = pucBoneIndices;
	m_iNumberOfBonesUsed = iNumberOfBonesUsed;
}

/***************************************************************************************************
*
*	FUNCTION		CopySkinMatrices
*
*	DESCRIPTION		CPU skinning version of UploadSkinMatrices
*
***************************************************************************************************/

void MaterialInstanceBase::CopySkinMatrices( CHierarchy* pobHierarchy, CSkinMatrix* pBoneArray ) const
{
	// ensure the transforms are up to date on the hierarchy
	pobHierarchy->UpdateSkinMatrices();
	ntError_p( m_pucBoneIndices, ( "no bone indices set on skinned material instance" ) );

	// upload only the matrices used by this mesh
	for(int iBone = 0; iBone < m_iNumberOfBonesUsed; ++iBone)
	{
		// get the matrix index
		int iMatrixIndex = m_pucBoneIndices[iBone];

		// get the matrix for the given index
		const CSkinMatrix* pobBone = &pobHierarchy->GetSkinMatrixArray()[iMatrixIndex];

		NT_MEMCPY( &pBoneArray[iBone], pobBone, sizeof( CSkinMatrix ) );
	}
}







/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::SetPropertyTableOveride
*
*	DESCRIPTION		Copies the material property table from the mesh header if arg is true, else frees
*
***************************************************************************************************/

void MaterialInstanceBase::SetPropertyTableOveride( bool bCopyTable )
{
	if( bCopyTable )
	{
		// allocate the table
		if( m_iPropertyOverideTableRefCount++ == 0 )
			m_propertyOverideTable.Reset( NT_NEW_CHUNK(Mem::MC_GFX) CMaterialProperty[m_iPropertyCount] );

		// copy in the defaults
		NT_MEMCPY( 
			m_propertyOverideTable.Get(), 
			m_pDefaultProperties, 
			m_iPropertyCount * sizeof( CMaterialProperty )
		);

		// set the new properties binding map
		m_PropertiesMap.clear();
		for (int i=0; i < m_iPropertyCount; i++)
		{
			m_PropertiesMap.insert(ntstd::make_pair( m_pDefaultProperties[i].m_iPropertyTag, &m_pDefaultProperties[i] ));
		}

	}
	else
	{
		// deallocate the table if the refcount hits zero
		ntError_p( m_iPropertyOverideTableRefCount > 0, ( "invalid property table refcount" ) );
		if( --m_iPropertyOverideTableRefCount == 0 )
			m_propertyOverideTable.Reset();
	}
}



/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::SetPropertyTable
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

void MaterialInstanceBase::SetPropertyTable( CMaterialProperty const* pobProperties, int iPropertyCount )
{
	// set the new property table
	m_pDefaultProperties = pobProperties;
	m_iPropertyCount = iPropertyCount;

	// set the new properties binding map
	m_PropertiesMap.clear();
	for (int i=0; i < iPropertyCount; i++)
	{
		m_PropertiesMap.insert(ntstd::make_pair( pobProperties[i].m_iPropertyTag, &pobProperties[i] ));
	}


	// adjust the override if necessary
	if( m_iPropertyOverideTableRefCount > 0 )
	{
		m_propertyOverideTable.Reset( NT_NEW_CHUNK(Mem::MC_GFX) CMaterialProperty[m_iPropertyCount] );
		NT_MEMCPY( 
			m_propertyOverideTable.Get(), 
			m_pDefaultProperties, 
			m_iPropertyCount * sizeof( CMaterialProperty )
		);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CGameMaterialInstance::GetPropertyTableSize
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

int MaterialInstanceBase::GetPropertyTableSize() const
{	
	return m_iPropertyCount;
}

CMaterialProperty const* MaterialInstanceBase::GetPropertyTable( void ) const
{
	if(m_propertyOverideTable)
	{
		return m_propertyOverideTable.Get();
	}
	else
	{
		return m_pDefaultProperties;
	}
}

CMaterialProperty const* MaterialInstanceBase::GetPropertyDefaultTable( void ) const
{
	return m_pDefaultProperties;
}

CMaterialProperty* MaterialInstanceBase::GetPropertyOverideTable( void ) const
{
	return m_propertyOverideTable.Get();
}

void MaterialInstanceBase::SetDebugIndices(Pixel3 debugIndices, int iMesh)
{
	m_debugIndices[3] = iMesh;
	for(int i = 0 ; i < 3 ; i++ )
	{
		if(debugIndices[i]==-1)
		{
			m_debugIndices[i] = -1;
		}
		else
		{
			m_debugIndices[i] = -1;
			for(int iLook = 0 ; iLook < m_iNumberOfBonesUsed ; iLook++ )
			{
				if(debugIndices[i]==static_cast<int>(m_pucBoneIndices[iLook]))
				{
					ntAssert(m_debugIndices[i]==-1);
					m_debugIndices[i] = iLook;
				}
			}
			if(m_debugIndices[i]>=-1)
			{
				ntPrintf("MaterialInstanceBase::SetDebugIndices: bad indices: %d\n", debugIndices[i]);
				//CerrLog()<<"MaterialInstanceBase::SetDebugIndices: bad indices: "<<debugIndices[i]<<DebugStream::flush;
			}
		}
	}
}

// set vertex element
void MaterialInstanceBase::SetVertexElement(CMeshVertexElement* pVertexElements,int iVertexElementCount)
{
	m_pVertexElements=pVertexElements;
	m_iVertexElementCount=iVertexElementCount;

	
	// check for skinning data on the mesh
	for( int iElement = 0; iElement < m_iVertexElementCount; ++iElement )
	{
		if( m_pVertexElements[iElement].m_eStreamSemanticTag == STREAM_BLENDWEIGHTS )
		{
			m_eBoundType = VSTT_SKINNED;
			break;
		}
	}

	if ( (!CRendererSettings::bEnableSkinning) && (m_eBoundType == VSTT_SKINNED) )
		m_eBoundType = VSTT_BASIC;
}

