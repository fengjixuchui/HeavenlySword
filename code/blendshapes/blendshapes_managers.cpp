
#include "blendshapes/blendshapes_managers.h"
#include "blendshapes/blendshapes_export.h"
#include "blendshapes/blendshapes_constants.h"
#include "core/file.h"

BSAnimExport* LoadBSAnim( void* pFileData, uint32_t fileSize, const char* pDebugTag );
BSClumpHeader* LoadBSClump( void* pFileData, uint32_t fileSize, const char* pDebugTag );

BSClumpHeader* BSClumpHeaderLoader::operator()( void* pFileData, uint32_t fileSize, const char* pDebugTag )
{
	return LoadBSClump( pFileData, fileSize, pDebugTag );
}

void BSClumpHeaderLoader::MakePlatformName( const char* pNeutralName, char* pPlatformName )
{
#ifndef _RELEASE
	// check for some very special case errors
	ntError_p( pNeutralName[0] != '\\', ("Clump path %s has a leading backslash, please remove.", pNeutralName) );
	ntError_p( pNeutralName[0] != '/', ("Clump path %s has a leading forwardslash, please remove.", pNeutralName) );
#endif

	// appends file system root and content directory
	Util::GetFiosFilePath_Platform( pNeutralName, pPlatformName );

#ifdef PLATFORM_PS3
	// we change our file extension to the correct one
	strcpy( strstr( pPlatformName, ".bsclump" ), ".bsclump_ps3" );
#endif
}

BSAnimExport* BSAnimHeaderLoader::operator()( void* pFileData, uint32_t fileSize, const char* pDebugTag )
{
	return LoadBSAnim( pFileData, fileSize, pDebugTag );
}

void BSAnimHeaderLoader::MakePlatformName( const char* pNeutralName, char* pPlatformName )
{
	// appends file system root and content directory
	Util::GetFiosFilePath_Platform( pNeutralName, pPlatformName );

#ifdef PLATFORM_PS3
	// we change our file extension to the correct one
	strcpy( strstr( pPlatformName, ".bsanim" ), ".bsanim_ps3" );
#endif
}
//--------------------------------------------------
//!
//!				Loading Helpers
//!
//-------------------------------------------------

#ifdef BS_USE_ALIGNER
	static const int BS_ALIGNMENT = 16;
#else
	static const int BS_ALIGNMENT = 16;
#endif


inline void CheckAlignment( const void* ptr )
{
	UNUSED(ptr);
#ifdef PLATFORM_PS3
	if ( ptr )
		ntError_p( Util::IsAligned( ptr, BS_ALIGNMENT ), ("Pointer (%#x) is not aligned to %#x", ptr, BS_ALIGNMENT ) );
#endif
}

inline void CheckPtr( const void* ptr )
{
	UNUSED(ptr);
	ntError_p( ptr, ("Pointer is NULL") );
#ifdef PLATFORM_PS3
	CheckAlignment( ptr );
#endif
}




BSAnimExport* LoadBSAnim( void* pFileData, uint32_t fileSize, const char* pDebugTag )
{
	UNUSED(pDebugTag);
	user_error_p( pFileData, ("Couldn't open bsanim '%s'", pDebugTag ) );

	char* pReadResult = NT_NEW_ARRAY_CHUNK( Mem::MC_PROCEDURAL ) char[ fileSize ];
	NT_MEMCPY( pReadResult, pFileData, fileSize );

	BSAnimExport* pAnimExp = reinterpret_cast<BSAnimExport*>( pReadResult );

	ResolveOffset( pAnimExp->m_pChannels,				pAnimExp );
//	ResolveOffset( pAnimExp->m_pIndexToTargetIDMap,		pAnimExp );

	ntError_p( pAnimExp->m_numOfChannels <= blendshapes::MAX_WEIGHTS, ("%s contains waaaay too many channels. Looks like a bad bsanim file to me...\n", pDebugTag) );

	for ( uint32_t iChannel = 0; iChannel < pAnimExp->m_numOfChannels; ++iChannel )
	{
		BSAnimChannelExport* pChannel = pAnimExp->m_pChannels + iChannel;
		ResolveOffset( pChannel->m_pKeyframeTimes,			pAnimExp );
		ResolveOffset( pChannel->m_pKeyframes,				pAnimExp );
	}
	
	return pAnimExp;
}



BSClumpHeader* LoadBSClump( void* pFileData, uint32_t fileSize, const char* pDebugTag )
{
	UNUSED(pDebugTag);
	user_error_p( pFileData, ("Couldn't open bsclump '%s'", pDebugTag ) );

	char* pReadResult = NT_NEW_ARRAY_CHUNK( Mem::MC_PROCEDURAL ) char[ fileSize ];
	NT_MEMCPY( pReadResult, pFileData, fileSize );

	BSClumpHeader* pBSClump = reinterpret_cast<BSClumpHeader*>( pReadResult );
	
	ResolveOffset( pBSClump->m_pBSMeshHeaders,					pBSClump );

	for ( u_int iMesh = 0; iMesh < pBSClump->m_numOfBSMeshHeaders; ++iMesh )
	{
		BSMeshHeader* pBSMesh = pBSClump->m_pBSMeshHeaders + iMesh;
		
		ResolveOffset( pBSMesh->m_pBSTargets,					pBSClump );
		CheckPtr( pBSMesh->m_pBSTargets );

		// check that this clump is not over budget
		ntError_p( pBSMesh->m_numOfBSTargets <= blendshapes::MAX_TARGETS, ("%s has too many targets in mesh %i. Please check exported asset\n", pDebugTag, iMesh) );

		for( u_int iTarget = 0 ; iTarget < pBSMesh->m_numOfBSTargets ; ++iTarget )
		{
			BSTarget* pBSTarget = pBSMesh->m_pBSTargets + iTarget;
			if ( pBSTarget->m_numOfDeltas )
			{
				// deltas should be within budget also
				ntError_p( pBSTarget->m_numOfDeltas <= blendshapes::MAX_DELTAS, ("%s: bstarget %i in bsmesh %i has too many deltas(%i). Please check exported asset\n", pDebugTag,iTarget,iMesh, pBSTarget->m_numOfDeltas) );

				ResolveOffset( pBSTarget->m_pIndices,				pBSClump );
				CheckAlignment( pBSTarget->m_pIndices );
				ResolveOffset( pBSTarget->m_pDeltas,				pBSClump );		
				CheckAlignment( pBSTarget->m_pDeltas );	
			}
			else
			{	
				ntAssert( !(pBSTarget->m_pIndices || pBSTarget->m_pDeltas) );
			}
		}
	}
	
	return pBSClump;
}



//eof
