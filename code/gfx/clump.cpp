/***************************************************************************************************
*
*	$Header:: /game/clump.cpp 19    14/08/03 10:48 Dean                                            $
*
*	Clump Manipulation
*
*	CHANGES
*
*	25/2/2003	Dean	Created
*
***************************************************************************************************/

#include "gfx/clump.h"
#include "gfx/renderer.h"
#include "core/semantics.h"
#include "core/file.h"
#include "gfx/texturemanager.h"
#include "gfx/fxmaterial.h"
#include "gfx/graphicsdevice.h"

#include "anim/hierarchy.h"		// TODO: Don't really like this - should separate out JointLinkage from hierarchy.h at some point. [ARV].

//--------------------------------------------------
//!
//!	CClumpLoader
//!	Exposed clump loader API ctor
//!
//--------------------------------------------------
CClumpLoader::CClumpLoader()
{
	Util::SetToPlatformResources();

#ifdef PLATFORM_PS3
	char fullFileName[MAX_PATH];
	Util::GetFiosFilePath( "entities/characters/nobody/nobody.clump_ps3", fullFileName );
#else
	char fullFileName[MAX_PATH];
	Util::GetFiosFilePath( "entities/error_cube/error_cube.clump", fullFileName );
#endif

	m_pErrorClump = ClumpLoad_Complete( fullFileName, true );

	Util::SetToNeutralResources();

	ntError_p( m_pErrorClump, ("Could not load our error clump file") );
}

//--------------------------------------------------
//!
//!	CClumpLoader
//!	Exposed clump loader API dtor
//!
//--------------------------------------------------
CClumpLoader::~CClumpLoader()
{	
	ClumpUnload_Complete( m_pErrorClump );
}

//--------------------------------------------------
//!
//!	CClumpLoader::MakePlatformClumpName
//!	Transform initial name into platform specific
//! file name.
//!
//--------------------------------------------------
void	CClumpLoader::MakePlatformClumpName( const char* pNeutralName, char* pPlatformName )
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
	strcpy( strstr( pPlatformName, ".clump" ), ".clump_ps3" );
#endif
}

//--------------------------------------------------
//!
//!	CClumpLoader::LoadClump_Neutral
//!	Takes a platform neutral path as an argument, such as:
//! "entities/Resources/Scenery/Fort/planesterrain.clump" or
//! "entities\\Resources\\Scenery\\Fort\\planesterrain.clump"
//!
//--------------------------------------------------
CClumpHeader* CClumpLoader::LoadClump_Neutral( const char* pNeutralName, bool bImmediate, bool bAllowMissingTex )
{
	char pPlatformName[MAX_PATH];
	MakePlatformClumpName( pNeutralName, pPlatformName );
	return LoadClump_Platform( pPlatformName, bImmediate, bAllowMissingTex );
}

//--------------------------------------------------
//!
//!	CClumpLoader::LoadClump_Platform
//!	Takes a platform specific path as an argument, such as:
//! "entities/resources/scenery/fort/planesterrain.clump" for PC
//! "content_ps3/entities/resources/scenery/fort/planesterrain.clump_ps3" for PS3
//! (Note: no back slashes or uppercase letters)
//!
//--------------------------------------------------
CClumpHeader* CClumpLoader::LoadClump_Platform( const char* pPlatformName, bool bImmediate, bool bAllowMissingTex )
{
	// Check clump cache first.
	CHashedString nameHash(pPlatformName);
	RefCountedClumpAdaptor& clumpAdaptor = m_clumpMap[ nameHash.GetValue() ]; 

	if( clumpAdaptor.m_iRefCount != 0 )
	{
		clumpAdaptor.AddRef();
		return clumpAdaptor.m_pobClump;
	}

	// nope, were going to load of disk instead
	Util::SetToPlatformResources();

	// if we dont exist, use a big red default clump
	if( !File::Exists( pPlatformName ) )
	{
		ntPrintf( Debug::DCU_CLUMP, "WARNING!! Missing clump: %s. Using shiny red cube instead.\n", pPlatformName );
		clumpAdaptor.m_pobClump = m_pErrorClump;
	}
	else
	{
		if (bImmediate)
			clumpAdaptor.m_pobClump = ClumpLoad_Complete( pPlatformName, bAllowMissingTex );
		else
			clumpAdaptor.m_pobClump = ClumpLoad_HeaderOnly( pPlatformName );

#ifdef PLATFORM_PS3
		// if we can't load this clump (old version?) we try to load a default error clump instead
		if ( clumpAdaptor.m_pobClump == reinterpret_cast<CClumpHeader*>(BADCLUMP) )
		{
			ntPrintf( Debug::DCU_CLUMP, "WARNING!! Missing clump: %s. Using shiny red cube instead.\n", pPlatformName );
			clumpAdaptor.m_pobClump = m_pErrorClump;
		}
#endif
	}

	clumpAdaptor.AddRef();
	m_reverseClumpMap[ clumpAdaptor.m_pobClump ] = nameHash.GetValue();

	Util::SetToNeutralResources();
	return clumpAdaptor.m_pobClump;
}

