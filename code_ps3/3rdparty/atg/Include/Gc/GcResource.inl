//--------------------------------------------------------------------------------------------------
/**
	@file		GcResource.inl

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_RESOURCE_INL
#define GC_RESOURCE_INL

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the derived class that this resource is a base class for.

	This function is only called internally.
**/
//--------------------------------------------------------------------------------------------------

inline void GcResource::SetDerivedClass( GcResource::DerivedClass derivedClass )
{
	m_derivedClassAndOwnsMemory = ( m_derivedClassAndOwnsMemory & 0x80 ) | ( u8 )derivedClass;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the derived class that this resource is a base class for.

	This function is only called internally.
**/
//--------------------------------------------------------------------------------------------------

inline GcResource::DerivedClass GcResource::GetDerivedClass() const
{
	return ( DerivedClass )( m_derivedClassAndOwnsMemory & 0x7f );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the buffer type of the resource.

	This function is only called internally.
**/
//--------------------------------------------------------------------------------------------------

inline void GcResource::SetBufferType( Gc::BufferType bufferType )
{
	m_bufferType = ( u8 )bufferType;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the buffer type of the resource.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::BufferType GcResource::GetBufferType() const
{
	return ( Gc::BufferType )m_bufferType;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		True if the memory for this class will be released on destruction.

	This function is only called internally.
**/
//--------------------------------------------------------------------------------------------------

inline void GcResource::SetOwnsClassMemory( bool ownsClassMemory )
{
	if( ownsClassMemory )
		m_derivedClassAndOwnsMemory |= 0x80;
	else
		m_derivedClassAndOwnsMemory &= 0x7f;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		True if the memory for this class will be released on destruction.

	This function is only called internally.
**/
//--------------------------------------------------------------------------------------------------

inline bool GcResource::OwnsClassMemory() const
{
	return ( m_derivedClassAndOwnsMemory & 0x80 ) != 0;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the static allocation type.

	This function is only called internally.
**/
//--------------------------------------------------------------------------------------------------

inline void GcResource::SetAllocationType( AllocationType allocType )
{
	m_allocationType = ( u8 )allocType;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the static allocation type.

	This function is only called internally.
**/
//--------------------------------------------------------------------------------------------------

inline GcResource::AllocationType GcResource::GetAllocationType() const
{
	return ( AllocationType )m_allocationType;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the memory context for the current allocation.

	This function is only called internally.
**/
//--------------------------------------------------------------------------------------------------

inline void GcResource::SetMemoryContext( Gc::MemoryContext context )
{
	m_memoryContext = ( u8 )context;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the resource size (in bytes)

	This function is only called internally.
**/
//--------------------------------------------------------------------------------------------------

inline void GcResource::SetSize( int size )
{
	FW_ASSERT(size > 0);
	
	m_size = size;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the memory context of the current allocation.

	Note that for scratch allocations, this may not be valid until the first scratch allocation.
	For user allocations, this may change depending on the user address that has been set.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::MemoryContext GcResource::GetMemoryContext() const
{
	return ( Gc::MemoryContext )m_memoryContext;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Performs no checking at all in non-debug mode.
**/
//--------------------------------------------------------------------------------------------------

#ifndef ATG_DEBUG_MODE
inline void GcResource::CheckAddressValid() const {}
#endif // ndef ATG_DEBUG_MODE

//--------------------------------------------------------------------------------------------------
/**
	@brief		Retrieve the memory from opengl associated with this resource
				eg texture rendered to
				PC only
**/
//--------------------------------------------------------------------------------------------------

#ifdef ATG_PC_PLATFORM
inline void GcResource::Retrieve()
{
	extern void atglRetrieveMemory(u32, u32, bool);
	atglRetrieveMemory(m_offset, m_size, m_memoryContext == Gc::kVideoMemory);
}
#endif

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the current data address for this object.
**/
//--------------------------------------------------------------------------------------------------

inline void* GcResource::GetDataAddress() const
{
	// check it is valid
	CheckAddressValid();

	// return the address
	return m_pAddress;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the current data offset for this object.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcResource::GetDataOffset() const
{
	// check it is valid
	CheckAddressValid();

	// return the address
	return m_offset;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the allocation size of this resource.
**/
//--------------------------------------------------------------------------------------------------

inline int GcResource::GetSize() const
{
	return m_size;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Updates the local offset using the local address and context.
**/
//--------------------------------------------------------------------------------------------------

inline void GcResource::UpdateOffset()
{
	if( GetMemoryContext() == Gc::kVideoMemory )
	{
		m_offset = Ice::Render::TranslateAddressToOffset( m_pAddress );
	}
	else
	{
		m_offset = Ice::Render::TranslateAddressToIoOffset( m_pAddress );
		FW_ASSERT_MSG(m_offset != 0xffffffffu, ("0x%p is not a mapped Host Memory address\n", m_pAddress));
	}
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Increments the reference count.
**/
//--------------------------------------------------------------------------------------------------
	
inline void IntrusivePtrAddRef( GcResource* pResource )
{
	++pResource->m_refCount;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the reference count.
**/
//--------------------------------------------------------------------------------------------------
	
inline u32 IntrusivePtrGetRefCount( GcResource* pResource )
{
	return ( u32 )( pResource->m_refCount );
}

#endif // ndef GC_RESOURCE_INL
