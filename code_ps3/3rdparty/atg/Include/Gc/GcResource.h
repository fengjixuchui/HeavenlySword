//--------------------------------------------------------------------------------------------------
/**
	@file		GcResource.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_RESOURCE_H
#define GC_RESOURCE_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/Gc.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class

	@brief			Base class for shared resources in Gc.

	This class handles all shared memory in Gc. It can be operated in 4 modes:

	* Static allocation (use AllocateColourSurface/AllocateDepthStencilSurface/AllocateLinear)
	* Scratch allocation (call AllocateScratchMemory from the derived class)
	* User allocation (call SetUserAddress from the derived class)
	* Share-from-parent allocation (call SetEverythingFromParent with an optional data offset)

	Cleanup is handled in the destructor. Note that IntrusivePtrRelease uses the DerivedClass
	type to call the correct destructor manually without incurring cost of a virtual destructor.
**/
//--------------------------------------------------------------------------------------------------

class GcResource : public FwNonCopyable
{
public:
	//! Instead of using virtuals we use this enum to identify the derived class.
	enum DerivedClass
	{
		kStreamBuffer,				//!< Used to detect a GcStreamBuffer.
		kRenderBuffer, 				//!< Used to detect a GcRenderBuffer.
		kTexture, 					//!< Used to detect a GcTexture.
	};

	//! This enum is used for the type of memory allocation we've done.
	/*! This will likely disappear when tiling is exposed through Gc as we will
		only need linear allocations.
	*/
	enum AllocationType
	{
		kColourSurface,				//!< We are a colour surface allocation.
		kDepthStencilSurface,		//!< We are a depth stencil surface allocation.
		kLinear,					//!< We are a linear allocation.
	};


	// Construction

	explicit GcResource( DerivedClass derivedClass, Gc::BufferType bufferType, bool ownsClassMemory );


	// Destruction

	~GcResource();


	// Public access

	Gc::BufferType GetBufferType() const;
	Gc::MemoryContext GetMemoryContext() const;

	void* GetDataAddress() const;
	uint GetDataOffset() const;

	int GetSize() const;


	// Utility functions

	static Gc::MemoryContext GetAddressContext( const void* pAddress );

	// PC only functionality
#ifdef ATG_PC_PLATFORM
	void Retrieve();
#endif

protected:

	// Static allocations (sets buffer type to static)

	void AllocateColourSurface( uint width, uint height, Gc::BufferFormat format, Gc::MultisampleMode multisample, Gc::MemoryContext context, uint& pitch );
	void AllocateDepthStencilSurface( uint width, uint height, Gc::BufferFormat format, Gc::MultisampleMode	multisample, Gc::MemoryContext context, uint& pitch );
	void AllocateLinear( uint amount, uint alignment, Gc::MemoryContext context );
	void FreeAllocation();

	// Scratch allocations (requires buffer type of scratch)

	void AllocateScratchMemory( uint amount, uint alignment );

	// User allocations (requires buffer type of user)

	void SetUserSize( uint size );
	void SetUserAddress( void* pAddress );


	// Use parent memory

	void SetEverythingFromParent( GcResource* pParent, int offset = 0 );


	// Protected access

	void SetDerivedClass( DerivedClass derivedClass );
	DerivedClass GetDerivedClass() const;

	void SetBufferType( Gc::BufferType bufferType );

	void SetOwnsClassMemory( bool ownsClassMemory );
	bool OwnsClassMemory() const;

	void SetAllocationType( AllocationType allocType );
	AllocationType GetAllocationType() const;

	void SetMemoryContext( Gc::MemoryContext context );

	void SetSize( int size );


	// Validity checks

	void CheckAddressValid() const;

private:
	
	// Update functions

	void UpdateOffset();

	// Intrusive pointer handlers

	friend void	IntrusivePtrAddRef( GcResource* p );
	friend void	IntrusivePtrRelease( GcResource* p );
	friend u32 IntrusivePtrGetRefCount( GcResource* p );

	int	m_refCount;					//!< Internal reference count for the handle.

	u8 m_derivedClassAndOwnsMemory;
	u8 m_bufferType;
	u8 m_allocationType;
	u8 m_memoryContext;

    void* m_pAllocation;			//!< The original allocation address (NULL of not owned).
	int m_size;						//!< The resource data / allocation size (is *always* valid).
	void* m_pAddress;				//!< The current local address.
	uint m_offset;					//!< The current local offset (within the memory context).
	GcResourceHandle m_hParent;		//!< The owner of the allocation or NULL if we own it.

	static int ms_surfaceCount;		//!< Static counter to check colour/depth surface limit.
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gc/GcResource.inl>

//--------------------------------------------------------------------------------------------------

#endif // ndef GC_RESOURCE_H
