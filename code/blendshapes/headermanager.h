//--------------------------------------------------
//!
//!	\file headermanager.h
//!	Generic header manager for loading/unloading
//! export headers
//!
//--------------------------------------------------

#ifndef _HEADERMANAGER_H_
#define _HEADERMANAGER_H_

#include "core/user.h"

#if !defined(CORE_FILE_H)
#include "core/file.h"
#endif // end CORE_FILE_H

//--------------------------------------------------
//!
//!	default deleter functor
//!
//--------------------------------------------------
template< typename T >
struct HeaderDeleterProcedural
{
	void operator()( T* ptr ) { NT_DELETE_ARRAY_CHUNK( Mem::MC_PROCEDURAL, (char*)ptr); }
};	

template<class T> class BSHeaderPtr;

//--------------------------------------------------
//!
//!	Header manager template
//!	T - header type
//! L - loading functor
//! U - unloading functor
//!
//--------------------------------------------------
template< typename T, typename L, typename U>
class HeaderManager : public Singleton< HeaderManager<T, L, U> >
{
	typedef HeaderManager	MyType;

	friend class BSHeaderPtr<MyType>;


public:

	struct HeaderAdaptor
	{
		typedef T		HeaderType;

		T*				m_pobHeader;
		u_int			m_iRefCount;

		HeaderAdaptor()
			: m_pobHeader( 0 )
			, m_iRefCount( 0 )
		{}

		explicit HeaderAdaptor( T* pobHeader )
			: m_pobHeader(  pobHeader )
			, m_iRefCount( 1 )
		{}

	};


	//! loads a header of type T from the specified file and caches it for future retrieval
	HeaderAdaptor*		Load_Neutral( const char* pNeutralName )
	{
		char pPlatformName[MAX_PATH];
		MakePlatformName( pNeutralName, pPlatformName );
		return Load_Platform( pPlatformName );
	}


	HeaderAdaptor*		Load_Platform( const char* pPlatformName )
	{
		if (!pPlatformName)
			return 0;

		// Check cache first.
		CHashedString name(pPlatformName);
		HeaderAdaptor& adaptor = GetAdapter(name); //m_obNameToAdaptorMap[ name.GetValue() ];

		if( adaptor.m_iRefCount != 0 )
		{
			adaptor.m_iRefCount++;
			//return adaptor.m_pobHeader;
			return &adaptor;
		}

	#ifdef PLATFORM_PC
		Util::SetToPlatformResources();
	#endif

		// nope proceed with load
		uint8_t *pReadResult = NULL;
		uint32_t fileSize = 0;
		if ( File::Exists( pPlatformName ) )
		{
			File dataFile;
			LoadFile_Chunk( pPlatformName, File::FT_READ | File::FT_BINARY, Mem::MC_PROCEDURAL, dataFile, &pReadResult );
			fileSize = dataFile.GetFileSize();
		}

		HeaderAdaptor* pResult = Load_FromData( name.GetValue(), (void *)pReadResult, fileSize, pPlatformName );

		// free temp buffers and return result
		if (pReadResult)
		{
			NT_DELETE_ARRAY_CHUNK( Mem::MC_PROCEDURAL, pReadResult );
		}

	#ifdef PLATFORM_PC
		Util::SetToNeutralResources();
	#endif

		// note: Load_FromData with have put the header into the cache
		return pResult;
	}


	HeaderAdaptor*		Load_FromData( uint32_t cacheKey, void* pFileData, uint32_t fileSize, const char* pDebugTag )
	{
		// Check cache first.
		HeaderAdaptor& adaptor = GetAdapter(CHashedString(cacheKey)); // m_obNameToAdaptorMap[ cacheKey ];

		if( adaptor.m_iRefCount != 0 )
		{
			adaptor.m_iRefCount++;
			//return adaptor.m_pobHeader;
			return &adaptor;
		}

		adaptor.m_pobHeader = m_obLoader( pFileData, fileSize, pDebugTag );

				user_code_start(Ozz)
			ntPrintf( "^^^^ %s loaded ^^^^\n", pDebugTag );
				user_code_end()

		adaptor.m_iRefCount++;
		m_obHeaderToNameMap[ adaptor.m_pobHeader ] = cacheKey;
			//return adaptor.m_pobHeader;
		return &adaptor;
	}

	private:
	bool Unload( HeaderAdaptor* ptr )
	{
		if ( ptr && ptr -> m_pobHeader)
		{
			// header should be present on both maps. Otherwise, we're in trouble
			typename HeaderToNameMap_t::iterator hnIt = m_obHeaderToNameMap.find( ptr -> m_pobHeader );
			ntError_p( hnIt != m_obHeaderToNameMap.end(), ("trying to load a missing header") );

			typename NameToAdaptorMap_t::iterator naIt = m_obNameToAdaptorMap.find( hnIt->second );
			ntError_p( naIt != m_obNameToAdaptorMap.end(), ("maps out of sync") );

			ntError(ptr == naIt -> second);
			
			// decrease the reference counter in adaptor and remove from map if last one
			//if ( (--(naIt->second->m_iRefCount)) == 0 )
			// this function is to be called from the smart pointer only 
			if (naIt->second->m_iRefCount == 0)
			{
				user_code_start(Ozz)
					ntPrintf( "^^^^ %s unloaded ^^^^\n", ntStr::GetString(  CHashedString(hnIt->second)  ) );
				user_code_end()

				m_obUnloader( naIt->second->m_pobHeader );
				DeleteAdapter(naIt);
				//m_obNameToAdaptorMap.erase( naIt );
				m_obHeaderToNameMap.erase( hnIt );
				return true;
			}
		}
		return false;
	}