//--------------------------------------------------
//!
//!	CClumpLoader::UnloadClump
//!	dec refcount, free resources when not req
//!
//--------------------------------------------------
void CClumpLoader::UnloadClump( CClumpHeader* pobClumpHeader )
{
	if (pobClumpHeader == m_pErrorClump)
		return;

	ReverseClumpMap::iterator rmapIt = m_reverseClumpMap.find( pobClumpHeader );
	ntError_p( rmapIt != m_reverseClumpMap.end(), ("Trying to unload a clump that isn't loaded\n") );

	ClumpMap::iterator mapIt = m_clumpMap.find( rmapIt->second ); 
	ntError_p( mapIt != m_clumpMap.end(), ("Eek clump maps out of sync\n") );

	// if the release will cause an unload clear the maps
	if( mapIt->second.m_iRefCount == 1 )
	{
		mapIt->second.Release();

		m_reverseClumpMap.erase( rmapIt );
		m_clumpMap.erase( mapIt );
	}
	else
	{
		mapIt->second.Release();
	}
}

//--------------------------------------------------
//!
//!	CClumpLoader::FixupClumpMeshData
//!	Find this clump from the cache, and fix up its
//! mesh data resources using the input pFileData ptr.
//!
//!	void* pFileData MUST point to the original clump
//! file in memory, usually a result of an async load.
//!
//! returns true if allocations actually took place
//!
//--------------------------------------------------
bool	CClumpLoader::FixupClumpMeshData( uint32_t cacheKey, void* pFileData )
{
	CClumpHeader* pClumpHeader = GetClumpFromCache_Key( cacheKey );
	ntError_p( pClumpHeader, ("Clump header MUST already be loaded by now") );

	if (pClumpHeader == m_pErrorClump)
		return false;

	if (pClumpHeader->m_pAdditionalData->m_bHasVRAMResources == false)
	{
		ntError_p( pFileData, ("MUST supply a valid file data pointer here") );
		ClumpFixup_MeshData( *pClumpHeader, pFileData );
		return true;
	}

	return false;
}

//--------------------------------------------------
//!
//!	CClumpLoader::FreeupClumpMeshData
//!	Find this clump from the cache, then unload its
//! associated mesh data
//!
//! returns true if allocations actually took place
//!
//--------------------------------------------------
bool	CClumpLoader::FreeupClumpMeshData( uint32_t cacheKey )
{
	CClumpHeader* pClumpHeader = GetClumpFromCache_Key( cacheKey );
	ntError_p( pClumpHeader, ("Clump header MUST already be loaded by now") );

	if (pClumpHeader == m_pErrorClump)
		return true;

	if (pClumpHeader->m_pAdditionalData->m_bAllocatedAsComplete == false)
	{
		ClumpFreeup_MeshData( *pClumpHeader );
		return true;
	}

	return false;
}

//--------------------------------------------------
//!
//!	CClumpLoader::GetClumpFromCache_Neutral
//!	check cache for clump using raw XML / lua path
//!
//--------------------------------------------------
CClumpHeader* CClumpLoader::GetClumpFromCache_Neutral( const char* pNeutralName )
{
	char pPlatformName[MAX_PATH];
	MakePlatformClumpName( pNeutralName, pPlatformName );
	return GetClumpFromCache_Key( CHashedString(pPlatformName).GetValue() );
}

//--------------------------------------------------
//!
//!	CClumpLoader::GetClumpFromCache_Key
//!	check cache for clump using the cache key (a hash of pPlatformName)
//!
//--------------------------------------------------
CClumpHeader* CClumpLoader::GetClumpFromCache_Key( uint32_t cacheKey )
{
	ClumpMap::iterator it = m_clumpMap.find( cacheKey ); 
	if ( it != m_clumpMap.end() )
		return it->second.m_pobClump;
	return 0;
}

//--------------------------------------------------
//!
//! these allow use to seperate ingame resources from the on disk mesh header.
//! meaning i can change their sizes for memory track without changing the 
//! export structure.
//!
//--------------------------------------------------
VBHandle CClumpLoader::RetrieveVBHandle( const CMeshHeader* pHeader )
{
	VBMap::iterator it = m_VBCache.find(pHeader);
	if (it != m_VBCache.end())
		return it->second.GetVBHandle(0);
	
	return VBHandle();
}

const CVertexStreamsContainter* CClumpLoader::RetrieveVBHandles( const CMeshHeader* pHeader )
{
	VBMap::iterator it = m_VBCache.find(pHeader);
	if (it != m_VBCache.end())
		return &(it->second);
	
	return NULL;
}

IBHandle CClumpLoader::RetrieveIBHandle( const CMeshHeader* pHeader )
{
	IBMap::iterator it = m_IBCache.find(pHeader);
	if (it != m_IBCache.end())
		return it->second;
	
	return IBHandle();
}

void	CClumpLoader::AddVBHandle( const CMeshHeader* pHeader, VBHandle hHandle )
{
	ntAssert_p( CClumpLoader::Get().RetrieveVBHandle(pHeader).Get() == 0, ("Already have a VB for this mesh header"));
	CVertexStreamsContainter dummyContainer;
	dummyContainer.SetVBHandle( hHandle, 0 );
	CClumpLoader::Get().m_VBCache[pHeader] = dummyContainer;
}

