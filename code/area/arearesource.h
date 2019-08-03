//--------------------------------------------------
//!
//!	\file arearesourcedb.h
//!	Database containing all resources managed by the
//! Area system
//!
//--------------------------------------------------

#ifndef _AREA_RESOURCE_H
#define _AREA_RESOURCE_H

//--------------------------------------------------
//!
//!	AreaResource
//! Single entry within the resource database
//!
//--------------------------------------------------
class AreaResource
{
public:
	// type of resource
	enum AR_TYPE
	{
		CLUMP = 0,
		TEXTURE,
		ANIM,
		BSANIM,
		ANIM_CONTAINER,
		BSANIM_CONTAINER,
		SPEEDTREE,
		SPEEDGRASS,
		WATER,

//		SOUND,
		AR_COUNT,
	};

	// status of resource
	enum AR_STATUS
	{
		UNLOADED,
		LOAD_REQUESTED,
		LOADING,
		LOADED,
	};

	// loading commands
	enum AR_REQUEST
	{
		// these commands can be aggregated 
		//---------------------------------
		// non blocking loads
		LOAD_ASYNC,
		UNLOAD_ASYNC,

		// these commands act imediately
		//---------------------------------
		// blocking loads
		LOAD_SYNC,
		UNLOAD_SYNC,

		// special install instructions
		CLUMP_LOAD_HEADER,
		CLUMP_UNLOAD_HEADER,
	};

	AreaResource( const char* pFullPath, uint32_t pathHash, AR_TYPE type, uint32_t areas );
	~AreaResource();

	const char*	GetFullPath() const				{ return m_pFullResourcePath; }
	uint32_t	GetPathHash() const				{ return m_pathHash; }
	AR_TYPE		GetType() const					{ return m_type; }
	AR_STATUS	GetStatus() const				{ return m_status; }
	uint32_t	GetAreas() const				{ return m_areas; }
	void		SetAreas( uint32_t i )			{ m_areas = i; }
	int32_t		GetRefCount() const				{ return m_refcount; }

	// flag changes of status
	inline void	MarkUnloaded()
	{
		ntAssert_p( m_refcount == 0, ("Resource (%s) has been requested", GetFullPath() ) );
		m_status = UNLOADED;
	}

	inline void	MarkLoadRequested()
	{
		ntAssert_p( GetStatus() == UNLOADED, ("Resource (%s) must be completely unloaded here", GetFullPath() ) );
		ntAssert_p( m_refcount > 0, ("Resource (%s) has not been requested", GetFullPath() ) );
		m_status = LOAD_REQUESTED;
	}

	inline void	MarkLoadRequestCanceled()
	{
		ntAssert_p( GetStatus() == LOAD_REQUESTED, ("Resource (%s) must be requested here", GetFullPath() ) );
		ntAssert_p( m_refcount == 0, ("Resource (%s) is still requested", GetFullPath() ) );
		m_status = UNLOADED;
	}

	inline void	MarkLoadProceeding()
	{
		ntAssert_p( GetStatus() == LOAD_REQUESTED, ("Resource (%s) must be completely unloaded here", GetFullPath() ) );
		ntAssert_p( m_refcount > 0, ("Resource (%s) has not been requested", GetFullPath() ) );
		m_status = LOADING;
	}

	void		Request( AR_REQUEST loadRequest );
	bool		RequiresSubmit() const			{ return m_requiresAsync; }
	void		SubmitAsync();

	int32_t		m_requestArea;
	int32_t		m_resourceSize;

	void			Load( void* pData, uint32_t fileSize );
	void			Unload();

private:
	char*			m_pFullResourcePath;	// the file path used to locate this resource
	uint32_t		m_pathHash;				// the hash of the above path, used to access resource caches
	AR_TYPE			m_type;					// what type of resource we are
	AR_STATUS		m_status;				// whats the status of this resource
	uint32_t		m_areas;				// what areas require this resource
	int				m_refcount;				// how many things have requested we're loaded
	bool			m_requiresAsync;		// flag that the status of this resource needs updating
};

#endif // _AREA_RESOURCE_H
