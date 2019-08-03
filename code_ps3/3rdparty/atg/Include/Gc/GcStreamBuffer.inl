//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_STREAM_BUFFER_INL_H
#define GC_STREAM_BUFFER_INL_H

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Query the amount of host or video memory required by the GcStreamBuffer's resource data
				- in this case, the vertex data.

	In otherwords, the size of host or video memory allocated if a GcStreamBuffer is created with the
	properties.
	
	@return		Size in bytes

	@note		The size returned takes into account any internal / trailing padding required, however,
				it does *not* take into account any additional bytes required for start alignment.

	@note		The size returned is for the GcStreamBuffer vertex data only and does *not* include the
				class memory required by the GcStreamBuffer object (which is distinct). To obtain the latter
				please use the QuerySizeInBytes() function.
**/
//--------------------------------------------------------------------------------------------------

inline int GcStreamBuffer::QueryResourceSizeInBytes(int vertexCount, int vertexStride)
{
	return vertexStride * vertexCount;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Query the amount of host or video memory required by the GcStreamBuffer's resource data
				- in this case, the index data.

	In otherwords, the size of host or video memory allocated if a GcStreamBuffer is created with the
	properties.
	
	@return		Size in bytes

	@note		The size returned takes into account any internal / trailing padding required, however,
				it does *not* take into account any additional bytes required for start alignment.

	@note		The size returned is for the GcStreamBuffer index data only and does *not* include the
				class memory required by the GcStreamBuffer object (which is distinct). To obtain the latter
				please use the QuerySizeInBytes() function.
**/
//--------------------------------------------------------------------------------------------------

inline int GcStreamBuffer::QueryResourceSizeInBytes(Gc::StreamIndexType indexType, uint indexCount)
{
	int indexStride = (indexType == Gc::kIndex16) ? 2 : 4;

	return indexStride * indexCount;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the number of fields in the field array.
**/
//--------------------------------------------------------------------------------------------------

inline int GcStreamBuffer::GetNumFields() const
{
	return m_numFields;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the stride of the data.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcStreamBuffer::GetStride() const
{
	return m_stride;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the number of data entries.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcStreamBuffer::GetCount() const
{
	return m_count;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the field at the given index.
**/
//--------------------------------------------------------------------------------------------------

inline const GcStreamField& GcStreamBuffer::GetField(int index) const
{
	FW_ASSERT(index < m_numFields);

	return m_pFields[index];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if this stream is an index buffer.
**/
//--------------------------------------------------------------------------------------------------

inline bool GcStreamBuffer::IsIndexBuffer() const
{
	return m_isIndexBuffer;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the index type for index buffers.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::StreamIndexType GcStreamBuffer::GetIndexType() const
{
	return m_indexType;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Reallocates the stream buffer in a new piece of scratch memory.
**/
//--------------------------------------------------------------------------------------------------

inline void GcStreamBuffer::GetNewScratchMemory( u_int iCountOveride ) 
{
	if ( iCountOveride == 0xffffffff )
	{
		GcResource::AllocateScratchMemory( m_stride * m_count, Gc::kStreamBufferAlignment );
	}
	else
	{
		iCountOveride = min( m_count, iCountOveride );
		GcResource::AllocateScratchMemory( m_stride * iCountOveride, Gc::kStreamBufferAlignment );
	}
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a new custom stream buffer data address.
**/
//--------------------------------------------------------------------------------------------------

inline void GcStreamBuffer::SetDataAddress( void* pUserAddress )
{
	GcResource::SetUserAddress( pUserAddress );
}

#endif // ndef GC_STREAM_BUFFER_INL_H
