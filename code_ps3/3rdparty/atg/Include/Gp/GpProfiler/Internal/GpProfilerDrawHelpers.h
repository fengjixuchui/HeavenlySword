//--------------------------------------------------------------------------------------------------
/**
	@file		GpProfilerDrawHelpers.h

	@brief		GpProfiler internal draw helpers.

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_PROFILER_DRAW_HELPERS_H
#define GP_PROFILER_DRAW_HELPERS_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/GcKernel.h>

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class	GpProfilerDrawHelpers

	@brief	Helper class for drawing 2D lines and triangles
**/
//--------------------------------------------------------------------------------------------------

class GpProfilerDrawHelpers : public FwNonCopyable
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
	
	void	Initialise(uint maxTris, uint maxLines, BufferType bufferType);
	void	Destroy();
	
    
	// Stream Buffer Update
	
	void	CommitWriteBuffer();                // For kDoubleBuffer only
	
	
	// Draw Functions	
	
	void	DrawLine(float x0, float y0, float x1, float y1, u32 c0, u32 c1);
    void	DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, u32 c0, u32 c1, u32 c2);

	void	Flush(GcContext& context = GcKernel::GetContext());
    

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
	
	uint					m_maxTris;				///< Max no. of triangles
	uint					m_maxLines;				///< Max no. of lines
	BufferType				m_bufferType;			///< GcStreamBuffer buffering scheme
	
	uint					m_readBufferIdx;		///< Index of GPU read stream buffer
	uint					m_writeBufferIdx;		///< Index of PPU write stream buffer
	
	uint					m_numTris[2];			///< Current no. of triangles
	uint					m_numLines[2];			///< Current no. of lines
	
	GcStreamBufferHandle	m_hTriVertexStream[2];	///< Triangle vertex stream
	GcStreamBufferHandle	m_hLineVertexStream[2];	///< Line vertex stream
	
	GcShaderHandle			m_hGouraud2dVp;			///< Gouraud 2D vertex shader
	GcShaderHandle			m_hGouraud2dFp;			///< Gouraud 2D fragment shader
	
	
	// Static Attributes
	
	// :NOTE: Static handles so a single set of shaders are shared by all GpProfilerDrawHelpers objects!
	
	static GcShaderHandle	ms_hOriginalGouraud2dVp; ///< Gouraud 2D vertex shader
    static GcShaderHandle	ms_hOriginalGouraud2dFp; ///< Gouraud 2D fragment shader
};

//--------------------------------------------------------------------------------------------------

#endif // GP_PROFILER_DRAW_HELPERS_H
