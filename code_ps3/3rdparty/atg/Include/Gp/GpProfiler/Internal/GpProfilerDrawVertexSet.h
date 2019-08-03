//--------------------------------------------------------------------------------------------------
/**
	@file		GpProfilerDrawVertexSet.h

	@brief		GpProfiler internal draw 2D vertex set helper.

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_PROFILER_DRAW_VERTEX_SET_H
#define GP_PROFILER_DRAW_VERTEX_SET_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/GcKernel.h>

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class	GpProfilerDrawVertexSet

	@brief	Lightweight helper class for updating a 2D vertex set efficiently. The vertices can then
			be rendered using any primitive type.
			
			Handy for point sets, line strips etc - especially when drawing graphs.
**/
//--------------------------------------------------------------------------------------------------

class GpProfilerDrawVertexSet : public FwNonCopyable
{
public:

	// Enumerations
	
	enum BufferType
	{
		/// kSingleBuffer - for static primitives which are created at initialisation and NOT updated
		///
		/// Consists of a single buffered static buffer. Can only be written to when the GPU
		/// is NOT accessing the static buffer - e.g. during initialisation. Primitives live
		/// forever and can be flushed many times.

		/// kDoubleBuffer - for dynamic primitives NOT updated every frame
		///
		/// Consists of a double buffered static buffer. Can be commited only *once* per frame.
		/// Primitives live until the next write buffer commit and can be flushed many times,
		/// in other words, compute once, draw over many frames.
		
		kSingleBuffer,
		kDoubleBuffer
	};
	
	
	// Explicit Initialisation and Destruction
	
	void	Initialise(uint maxVertices, BufferType bufferType);
	void	Destroy();

    
	// Stream Buffer Update
	
	void	CommitWriteBuffer();                // For kDoubleBuffer only
	
	
	// Accessors
	
	void	SetVertex(uint i, float x, float y, u32 colour);
	
	void	SetNumVertices(uint numVertices);
	uint   	GetNumVertices() const;
	
	
	// Draw Functions	
	
	void	Flush(Gc::PrimitiveType primType, GcContext& context = GcKernel::GetContext());


private:

	// Constants
	
	static const uint	kVertexStride = (sizeof(float) * 3) + sizeof(u32);
	
	
	// Structure Definitions
	
	struct Vertex									///< :NOTE: Must match GcStreamFields vertex format!
	{
		float		m_position[3];					///< Vertex NDC position, (0, 0) top-left, (1, 1) bottom-right
		u32			m_colour;						///< Packed RGBA8 colour.
    };


	// Attributes
	
	uint					m_maxVertices;			///< Max no. of vertices in the vertex set
	BufferType				m_bufferType;			///< GcStreamBuffer buffering scheme
	
	uint					m_readBufferIdx;		///< Index of GPU read stream buffer
	uint					m_writeBufferIdx;		///< Index of PPU write stream buffer
	
	uint					m_numVertices[2];		///< Current no. of vertices (for each buffer)
	GcStreamBufferHandle	m_hVertexStream[2];		///< Vertex set stream
	
	GcShaderHandle			m_hGouraud2dVp;			///< Gouraud 2D vertex shader
	GcShaderHandle			m_hGouraud2dFp;			///< Gouraud 2D fragment shader
	
	
	// Static Attributes
	
	// :NOTE: Static handles so a single set of shaders are shared by all GpProfilerDrawVertexSet objects!
	
	static GcShaderHandle	ms_hOriginalGouraud2dVp; ///< Gouraud 2D vertex shader
    static GcShaderHandle	ms_hOriginalGouraud2dFp; ///< Gouraud 2D fragment shader
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfilerDrawVertexSet::CommitWriteBuffer()
{
	FW_ASSERT(m_bufferType == kDoubleBuffer);
	
    m_readBufferIdx ^= 0x1;
    m_writeBufferIdx ^= 0x1;

	m_numVertices[m_writeBufferIdx] = 0;
}
	
//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfilerDrawVertexSet::SetVertex(uint i, float x, float y, u32 colour)
{

	FW_ASSERT(i < m_maxVertices);
	
	Vertex*	pVertex = (Vertex*) m_hVertexStream[m_writeBufferIdx]->GetDataAddress() + i;
	
	pVertex->m_position[0] = x;
	pVertex->m_position[1] = y;
	pVertex->m_position[2] = -1.0f;
	pVertex->m_colour = colour;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline void GpProfilerDrawVertexSet::SetNumVertices(uint numVertices)
{
	FW_ASSERT(numVertices <= m_maxVertices);
	
	m_numVertices[m_writeBufferIdx] = numVertices;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline uint GpProfilerDrawVertexSet::GetNumVertices() const
{
	return m_numVertices[m_writeBufferIdx];
}

//--------------------------------------------------------------------------------------------------

#endif // GP_PROFILER_DRAW_VERTEX_SET_H