	public:

	//! unloads header if no more references to it are pending
	bool	Unload_Neutral( const char* pNeutralName );
	bool	Unload_Key( uint32_t cacheKey );
	//bool	Unload( T* ptr );

	// test to see if this anim is present already
	bool Loaded_Neutral		( const char *pNeutralName )	const;
	bool Loaded_Platform	( const char *pPlatformName )	const;
	bool Loaded_Cache		( uint32_t cacheKey )			const;

	//! make a platform specific name, used as cache keys in header manager
	static void	MakePlatformName( const char* pNeutralName, char* pPlatformName )
	{
		return L::MakePlatformName( pNeutralName, pPlatformName );
	}

//private:
public:
	
	//! reference count adaptor
	typedef ntstd::Map< uint32_t, HeaderAdaptor*, ntstd::less<uint32_t>, Mem::MC_PROCEDURAL >	NameToAdaptorMap_t;
	typedef ntstd::Map< T*, uint32_t, ntstd::less<T*>, Mem::MC_PROCEDURAL >						HeaderToNameMap_t;

	HeaderAdaptor&	GetAdapter(CHashedString name)
	{
		typename NameToAdaptorMap_t::iterator iter = m_obNameToAdaptorMap.find( ntStr::GetHashKey(name) );
		if ( m_obNameToAdaptorMap.end() != iter )
		{
			return *iter -> second;
		}
		else
		{
			HeaderAdaptor* newAdapter = NT_NEW_CHUNK(Mem::MC_PROCEDURAL) HeaderAdaptor;
			m_obNameToAdaptorMap.insert(typename NameToAdaptorMap_t::value_type( ntStr::GetHashKey(name), newAdapter ));
			return *newAdapter;
		}

	}

	void DeleteAdapter(typename NameToAdaptorMap_t::iterator iterToDo)
	{
		NT_DELETE_CHUNK(Mem::MC_PROCEDURAL, iterToDo -> second);
		m_obNameToAdaptorMap.erase(iterToDo);
	}

	//! maps to keep track of things
	NameToAdaptorMap_t	m_obNameToAdaptorMap;
	HeaderToNameMap_t	m_obHeaderToNameMap;

	//! load/unload objs
	L					m_obLoader;
	U					m_obUnloader;

	//friend class BSHeaderPtr<MyType::HeaderAdaptor>;

};

//template< typename T, typename L, typename U >
//HeaderManager<T,L,U>::HeaderAdapter* HeaderManager<T,L,U>::Load_Neutral( const char* pNeutralName )
//{
//	char pPlatformName[MAX_PATH];
//	MakePlatformName( pNeutralName, pPlatformName );
//	return Load_Platform( pPlatformName );
//}


//template< typename T, typename L, typename U >
//HeaderManager<T,L,U>::HeaderAdapter* HeaderManager<T,L,U>::Load_Platform( const char* pPlatformName )
//{
//	if (!pPlatformName)
//		return 0;
//
//	// Check cache first.
//	CHashedString name(pPlatformName);
//	HeaderAdaptor& adaptor = GetAdapter(name); //m_obNameToAdaptorMap[ name.GetValue() ];
//
//	if( adaptor.m_iRefCount != 0 )
//	{
//		adaptor.m_iRefCount++;
//		//return adaptor.m_pobHeader;
//		return &adaptor;
//	}
//
//#ifdef PLATFORM_PC
//	Util::SetToPlatformResources();
//#endif
//
//	// nope proceed with load
//	uint8_t *pReadResult = NULL;
//	uint32_t fileSize = 0;
//	if ( File::Exists( pPlatformName ) )
//	{
//		File dataFile;
//		LoadFile_Chunk( pPlatformName, File::FT_READ | File::FT_BINARY, Mem::MC_PROCEDURAL, dataFile, &pReadResult );
//		fileSize = dataFile.GetFileSize();
//	}
//
//	HeaderAdaptor* pResult = Load_FromData( name.GetValue(), (void *)pReadResult, fileSize, pPlatformName );
//
//	// free temp buffers and return result
//	if (pReadResult)
//	{
//		NT_DELETE_ARRAY_CHUNK( Mem::MC_PROCEDURAL, pReadResult );
//	}
//
//#ifdef PLATFORM_PC
//	Util::SetToNeutralResources();
//#endif
//
//	// note: Load_FromData with have put the header into the cache
//	return pResult;
//}