void	CClumpLoader::AddVBHandles( const CMeshHeader* pHeader, const CVertexStreamsContainter& oVBHandles )
{
	ntAssert_p( CClumpLoader::Get().RetrieveVBHandles(pHeader) == 0, ("Already have a VB for this mesh header"));
	CClumpLoader::Get().m_VBCache[pHeader] = oVBHandles;
}
	
void	CClumpLoader::AddIBHandle( const CMeshHeader* pHeader, IBHandle hHandle )
{
	ntAssert_p( CClumpLoader::Get().RetrieveIBHandle(pHeader).Get() == 0, ("Already have a IB for this mesh header"));
	CClumpLoader::Get().m_IBCache[pHeader] = hHandle;
}

void	CClumpLoader::RemoveVBHandle( const CMeshHeader* pHeader )
{
	CClumpLoader::VBMap::iterator it = CClumpLoader::Get().m_VBCache.find(pHeader);
	ntAssert_p( it != CClumpLoader::Get().m_VBCache.end(), ("Cannot find VB for this mesh header") );
	CClumpLoader::Get().m_VBCache.erase(it);
}
	
void	CClumpLoader::RemoveIBHandle( const CMeshHeader* pHeader )
{
	CClumpLoader::IBMap::iterator it = CClumpLoader::Get().m_IBCache.find(pHeader);
	ntAssert_p( it != CClumpLoader::Get().m_IBCache.end(), ("Cannot find IB for this mesh header") );
	CClumpLoader::Get().m_IBCache.erase(it);
}

void CClumpLoader::RemapLinkageArray( CClumpHeader *clump_header )
{
	//
	//	We remap the data in-place - the starts of the two resulting arrays are recovered and stored in
	//	CHierarchy (which should be the only thing using the linkage anyway). I've made the old
	//	m_pobTransformLinkage member private so nothing can accidentally access the old pointer. [ARV].
	//

	// Work out where the pointers should start.
	GpJointLinkage *joint_linkage = (GpJointLinkage *)( (CTransformLinkage *)clump_header->m_pobTransformLinkageArray );
	GpJointName *joint_name = (GpJointName *)( (uintptr_t)joint_linkage + sizeof( GpJointLinkage )*clump_header->m_iNumberOfTransforms );
	ntError( (uintptr_t)( joint_name + clump_header->m_iNumberOfTransforms ) == (uintptr_t)( (CTransformLinkage *)clump_header->m_pobTransformLinkageArray + clump_header->m_iNumberOfTransforms ) );

	const CTransformLinkage *old_linkage = clump_header->m_pobTransformLinkageArray;

	// Store a copy of the hash-names so we don't overwrite data we haven't used yet.
	static const uint32_t MaxNumTransforms = 512;
	GpJointName temp_joint_names[ MaxNumTransforms ];
	ntError_p( (uint32_t)clump_header->m_iNumberOfTransforms <= MaxNumTransforms, ("You cannot have more than 512 transforms in any single hierarchy.") );

	for ( int32_t i=0;i<clump_header->m_iNumberOfTransforms;i++ )
	{
		temp_joint_names[ i ].m_name	= old_linkage[ i ].m_obNameHash;
		temp_joint_names[ i ].m_index	= i;
	}

	// Now go through and remap the linkage data.
	for ( int32_t i=0;i<clump_header->m_iNumberOfTransforms;i++ )
	{
		ntError( (uintptr_t)( joint_linkage + i ) <= (uintptr_t)( old_linkage + i ) );
		CTransformLinkage linkage = old_linkage[ i ];

		joint_linkage[ i ].m_parentIndex = linkage.m_sParentIndex;

		joint_linkage[ i ].m_firstChildIndex = linkage.m_sFirstChildIndex;
		joint_linkage[ i ].m_nextSiblingIndex = linkage.m_sNextSiblingIndex;

		// Check the range.
		ntError( linkage.m_iFlags >= INT16_MIN && linkage.m_iFlags <= INT16_MAX );
		joint_linkage[ i ].m_flags = (int16_t)linkage.m_iFlags;
	}

	// Sort the joint name array by hash-value.
	for ( int32_t i=0;i<clump_header->m_iNumberOfTransforms;i++ )
	{
		for ( int32_t j=i+1;j<clump_header->m_iNumberOfTransforms;j++ )
		{
			if ( temp_joint_names[ j ].m_name < temp_joint_names[ i ].m_name )
			{
				GpJointName temp = temp_joint_names[ i ];
				temp_joint_names[ i ] = temp_joint_names[ j ];
				temp_joint_names[ j ] = temp;
			}
		}
	}

	// Now patch up the joint hash-names.
	for ( int32_t i=0;i<clump_header->m_iNumberOfTransforms;i++ )
	{
		ntError( (uintptr_t)( joint_name + i ) < (uintptr_t)( old_linkage + clump_header->m_iNumberOfTransforms ) );
		joint_name[ i ] = temp_joint_names[ i ];
	}
}








