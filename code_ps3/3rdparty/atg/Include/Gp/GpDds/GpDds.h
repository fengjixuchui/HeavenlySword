//--------------------------------------------------------------------------------------------------
/**
	@file

	@brief		DDS Loading Code.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef GPDDS_GPDDS_H
#define GPDDS_GPDDS_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/Gc.h>
#include <Gc/GcTexture.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		DDS Loader.

	This currently wraps the GcTexture native DDS loader.  The DDS loading code will be migrated to
	here in a future release.
**/
//--------------------------------------------------------------------------------------------------

class GpDds
{
public:
	// Simple (single texture only) interface

	static GcTextureHandle	LoadTexture( const FwResourceHandle& hResource,	
										 void* pClassMemory = NULL,
										 Gc::MemoryContext memoryContext = Gc::kVideoMemory, 
										 bool forceLinear = false );

	static GcTextureHandle	LoadTexture( const void* pResource, 
										 uint resourceSize,
										 void* pClassMemory = NULL,
										 Gc::MemoryContext memoryContext = Gc::kVideoMemory, 
										 bool forceLinear = false );
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Loads a DDS texture.
**/
//--------------------------------------------------------------------------------------------------

inline GcTextureHandle GpDds::LoadTexture( const FwResourceHandle& hResource, 
										   void* pClassMemory, 
										   Gc::MemoryContext memoryContext, 
										   bool forceLinear )
{
	return LoadTexture( hResource.GetData(), hResource.GetSize(), pClassMemory, memoryContext, forceLinear );
}

#endif // ndef GPDDS_GPDDS_H