//template< typename T, typename L, typename U >
//(typename HeaderManager<T,L,U>::HeaderAdapter)* HeaderManager<T,L,U>::Load_FromData( uint32_t cacheKey, void* pFileData, uint32_t fileSize, const char* pDebugTag )
//{
//	// Check cache first.
//	HeaderAdaptor& adaptor = GetAdapter(cacheKey); // m_obNameToAdaptorMap[ cacheKey ];
//
//	if( adaptor.m_iRefCount != 0 )
//	{
//		adaptor.m_iRefCount++;
//		//return adaptor.m_pobHeader;
//		return &adaptor;
//	}
//
//	adaptor.m_pobHeader = m_obLoader( pFileData, fileSize, pDebugTag );
//
//			user_code_start(Ozz)
//		ntPrintf( "^^^^ %s loaded ^^^^\n", pDebugTag );
//			user_code_end()
//
//	//adaptor.m_iRefCount++;
//	m_obHeaderToNameMap[ adaptor.m_pobHeader ] = cacheKey;
//		return adaptor.m_pobHeader;
//	}
//
template< typename T, typename L, typename U >
bool HeaderManager<T,L,U>::Unload_Neutral( const char* pNeutralName )
{
	if ( pNeutralName )
	{
		char pPlatformName[MAX_PATH];
		MakePlatformName( pNeutralName, pPlatformName );
		if ( Unload_Key( pPlatformName ) )
		{
			user_code_start(Ozz)
				ntPrintf( "^^^^ %s unloaded ^^^^\n", pNeutralName );
			user_code_end()
			return true;
		}
}

	return false;
}

template< typename T, typename L, typename U >
bool HeaderManager<T,L,U>::Unload_Key( uint32_t cacheKey )
	{
		// header should be present on both maps. Otherwise, we're in trouble
	typename NameToAdaptorMap_t::iterator naIt = m_obNameToAdaptorMap.find( cacheKey );
		ntError_p( naIt != m_obNameToAdaptorMap.end(), ("trying to load a missing header") );
		
		typename HeaderToNameMap_t::iterator hnIt = m_obHeaderToNameMap.find( naIt->second->m_pobHeader );
		ntError_p( hnIt != m_obHeaderToNameMap.end(), ("maps out of sync") );

		// decrease the reference counter in adaptor and remove if last one
		if ( (--(naIt->second->m_iRefCount)) == 0 )
		{
			m_obUnloader( naIt->second->m_pobHeader );
			//m_obNameToAdaptorMap.erase( naIt );
			DeleteAdapter(naIt);
			m_obHeaderToNameMap.erase( hnIt );
		return true;
		}
	return false;
	}

//template< typename T, typename L, typename U >
//bool HeaderManager<T,L,U>::Unload( T* ptr )
//{
//	if ( ptr )
//	{
//		// header should be present on both maps. Otherwise, we're in trouble
//		typename HeaderToNameMap_t::iterator hnIt = m_obHeaderToNameMap.find( ptr );
//		ntError_p( hnIt != m_obHeaderToNameMap.end(), ("trying to load a missing header") );
//
//		typename NameToAdaptorMap_t::iterator naIt = m_obNameToAdaptorMap.find( hnIt->second );
//		ntError_p( naIt != m_obNameToAdaptorMap.end(), ("maps out of sync") );
//		
//		
//		// decrease the reference counter in adaptor and remove from map if last one
//		if ( (--(naIt->second->m_iRefCount)) == 0 )
//		{
//			user_code_start(Ozz)
//				ntPrintf( "^^^^ %s unloaded ^^^^\n", ntStr::GetString(  CHashedString(hnIt->second)  ) );
//			user_code_end()
//
//			m_obUnloader( naIt->second->m_pobHeader );
//			DeleteAdapter(naIt);
//			//m_obNameToAdaptorMap.erase( naIt );
//			m_obHeaderToNameMap.erase( hnIt );
//			return true;
//		}
//	}
//	return false;
//}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::Loaded_Neutral
*
*	DESCRIPTION		Test for presence of this animation
*
***************************************************************************************************/
template< typename T, typename L, typename U >
bool HeaderManager<T,L,U>::Loaded_Neutral( const char* pNeutralName ) const
{
	char pPlatformName[MAX_PATH];
	MakePlatformName( pNeutralName, pPlatformName );
	return Loaded_Platform( pPlatformName );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::Loaded_Platform
*
*	DESCRIPTION		Test for presence of this animation
*
***************************************************************************************************/
template< typename T, typename L, typename U >
bool HeaderManager<T,L,U>::Loaded_Platform( const char *pPlatformName ) const
{
	CHashedString nameHash(pPlatformName);
	return Loaded_Cache( nameHash.GetValue() );
}

/***************************************************************************************************
*
*	FUNCTION		CAnimLoader::Loaded_Cache
*
*	DESCRIPTION		Test for presence of this animation
*
***************************************************************************************************/
template< typename T, typename L, typename U >
bool HeaderManager<T,L,U>::Loaded_Cache( uint32_t cacheKey ) const
{
	typename NameToAdaptorMap_t::const_iterator naIt = m_obNameToAdaptorMap.find( cacheKey );
	if (naIt != m_obNameToAdaptorMap.end())
		return (naIt->second->m_iRefCount != 0);
	return false;
}

#endif // end of _HEADERMANAGER_H_

//eof


