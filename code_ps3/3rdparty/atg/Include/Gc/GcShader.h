//--------------------------------------------------------------------------------------------------
/**
	@file		GcShader.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_SHADER_H
#define GC_SHADER_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fw/FwStd/FwStdIntrusivePtr.h>
#include	<Fw/FwStd/FwHashedString.h>
#include	<Fw/FwResource.h>
#include 	<Gc/Gc.h>
#include	<Fw/FwMaths/FwVector4.h>
#include	<Fw/FwMaths/FwMatrix44.h>

#ifdef ATG_PC_PLATFORM
#include <system/icetypes.h>
typedef U16endianswap u16e;
typedef U32endianswap u32e;
#else
typedef u16 u16e;
typedef u32 u32e;
#endif

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			Binary shader resource.
**/
//--------------------------------------------------------------------------------------------------

class GcShaderResource
{
public:
	//! The attribute resource structure.
	struct Attribute
	{
		u32e m_hash;
		u32e m_resource;			//!< The attribute resource in [0, 15]
	};

	//! The constant resource structure.
	struct Constant
	{
		u32e m_hash;		
		u8 m_componentType;
		u8 m_componentCount;
		u16e m_rowCount;
		u16e m_resourceStart;		//!< resource start if contiguous, 0xFFFF if not
		u16e m_resourceArrayOffset;	//!< offset to resource array if not contiguous

		bool IsContiguous() const	{ return m_resourceStart != 0xffff; }
	};

	//! The sampler resource structure.
	struct Sampler
	{
		u32e m_hash;
		u32e m_resource;			//!< The sampler resource in [0, 15]
	};

	u32e m_cookie;
	u32e m_programType;
	
	u16e m_gpuType;
	u16e m_attributeCount;
	u16e m_constantCount;
	u16e m_samplerCount;

	u16e m_offsetToAttributes;
	u16e m_offsetToConstants;
	u16e m_offsetToSamplers;
	u16e m_offsetToProgram;

	const Attribute* GetAttributes() const
	{
		return reinterpret_cast< const Attribute* >( 
			reinterpret_cast< const u8* >( this ) + m_offsetToAttributes
		);
	}

	const Constant* GetConstants() const
	{
		return reinterpret_cast< const Constant* >( 
			reinterpret_cast< const u8* >( this ) + m_offsetToConstants
		);
	}

	const Sampler* GetSamplers() const
	{
		return reinterpret_cast< const Sampler* >( 
			reinterpret_cast< const u8* >( this ) + m_offsetToSamplers
		);
	}

	const u16e* GetResourceArray( u16 offset ) const
	{
		FW_ASSERT( offset != 0xffff );
		return reinterpret_cast< const u16e* >( 
			reinterpret_cast< const u8* >( this ) + offset
		);
	}

	const Ice::Render::VertexProgram* GetVertexProgram() const
	{
		FW_ASSERT( m_programType == Gc::kVertexProgram );
		return reinterpret_cast< const Ice::Render::VertexProgram* >( 
			reinterpret_cast< const u8* >( this ) + m_offsetToProgram
		);
	}

	const Ice::Render::FragmentProgram* GetFragmentProgram() const
	{
		FW_ASSERT( m_programType == Gc::kFragmentProgram );
		return reinterpret_cast< const Ice::Render::FragmentProgram* >( 
			reinterpret_cast< const u8* >( this ) + m_offsetToProgram
		);
	}
};

//--------------------------------------------------------------------------------------------------
/**
	@class

	@brief			Shader object.
**/
//--------------------------------------------------------------------------------------------------

class GcShader : public FwNonCopyable
{
public:
	// Constants

	static const uint kInvalidIndex = ( uint )-1;

	// Query Functions
	
	static int QuerySizeInBytes();

	static int QueryResourceSizeInBytes(const FwResourceHandle& hResource);

	static int QueryResourceSizeInBytes(const void* pResource);
	

	// Creation
    
	static GcShaderHandle Create( const FwResourceHandle& hResource, 
								  Gc::BufferType bufferType = Gc::kStaticBuffer, 
								  Gc::MemoryContext memoryContext = Gc::kVideoMemory, 
								  void* pClassMem = NULL );

	static GcShaderHandle Create( const void* pResource, 
								  size_t size, 
								  Gc::BufferType bufferType = Gc::kStaticBuffer, 
								  Gc::MemoryContext memoryContext = Gc::kVideoMemory, 
								  void* pClassMem = NULL );

