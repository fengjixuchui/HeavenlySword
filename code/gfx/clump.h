/***************************************************************************************************
*
*	$Header:: /game/clump.h 2     26/03/03 15:27 Dean                                              $
*
*	Clump Manipulation
*
*	CHANGES
*
*	25/2/2003	Dean	Created
*
***************************************************************************************************/

#ifndef	GFX_CLUMP_H
#define	GFX_CLUMP_H

#if defined( PLATFORM_PC )
#include "gfx/clump_pc.h"
#elif defined( PLATFORM_PS3 )
#include "gfx/clump_ps3.h"
#endif

#ifndef GFX_VERTEXDECLARATION_H
#include "gfx/vertexdeclaration.h"
#endif

#ifndef _EXPORTSTRUCT_CLUMP_H_
#include "core/exportstruct_clump.h"
#endif

class	CMeshHeader;
class	CClumpLoaderImpl;

/***************************************************************************************************
*
*	CLASS			CVertexStreamsContainter
*
*	DESCRIPTION		This is very simple class that contains all the vertex streams that belong to a mesh
*
***************************************************************************************************/

class CVertexStreamsContainter
{
public:

	CVertexStreamsContainter(void)
	{
		s_aobVBHandle[0] = s_aobVBHandle[1] = VBHandle();
	}

	VBHandle GetVBHandle(uint32_t uiVBIndex) const
	{
		if (uiVBIndex < 2)
			return s_aobVBHandle[uiVBIndex];
		else
			return VBHandle();
	}

	void SetVBHandle(VBHandle hVBhandle, uint32_t uiVBIndex)
	{
		if (uiVBIndex < 2)
			s_aobVBHandle[uiVBIndex] = hVBhandle;
	}

private:

	VBHandle s_aobVBHandle[2];
};


/***************************************************************************************************
*
*	CLASS			CClumpLoader
*
*	DESCRIPTION		This is a load/unload class used to get clumps into memory.
*
*	NOTES			Uses the pPlatformName string as the cache key, perfectly valid for other systems
*					to attempt clump retrieval using their own hashes of this string.
*
*					pPlatformName MUST be the result of MakePlatformClumpName(), and be a valid
*					plaform specific file name of the clump in question.
*
***************************************************************************************************/

#define BADCLUMP (-1)

class	CClumpLoader : public Singleton<CClumpLoader>
{
public:
	CClumpLoader();
	~CClumpLoader();

	// load / unload the header information
	// these methods are refcounted, and so will allocate or destroy
	// the resource appropriately
	CClumpHeader*	LoadClump_Neutral( const char* pNeutralName, bool bImmediate, bool bAllowMissingTex = true );
	CClumpHeader*	LoadClump_Platform( const char* pPlatformName, bool bImmediate, bool bAllowMissingTex = true );
	void			UnloadClump( CClumpHeader* pobClumpHeader );

	// convert a neutral name into the real platform specific resource path
	// note, the result is used as the hash basis for the internal cache.
	static void		MakePlatformClumpName( const char* pNeutralName, char* pPlatformName );

	// patchup or release the clump mesh data associated with this clump
	bool			FixupClumpMeshData( uint32_t cacheKey, void* pFileData );
	bool			FreeupClumpMeshData( uint32_t cacheKey );
	
	// note this method is NOT refcounted.
	CClumpHeader*	GetClumpFromCache_Neutral( const char* pNeutralName );
	CClumpHeader*	GetClumpFromCache_Key( uint32_t cacheKey );

	// GFX data retrieve methods
	IBHandle	RetrieveIBHandle( const CMeshHeader* pHeader );

	// PC-centric retrieve method
	VBHandle	RetrieveVBHandle( const CMeshHeader* pHeader );

	// PS3-centric retrieve method 
	const CVertexStreamsContainter* RetrieveVBHandles( const CMeshHeader* pHeader );

	// PC-centric VB addition method
	void	AddVBHandle( const CMeshHeader* pHeader, VBHandle hHandle );

	// PS3-centric VB addition method
	void	AddVBHandles( const CMeshHeader* pHeader, const CVertexStreamsContainter& oVBHandles );
	void	AddIBHandle( const CMeshHeader* pHeader, IBHandle hHandle );

private:
	CClumpHeader*	LoadClumpInternal( const char* pcClumpName, bool bImmediate );

	// clump header manipulation functions
	static	CClumpHeader*	ClumpLoad_Complete( const char* pcClumpName, bool bAllowMissingTex );
	static	void			ClumpUnload_Complete( CClumpHeader* pobClumpHeader );

	static	CClumpHeader*	ClumpLoad_HeaderOnly( const char* pcClumpName );
	static	void			ClumpUnload_HeaderOnly( CClumpHeader* pobClumpHeader );

	static	void	ClumpFixup_MeshData( CClumpHeader& header, void* pFileData );
	static	void	ClumpFreeup_MeshData( CClumpHeader& header );

	static	void	ClumpFixup_Textures( CClumpHeader& header, bool bAllowTextureLoad );
	static	void	ClumpFreeup_Textures( CClumpHeader& header );

#ifdef PLATFORM_PS3
	friend CClumpHeader* ClumpLoad_HeaderOnly_New(File& clumpFile, const CClumpHeader& tempCH);

	static void  ClumpFixup_DiscardableSection( CClumpHeader& header, void* pFileData );
	static void  ClumpFixup_GlobalSection( CClumpHeader* pobClumpHeader );
	static void	 ClumpFreeup_DiscardableSection( CClumpHeader* clumpHeader );
#endif

	// GFX data manipulation
	void	RemoveVBHandle( const CMeshHeader* pHeader );
	void	RemoveIBHandle( const CMeshHeader* pHeader );

	// Remap linkage to a more atg friendly version.
	static void	RemapLinkageArray	( CClumpHeader *clump_header );

	typedef ntstd::Map<const CMeshHeader*, CVertexStreamsContainter> VBMap;
	typedef ntstd::Map<const CMeshHeader*, IBHandle> IBMap;

	VBMap m_VBCache;
	IBMap m_IBCache;

	CClumpHeader*	m_pErrorClump;

	struct RefCountedClumpAdaptor
	{
		RefCountedClumpAdaptor() : 
			m_pobClump( 0 ),
			m_iRefCount( 0 )
		{}

		explicit RefCountedClumpAdaptor(CClumpHeader* clump) : 
			m_pobClump( clump ),
			m_iRefCount(1)
		{
		}

		void AddRef() { m_iRefCount++; };
		void Release() 
		{
			--m_iRefCount;
			if( m_iRefCount == 0 )
			{
				if (m_pobClump->m_pAdditionalData->m_bAllocatedAsComplete)
					CClumpLoader::Get().ClumpUnload_Complete( m_pobClump );
				else
					CClumpLoader::Get().ClumpUnload_HeaderOnly( m_pobClump );
			}
		}

		CClumpHeader* m_pobClump;
		unsigned int m_iRefCount;
	};

	typedef ntstd::Map< uint32_t, RefCountedClumpAdaptor > ClumpMap;
	typedef ntstd::Map< CClumpHeader*, uint32_t > ReverseClumpMap;

	ClumpMap		m_clumpMap;
	ReverseClumpMap m_reverseClumpMap;
};


#endif	//_CLUMP_H
