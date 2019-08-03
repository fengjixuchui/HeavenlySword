//--------------------------------------------------
//!
//!	\file clump_ps3.h
//!
//--------------------------------------------------
#if !defined( GFX_CLUMP_PS3_H )
#define GFX_CLUMP_PS3_H

#ifndef _NO_DBGMEM_OR_RELEASE
#	define CLUMP_USE_DEBUG_TAG
#endif

//--------------------------------------------------
//!
//!	ClumpHeader_RuntimeDataPS3
//! Allocated structure indicates the state of play
//! with a given clump header. Could be replaced 
//! with additional room in the clump header....
//!
//--------------------------------------------------
class ClumpHeader_RuntimeDataPS3
{
public:
	ClumpHeader_RuntimeDataPS3( const char* pClumpName ) :
		m_bAllocatedAsComplete(true),
		m_bHasVRAMResources(true),
		m_bHasTexResources(false)
	{
		// at the moment we use this to fake mesh data
		// loading, will dissapear again when async loading done
#ifdef CLUMP_USE_DEBUG_TAG
		ntAssert_p( pClumpName, ("Must provide a valid clump name here") );

		int len = strlen( pClumpName );
		m_pDebugTag = NT_NEW_CHUNK(Mem::MC_DEBUG) char[len+1];
		strcpy( m_pDebugTag, pClumpName );
#endif
	}

	~ClumpHeader_RuntimeDataPS3()
	{
#ifdef CLUMP_USE_DEBUG_TAG
		NT_DELETE_ARRAY_CHUNK( Mem::MC_DEBUG, m_pDebugTag );
#endif
	}

	char*	m_pDebugTag;

	// mark how much of a header we really are.
	bool	m_bAllocatedAsComplete;
	bool	m_bHasVRAMResources;
	bool	m_bHasTexResources;

	size_t	m_headerSize;
	size_t	m_resourceSize;

	char*	m_CPUMeshData;
};

#endif // GFX_CLUMP_PS3_H
