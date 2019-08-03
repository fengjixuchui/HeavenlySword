
#include "blendshapes/blendshapes.h"
//#include "core/hash.h"
#include "core/exportstruct_clump.h"
#include "blendshapes/blendshapes_managers.h"


// ----------------------------------------------------------------- //
//							BSSet									 //
// ----------------------------------------------------------------- //



BSSet::BSSet( BSClumpHeaderPtr_t pBSClumpHeader, CEntity* pEntity )
:	m_pEntity( pEntity ),
	m_pBSClumpHeader( pBSClumpHeader ),
	m_aMeshBS( NT_NEW_ARRAY_CHUNK (Mem::MC_PROCEDURAL) MeshBSSet[m_pBSClumpHeader->m_numOfBSMeshHeaders] ),
	m_aTargetWeights( NT_NEW_ARRAY_CHUNK(Mem::MC_PROCEDURAL) float[m_pBSClumpHeader->m_numOfBSTargets] )
{
	InitBSMeshSetArray();
	ResetWeights();
}


BSSet::BSSet()
:	m_pEntity( 0 ),
	m_pBSClumpHeader(0),
	m_aMeshBS(0),
	m_aTargetWeights(0)
{

}


void BSSet::InitBSMeshSetArray( void ) 
{
	for ( u_int iMesh = 0; iMesh < GetNumOfMeshBSSets(); ++iMesh )
	{
		const BSMeshHeader* pMeshHeader = m_pBSClumpHeader->m_pBSMeshHeaders + iMesh;
		m_aMeshBS[ iMesh ] = MeshBSSet( pMeshHeader, this );
	}
}

void BSSet::DestroyBSMeshSetArray( void )
{
	// nothing to do here now
}

BSSet::~BSSet()
{
	if ( m_aTargetWeights )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_PROCEDURAL, m_aTargetWeights );
		m_aTargetWeights = 0;
	}

	if ( m_aMeshBS )
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_PROCEDURAL, m_aMeshBS );
		m_aMeshBS = 0;
	}

	//if ( m_pBSClumpHeader )
	//{
	//	BSClumpManager::Get().Unload( m_pBSClumpHeader );
	//	m_pBSClumpHeader = 0;
	//}
}



// ----------------------------------------------------------------- //
//							MeshBSSet								 //
// ----------------------------------------------------------------- //

MeshBSSet::MeshBSSet( const BSMeshHeader* pHeader, BSSet* pParent  )
:	m_pParent( pParent ),
	m_pHeader( pHeader )
	
{
	// empty
}

