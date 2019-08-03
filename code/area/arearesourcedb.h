//--------------------------------------------------
//!
//!	\file arearesourcedb.h
//!	Database containing all resources managed by the
//! Area system
//!
//--------------------------------------------------

#ifndef AREA_RESOURCE_DB_H
#define AREA_RESOURCE_DB_H

#ifndef AREA_SYSTEM_H
#include "area/areasystem.h"
#endif

#ifndef _AREA_RESOURCE_H
#include "area/arearesource.h"
#endif

#ifdef PLATFORM_PS3
#ifndef LOADER_PS3_H_
#include "core/loader_ps3.h"
#endif
#endif

#ifndef _RELEASE
#ifdef TRACK_GFX_MEM
#define TRACK_AREA_RAM
#endif
#endif

#ifdef TRACK_AREA_RAM
#define LOG_RESOURCE_LOADING
#endif

//--------------------------------------------------
//!
//!	AreaResourceDB
//! Class used by the area system to maintain
//! the resources used within a level
//!
//--------------------------------------------------
class AreaResourceDB : public Singleton<AreaResourceDB>
{
public:
	typedef ntstd::Map< uint32_t, AreaResource* >	ResMap;
	typedef ntstd::List< AreaResource* >			ResList;

	AreaResourceDB();
	~AreaResourceDB();

	// prime the clump cache, load global resources
	void PreLevelLoadInit( const char* pLevelName );
	void PostLevelLoadInit();

	// unload everything we have requested
	void UnloadDatabase();

	// Update, handles async load / unload requests
	void Update();

	// Add an entry to the database, called by lua functions
	void AutoAddAreaResource( const char* pName, AreaResource::AR_TYPE type );

	// Add an entry to the database
	uint32_t AddAreaResource( const char* pName, AreaResource::AR_TYPE type, uint32_t areas );

	// simple retrieval
	inline AreaResource* GetEntry( uint32_t resID )
	{
		ResMap::iterator it = m_resourceDB.find( resID );
		if (it != m_resourceDB.end())
			return it->second;
		return 0;
	}

	// request load / unload
	inline void Area_LoadSync( int32_t iAreaNumber )
	{
		m_debugArea = iAreaNumber;
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		SendAreaRequest( (1 << GetAreaIndex(iAreaNumber)), AreaResource::LOAD_SYNC );
	}

	inline void Area_UnloadSync( int32_t iAreaNumber )
	{
		m_debugArea = iAreaNumber;
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		SendAreaRequest( (1 << GetAreaIndex(iAreaNumber)), AreaResource::UNLOAD_SYNC );
	}

	void Area_LoadAsync( int32_t iAreaNumber );
	inline void Area_UnloadAsync( int32_t iAreaNumber )
	{
		m_debugArea = iAreaNumber;
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		SendAreaRequest( (1 << GetAreaIndex(iAreaNumber)), AreaResource::UNLOAD_ASYNC );
		m_bAsyncSubmitRequired = true;
	}

	// check to see if a load has finished
	inline bool Area_LoadFinished( int32_t iAreaNumber )
	{
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );

		// If this area exists in the deferred list then always return false.
		if ( ntstd::find( m_DeferredAreas.begin(), m_DeferredAreas.end(), iAreaNumber ) != m_DeferredAreas.end() )
			return false;

		return LoadFinished( (1 << GetAreaIndex(iAreaNumber)) );
	}

	// methods for adding resource load requests
	void RequestAsyncLoad( uint32_t resID );
	void CancelAsyncLoad( uint32_t resID );

#ifndef _RELEASE
	bool DebugIsLoadRequested( uint32_t resID );
	bool DebugIsLoadProceeding( uint32_t resID );

	// check to see the estimated time of arrival
	float Area_LoadFraction( int32_t iAreaNumber );

	// Query database for total size of loaded resources in these areas
	int32_t GetSizeFor( uint32_t mustMatch, uint32_t areasToTest, AreaResource::AR_TYPE type );
