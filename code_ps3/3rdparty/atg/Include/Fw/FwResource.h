//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Thin and relatively abstract interface to loaded resources

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_RESOURCE_H
#define	FW_RESOURCE_H

#include <Fw/FwStd/FwHashedString.h>

class	FwResource;
class	FwResourceHandle;

//--------------------------------------------------------------------------------------------------
/**
	@class		FwResourceCtrl
	
	@brief		Provides an interface between base classes (FwResource and FwResourceHandle),
				and the class on the project side that manages resource acquisition and release.

	Rather than have FwResource be an object that has virtual functions (and therefore a pointer to
	a vtable), resources interface with project-specific resource management systems via this static
	class. In this way, FwResource objects (or derived works) may be exported from resource building
	tools and used directly. If there were virtual functions on FwResource, this wouldn't be possible.

	Obviously, all handlers in this object must be set by the project-specific resource management
	system prior to any resource acquisition taking place.
**/
//--------------------------------------------------------------------------------------------------

class	FwResourceCtrl : public FwNonCopyable
{
	friend	class FwResource;
	friend	class FwResourceHandle;

	// Typedefs used for our callbacks
	typedef FwResource*		( *FindResourceHandler )( const char* pResourceName );
	typedef	void			( *ReleaseResourceHandler )( FwResource& resource );

public:
	static	void			SetFindResourceHandler( FindResourceHandler pHandler );	
	static	void			SetReleaseResourceHandler( ReleaseResourceHandler pHandler );	

protected:
	static	FwResource*		FindResource( const char* pResourceName )
	{
		FW_ASSERT( ms_pFindResource );
		return ms_pFindResource( pResourceName );
	}

	static	void			ReleaseResource( FwResource& resource )
	{
		FW_ASSERT( ms_pReleaseResource );
		ms_pReleaseResource( resource );
	}

private:
	static	FindResourceHandler		ms_pFindResource;
	static	ReleaseResourceHandler	ms_pReleaseResource;
};

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the handler used to obtain a resource using a resource name

	@param		Pointer to the handler function
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwResourceCtrl::SetFindResourceHandler( FindResourceHandler pHandler )
{
	ms_pFindResource = pHandler;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the handler used to release a resource (when the reference count reaches 0)

	@param		Pointer to the handler function
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwResourceCtrl::SetReleaseResourceHandler( ReleaseResourceHandler pHandler )
{
	ms_pReleaseResource = pHandler;
}

//--------------------------------------------------------------------------------------------------
/**
	@class		FwResource
	
	@brief		Base class representing a resource. Only FwResourceHandle and derived
				resource types (eg: FpResource) have access to the data in here..	
**/
//--------------------------------------------------------------------------------------------------

class	FwResource
{
	friend	class FwResourceHandle;

protected:
	FwResource()	{}

	FwResource( FwHashedString name, int byteSize, const void* pData )
	{
		m_refCount	= 0;
		m_name		= name;
		m_byteSize	= byteSize;
		m_pData		= pData;
	}

	// Main accessors.. only accessed via FwResourceHandle..
	int				GetRefCount( void ) const	{ return m_refCount;	};
	FwHashedString	GetName( void ) const		{ return m_name;		};
	int				GetSize( void ) const		{ return m_byteSize;	};
	const void*		GetData( void ) const		{ return m_pData;		};

	// Reference counting
	void			AddRef( void );
	void			Release( void );

private:
	int				m_refCount;
	FwHashedString	m_name;
	int				m_byteSize;
	const void*		m_pData;
};

//--------------------------------------------------------------------------------------------------
/**
	@brief		Increments the reference count for an object. It should be called for every new
				copy of a pointer to a given object.
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwResource::AddRef( void )
{ 
	m_refCount++;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Decrements the reference count for an object. If the reference count on the object
				reaches 0, the object is released.
**/
//--------------------------------------------------------------------------------------------------

inline	void	FwResource::Release( void )
{
	FW_ASSERT( m_refCount );
	m_refCount--;
	if ( m_refCount == 0 )
		FwResourceCtrl::ReleaseResource( *this );
}

//--------------------------------------------------------------------------------------------------
/**
	@class		FwResourceHandle
	
	@brief		The main project-visible object associated with resource management. 

	Resources need to be carefully controlled. Failure to adequately deal with access & lifetime 
	issues can result in hard-to-track bugs involving use of invalid pointers and, ultimately, freed
	memory. We also need to separate the load & use of resources.. after all, the API internally 
	may require resources, but it cannot dictate how those resources should be loaded into memory.

	To allow this separation, the API will access all resources via this class. The class behaves
	like a smart object, managing references on the underlying resource structures, and allowing
	access to fundamental data associated with resources (name hash, size, and address in main RAM).

	Usage of FwResourceHandle is particularly straightforward.. just check out these examples:
	
	@code
			void	PrintFile( const char* pFilename )
			{
				// Create a handle to the specified file.
				FwResourceHandle	fileHandle( pFilename );

				// Get information about the resource
				const char*			pFileData	= ( const char* )fileHandle.GetData();
				int					fileSize	= fileHandle.GetSize();
					
				// Now print the contents out, a character at a time.
				for ( int loop = 0; loop < fileSize; i++ )
					FwPrintf( "%c", pData[ loop ] );

				// At this point, fileHandle goes out of scope, and the file is released.
			}
	@endcode

	When dealing with object ownership of resources, you can (and should) embed an FwResourceHandle
	object within your class. In that way, once the class is destroyed it will automatically release
	the handle. 
			
**/
//--------------------------------------------------------------------------------------------------

class	FwResourceHandle
{
public:
	// Construction
	FwResourceHandle()	{ m_pResource = NULL; }
	explicit FwResourceHandle( const char* pResourceName )
	{
		m_pResource = FwResourceCtrl::FindResource( pResourceName );
		
		if (m_pResource)
			m_pResource->AddRef();
	}

	// Destruction
	~FwResourceHandle()
	{
		if ( m_pResource )
			m_pResource->Release();
	}

	// Copy/assignment
	FwResourceHandle( const FwResourceHandle& handle ) : m_pResource( handle.m_pResource )
	{
		if ( m_pResource )
			m_pResource->AddRef();
	}

	FwResourceHandle& operator = ( const FwResourceHandle& handle )
	{
		FwResourceHandle temp( handle );
		Swap( temp );
		return *this;
	}
	
	// Release
	void	Release( void )
	{
		if ( m_pResource )
		{
			m_pResource->Release();
			m_pResource = NULL;
		}
	}
	
	// Accessors
	bool			IsValid( void ) const		{ return ( m_pResource != NULL ); }
	FwResource*		GetResource( void )			{ FW_ASSERT( m_pResource ); return m_pResource; }
	int				GetRefCount( void ) const	{ FW_ASSERT( m_pResource ); return m_pResource->GetRefCount(); }
	FwHashedString	GetName( void ) const		{ FW_ASSERT( m_pResource ); return m_pResource->GetName(); }
	int				GetSize( void ) const		{ FW_ASSERT( m_pResource ); return m_pResource->GetSize(); }
	const void*		GetData( void ) const		{ FW_ASSERT( m_pResource ); return m_pResource->GetData(); }

private:
	void Swap( FwResourceHandle& arg )
	{
		FwResource* pTemp = m_pResource;
		m_pResource = arg.m_pResource;
		arg.m_pResource = pTemp;
	}
	
	FwResource*		m_pResource;
};

#endif	//FW_RESOURCE_H