	// Destruction

	~GcShader();

	// Resource access

	const GcShaderResource*	GetResource() const;

	// Fragment program instance control

	Gc::BufferType		GetBufferType() const;
	Gc::MemoryContext	GetMemoryContext() const;

	void*				GetDataAddress() const;
	uint				GetDataOffset() const;
	uint				GetControl() const;

	bool				QueryGetNewScratchMemory() const;
	void				GetNewScratchMemory( const GcShaderHandle& hBaseShader = GcShaderHandle() );
	void				SetDataAddress( void* pUserAddress );

	// Fragment program constant patching

	void PatchConstant( uint index, 
						const float* pValues, 
						uint rowOffset = 0 );

	void PatchConstant( uint index, 
						const float* pValues, 
						uint rowOffset, 
						uint rowCount );

	void PatchConstant( uint index, 
						float value, 
						uint rowOffset = 0 );

	void PatchConstant( uint index, 
						const FwVector4& vec, 
						uint rowOffset = 0 );

	void PatchConstant( uint index, 
						const FwMatrix44& mat, 
						uint rowOffset = 0, 
						uint rowCount = 4 );

	// Fragment program threading control (PS3 only)

	uint			GetInitialRegisterCount() const;
	uint			GetRegisterCount() const;
    void			SetRegisterCount( uint registerCount );

	// Generic shader property access

	Gc::ProgramType GetType() const;

	uint 			GetAttributeCount() const;
	uint 			GetConstantCount() const;
	uint 			GetSamplerCount() const;

	// Attribute access by index (fast)
	
	FwHashedString 	GetAttributeHash( uint index ) const;
	uint			GetAttributeResourceIndex( uint index ) const;

	// Attribute access by hash (slow)
	
	uint 			GetAttributeIndex( FwHashedString hash ) const;
	uint			GetAttributeResourceIndex( FwHashedString hash ) const;

	// Constant access by index (fast)
	
	FwHashedString	GetConstantHash( uint index ) const;

	bool			IsConstantContiguous( uint index ) const;
	uint			GetConstantResourceIndex( uint index, uint row = 0 ) const;
	
	// Constant access by hash (slow)
	
	uint 			GetConstantIndex( FwHashedString hash ) const;
	
	bool			IsConstantContiguous( FwHashedString hash ) const;
	uint			GetConstantResourceIndex( FwHashedString hash, uint row = 0 ) const;
	
	// Sampler access by index (fast)

	FwHashedString	GetSamplerHash( uint index ) const;
	uint 			GetSamplerResourceIndex( uint index ) const;
	
	// Sampler access by hash (slow)

	uint 			GetSamplerIndex( FwHashedString hash ) const;
	uint			GetSamplerResourceIndex( FwHashedString hash ) const;

private:

	// Private construction

	explicit GcShader( const void* pResource, 
					   Gc::BufferType bufferType, 
					   Gc::MemoryContext memoryContext, 
					   bool ownsMemory );

	// Update functions

	void UpdateOffset();

	// Validity checks

	void CheckAddressValid() const;


	const GcShaderResource*		m_pHeader;			//!< The resource pointer
	FwResourceHandle			m_hResource;		//!< The resource handle (optional)
	
	Gc::BufferType				m_bufferType;
	Gc::MemoryContext			m_memoryContext;
	void*						m_pCodeInstance;	//!< Fragment program instance
	uint						m_codeOffset;		//!< Cached fragment program offset
	uint						m_control;			//!< Cache control register.

	bool						m_ownsMemory;
	int							m_refCount;

	// Friends

	friend void	IntrusivePtrAddRef( GcShader* p )
	{
		++p->m_refCount;
	}
	friend void	IntrusivePtrRelease( GcShader* p )
	{
		if ( --p->m_refCount == 0 )
		{
			bool ownsMemory = p->m_ownsMemory;
			p->~GcShader();
			if(ownsMemory)
				FW_DELETE_ARRAY( (u8*)p );
		}
	}
	friend u32 IntrusivePtrGetRefCount( GcShader* p )
	{
		return (u32)( p->m_refCount );
	}
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gc/GcShader.inl>

//--------------------------------------------------------------------------------------------------

#endif // GC_SHADER_H