#endif

	int32_t	DebugGetLastAreaLoaded() const { return m_debugArea; }

	float GetAreaSysTexRamMB()		const { return _R(m_totalTexVRAM)/(1024.f*1024.f); }
	float GetAreaSysClumpRamMB()	const { return _R(m_totalClumpVRAM)/(1024.f*1024.f); }
	float GetAreaSysAnimRamMB()		const { return _R(m_totalAnimRAM)/(1024.f*1024.f); }
	
	inline void DumpAreaSysRam()
	{
#ifdef LOG_RESOURCE_LOADING
		ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
		ntPrintf( Debug::DCU_RESOURCES, "- Area texture VRAM now %.2f Mb.\n", GetAreaSysTexRamMB() );
		ntPrintf( Debug::DCU_RESOURCES, "- Area clump VRAM now %.2f Mb.\n", GetAreaSysClumpRamMB() );
		ntPrintf( Debug::DCU_RESOURCES, "- Area anim RAM now %.2f Mb.\n", GetAreaSysAnimRamMB() );
		ntPrintf( Debug::DCU_RESOURCES, "-----------------------------------------------\n" );
#endif
	}

	// public ram counters, individual resources adjust these
	int32_t	m_totalTexVRAM;
	int32_t	m_totalClumpVRAM;
	int32_t	m_totalAnimRAM;

	static void	AsycLoadCallback( const char* pFullPath, void* pUserData, void* pFileData, uint32_t file_size );

	// dump the area resource database to a log file
	void DumpDatabase( uint32_t areaMask, bool bSortBySize );

private:

	// get unique name
	void TransformName( const char* pSrc, char* pDest )
	{
		while( *pSrc != 0 )
		{
			if( *pSrc >= 'A' && *pSrc <= 'Z' )
				*pDest = (char)tolower( *pSrc );
			else if( *pSrc == '\\' )
				*pDest = '/';
			else
				*pDest = *pSrc;
			pDest++;
			pSrc++;
		}
		*pDest = 0;
	}

	// clear entries out of the database
	void ResetDatabase();

	// to stop me cocking up...
	inline int32_t	GetAreaIndex( int32_t areaID ) { return areaID-1; }

	// Collapse and submit async requests
	void SubmitResourceRequests();

	// Send resource requests to all within this area
	void SendAreaRequest( uint32_t areaMask, AreaResource::AR_REQUEST req );

	// Cancel an already submitted load request 
	void CancelLoadRequest( uint32_t areaMask );

	// make sure all the load requests for this area are finished
	bool LoadFinished( uint32_t areaMask );

	ResMap		m_resourceDB;
	ResList		m_clumps;
	ResList		m_textures;
	ResList		m_anims;
	ResList		m_misc;
	ResList		m_toBeConfirmed;

#ifdef PLATFORM_PS3
	Loader::RequestList m_loadRequests;
#else
	ResList		m_loadRequests;
#endif

	enum	INIT_PHASE
	{
		UNINITIALISED,
		INSTALLING_MANIFESTS,
		INSTALLING_LEVEL,
		INITIALISED,
	};

	INIT_PHASE	m_eInitPhase;
	bool		m_bAsyncSubmitRequired;

	int32_t		m_installArea;
	int32_t		m_debugArea;

#ifndef _RELEASE
	int32_t		m_loadCount[ AreaSystem::NUM_AREAS ];
#endif

	typedef ntstd::List< int32_t > DeferredAreaContainer;
	DeferredAreaContainer m_DeferredAreas;
};

//--------------------------------------------------
//!
//!	ClumpMemTracker::debug tracking class
//!
//--------------------------------------------------
class ClumpMemTracker
{
public:
#ifdef TRACK_AREA_RAM
	ClumpMemTracker();
	int GetDelta() const;
#else
	int GetDelta() const { return 0; }
#endif

	float GetDeltaKb() const { return _R( GetDelta() ) / 1024.0f; }
	float GetDeltaMb() const { return GetDeltaKb() / 1024.0f; }

private:
	uint32_t m_startValueVB;
	uint32_t m_startValueIB;
};

#ifdef TRACK_AREA_RAM
#define CLUMP_TRACKER()	ClumpMemTracker	mem;
#else
#define CLUMP_TRACKER()
#endif

#endif
