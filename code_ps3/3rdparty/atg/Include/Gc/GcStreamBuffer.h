//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Graphics Core Stream Buffer

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_STREAM_BUFFER_H
#define GC_STREAM_BUFFER_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/Gc.h>
#include <Gc/GcResource.h>
#include <Gc/GcStreamField.h> 
#include <Fw/FwStd/FwHashedString.h>

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class

	@brief		A stream buffer object.
**/
//--------------------------------------------------------------------------------------------------

class GcStreamBuffer : public GcResource
{
public:

	// Query Functions

	static int QuerySizeInBytes( int fieldCount );

	static int QueryResourceSizeInBytes(int vertexCount, int vertexStride);

	static int QueryResourceSizeInBytes(Gc::StreamIndexType indexType, uint indexCount);
	
	
	// Creation

	static GcStreamBufferHandle CreateVertexStream(	int vertexCount,
													int vertexStride,
													int	fieldCount,
													const GcStreamField* pFieldArray,
													Gc::BufferType bufferType  = Gc::kStaticBuffer,
													void* pClassMemory = NULL,
													Gc::MemoryContext memoryContext = Gc::kVideoMemory );

	static GcStreamBufferHandle	CreateIndexStream( Gc::StreamIndexType indexType,
												   uint indexCount,
												   Gc::BufferType bufferType = Gc::kStaticBuffer,
												   void* pClassMemory = NULL,
												   Gc::MemoryContext memoryContext = Gc::kVideoMemory );

	// Operations

	void					Write( const void* pData );
	void					Write( const void* pData, uint offset, uint size );

	// Access

	uint					GetCount() const;
	uint					GetStride() const;
	int						GetNumFields() const;
	const GcStreamField&	GetField( int index ) const;
	Gc::StreamIndexType		GetIndexType() const;

	// Inquiry

	bool					IsIndexBuffer() const;

	// Stream Buffer Data Interface

	bool					QueryGetNewScratchMemory( u_int iCountOveride = 0xffffffff ) const;
	void					GetNewScratchMemory( u_int iCountOveride = 0xffffffff );
	void					SetDataAddress( void* pUserAddress );

private:

	// Construction

	GcStreamBuffer( bool isIndexBuffer,
					Gc::StreamIndexType indexType,
					int stride,
					int count,
					int fieldCount,
					GcStreamField* pFields,
					Gc::BufferType bufferType, 
					bool ownsClassMemory );

	// Attributes

	bool					m_isIndexBuffer;
	Gc::StreamIndexType		m_indexType;

	uint					m_stride;
	uint					m_count;

	int						m_numFields;
	GcStreamField*			m_pFields;
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gc/GcStreamBuffer.inl>

//--------------------------------------------------------------------------------------------------

#endif // GC_STREAM_BUFFER_H
