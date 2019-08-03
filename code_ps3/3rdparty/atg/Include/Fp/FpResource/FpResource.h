//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Simple Resource Manager

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FP_RESOURCE_H
#define	FP_RESOURCE_H

#include <Fw/FwResource.h>
#include <Fw/FwStd/FwStdMap.h>

class	FpResource;

//--------------------------------------------------------------------------------------------------
/**
	@class		FpResourceManager
	
	@brief		Management class for our simple resource system.

	Our simple resource system loads resources if they are not already in RAM. If the resource is
	already in RAM, then additional loading does not take place. As the address associated with a
	resources data is accessed via a const void pointer (ie: it won't be changing), this allows for
	efficient sharing of loaded data. When all uses of a resource are released (so the reference
	count hits zero), then the loaded resource data is removed from memory.

	This resource system uses STL map to allow for efficient access to loaded resources by name. At
	the	time of writing, the system does not use a pool for storage of FpResource/FwResource objects,
	although this could be added at some point in the future. From a project point of view, I would
	expect resources to be exported data (in some kind of TOC).

**/
//--------------------------------------------------------------------------------------------------

class	FpResourceManager : public FwSingleton<FpResourceManager>
{
public:
	// Wrap our singleton construction/destruction
	static	void	Initialise( void )	{ FW_NEW FpResourceManager();	}
	static	void	Shutdown( void );

	static const int kMaxSearchPaths = 20;

	// Add a base data-loading path. Allocates its own memory for the string, too, just in case.
	// Path to be specified is to be relative to lcnslsrv's base path
	
	void AddSearchPath( const char* pSearchPath );

#ifdef ATG_ASSERTS_ENABLED
	static inline void ResourceFoundAsserts(bool enabled);
#endif

private:
	FpResourceManager();

	// Here we define our handler interfaces
	static	FwResource*		FindResourceHandler( const char* pResourceName );
	static	void			ReleaseResourceHandler( FwResource& resource );

	// Our map, used to associate a 32-bit hashed string with a pointer to an FpResource object
	FwStd::Map< u32, FpResource* >	m_resourceMap;

	char*						m_pSearchPath[ kMaxSearchPaths ]; 	/// array of path names mgr will attempt to look in
	int							m_numSearchPaths;
	
#ifdef ATG_ASSERTS_ENABLED
	static bool					m_resourceFoundAssertsEnabled;		/// Runtime enable/disable of 'resource found' asserts.
#endif
};

//--------------------------------------------------------------------------------------------------
/**
	@class		FpResource
	
	@brief		Runtime enable / disable of 'resource found' assertions.

	By default the FpResourceManager will assert when a resource is not found. However, using this function
	these asserts can be disabled, thus allowing you to implement your own application-defined 'resource not
	found' behaviour.
	
	The enabled / disabled status of these assertions can be changed at any time.
**/
//--------------------------------------------------------------------------------------------------

#ifdef ATG_ASSERTS_ENABLED

inline void FpResourceManager::ResourceFoundAsserts(bool enabled)
{
	m_resourceFoundAssertsEnabled = enabled;
}

#endif

//--------------------------------------------------------------------------------------------------
/**
	@class		FpResource
	
	@brief		Our resource object, derived from the base FwResource clas.

	Ok, so in this particular case we don't have any additional data present above that which is 
	already defined in FwResource, but this is defined for completeness. If we needed additional
	data at a resource level, it would go here. 

	@note		Although you would add additional data here (and the appropriate interfaces), be
				aware that you need to access those interfaces via the GetResource() method on
				FwResource. It's not possible to expose those interfaces on the FwResourceHandle
				object, you see. 
**/
//--------------------------------------------------------------------------------------------------

class	FpResource : public FwResource
{
	friend class FpResourceManager;			// build fix for gcc: requires "class" keyword

public:
	FpResource()	{};
	FpResource( FwHashedString name, int byteSize, const void* pData ) : FwResource( name, byteSize, pData ) {}

protected:
	// If you wanted additional data on a resource, you'd place it here and then supply public
	// interfaces
};

#endif	//FP_RESOURCE_H
