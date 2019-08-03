//--------------------------------------------------
//!
//!	\file clump_pc.h
//! 
//!
//--------------------------------------------------
#if !defined( GFX_CLUMP_PC_H )
#define GFX_CLUMP_PC_H

#ifndef _GOLD_MASTER
#	define CLUMP_USE_DEBUG_TAG
#endif

//--------------------------------------------------
//!
//!	ClumpHeader_RuntimeDataPC
//! Allocated structure indicates the state of play
//! with a given clump header. Could be replaced 
//! with additional room in the clump header....
//!
//--------------------------------------------------
class ClumpHeader_RuntimeDataPC
{
public:
	ClumpHeader_RuntimeDataPC( const char* pClumpName ) :
		m_bAllocatedAsComplete(true),
		m_bHasVRAMResources(true),
		m_bHasTexResources(false)
	{
		UNUSED(pClumpName);

		// at the moment we use this to fake mesh data
		// loading, will dissapear again when async loading done
#ifdef CLUMP_USE_DEBUG_TAG
		ntAssert_p( pClumpName, ("Must provide a valid clump name here") );

		int len = strlen( pClumpName );
		m_pDebugTag = NT_NEW_CHUNK(Mem::MC_DEBUG) char[len+1];
		strcpy( m_pDebugTag, pClumpName );
#endif
	}

	~ClumpHeader_RuntimeDataPC()
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
};

#endif // GFX_CLUMP_PC_H
