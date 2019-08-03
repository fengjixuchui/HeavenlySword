//--------------------------------------------------------------------------------------------------
/**
	@file		GpBasicPrims2d.h

	@brief		Basic 2D primitive drawing.

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_BASIC_PRIMS_2D_H
#define GP_BASIC_PRIMS_2D_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/GcKernel.h>
#include <Gc/GcStreamField.h>

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class	GpBasicPrims2d

	@brief	Helper class for 2D primitive drawing of lines, triangles, quads etc.
**/
//--------------------------------------------------------------------------------------------------

class GpBasicPrims2d : public FwNonCopyable
{
public:

	// Enumerations
	
	enum BufferType
	{
		/// kScratchBuffer - for dynamic primitives updated every frame
		///
		/// Consists of a single scratch buffer. Can be recreated and flushed many times per frame.
		/// Primitives live for one frame only.
		
		/// kDoubleBuffer - for dynamic primitives not updated every frame
		///
		/// Consists of a double buffered static buffer. Can be commited only *once* per frame.
		/// Primitives live until the next write buffer commit and can be flushed many times -
		/// in other words, compute once, draw over many frames.

		kScratchBuffer,
		kDoubleBuffer
	};
	
	
	// Explicit Initialisation and Destruction
	
	void	Initialise(BufferType bufferType, uint maxQuads = 0, uint maxTris = 0, uint maxLines = 0);
	void	Destroy();
	
    
	// Stream Buffer Update
	
	void	GetNewScratchBuffer(uint numQuads, uint numTris, uint numLines);	// Use with kScratchBuffer only
	void	CommitWriteBuffer();                								// Use with kDoubleBuffer only
	
	
	// Wireframe Draw Functions	
	
	void	DrawLine(float x0, float y0, float x1, float y1, u32 colour);
	void	DrawLine(float x0, float y0, float x1, float y1, u32 colour0, u32 colour1);
    void	DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, u32 colour);
    void	DrawRect(float x0, float y0, float x1, float y1, u32 colour);
	void	DrawQuad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, u32 colour);
  
  
	// Filled Draw Functions

    void	DrawFilledTriangle(float x0, float y0, float x1, float y1, float x2, float y2, u32 c0, u32 c1, u32 c2);
    void	DrawFilledRect(float x0, float y0, float x1, float y1, u32 c0, u32 c1, u32 c2, u32 c3);
	void	DrawFilledQuad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, u32 c0, u32 c1, u32 c2, u32 c3);


	// Flush Drawn Primitives
	
	void	Flush(GcContext& context = GcKernel::GetContext());
    

private:

	// Constants
	
	static const uint	kVertexStride = (sizeof(float) * 3) + sizeof(u32);
	static const		GcStreamField	kStreamFields[2];

	
	// Structure Definitions
	
	struct Vertex									///< :NOTE: Must match GcStreamFields vertex format!
	{
		float		m_position[3];					///< Vertex NDC position, (0, 0) top-left, (1, 1) bottom-right
		u32			m_colour;						///< Packed RGBA8 colour.
    };


	// Attributes
	
	BufferType				m_bufferType;				///< GcStreamBuffer buffering scheme
	
	uint					m_maxQuads;					///< Max no. of quads
	uint					m_maxTris;					///< Max no. of triangles
	uint					m_maxLines;					///< Max no. of lines
	
	uint					m_readBufferIdx;			///< Index of GPU read stream buffer
	uint					m_writeBufferIdx;			///< Index of PPU write stream buffer
	
	uint					m_numQuads[2];				///< Current no. of quad primitives
	uint					m_numTris[2];				///< Current no. of triangle primitives
	uint					m_numLines[2];				///< Current no. of line primitives
	
	GcStreamBufferHandle	m_hQuadVertexStream[2];		///< Quad primitives vertex stream
	GcStreamBufferHandle	m_hTriVertexStream[2];		///< Triangle primitives vertex stream
	GcStreamBufferHandle	m_hLineVertexStream[2];		///< Line primitives vertex stream
	
	Vertex*					m_pQuadCurrentVertexBuffer;	///< Current quad vertex buffer base
	Vertex*					m_pTriCurrentVertexBuffer;	///< Current triangle vertex buffer base
	Vertex*					m_pLineCurrentVertexBuffer;	///< Current line vertex buffer base
	
	static GcShaderHandle	ms_hGouraud2dVp;			///< Gouraud 2D vertex shader
	static GcShaderHandle	ms_hGouraud2dFp;			///< Gouraud 2D fragment shader
	
	static uint				ms_positionResourceIndex;	///< Resource index for the position vertex attribute
	static uint				ms_colourResourceIndex;		///< Resource index for the colour vertex attribute
	
	
	// Operations
	
	void	SetLineVertices(Vertex* pVertex, float x0, float y0, float x1, float y1, u32 colour0, u32 colour1);
	void 	SetVertexAttributePointers(GcContext& context, void* pVertexBuffer);
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief	Set line vertex attributes.			
**/
//--------------------------------------------------------------------------------------------------

inline void GpBasicPrims2d::SetLineVertices(Vertex* pVertex, float x0, float y0, float x1, float y1, u32 colour0, u32 colour1)
{
	pVertex[0].m_position[0] = x0;
	pVertex[0].m_position[1] = y0;
	pVertex[0].m_position[2] = -1.0f;
	pVertex[0].m_colour = colour0;
	
	pVertex[1].m_position[0] = x1;
	pVertex[1].m_position[1] = y1;
	pVertex[1].m_position[2] = -1.0f;
	pVertex[1].m_colour = colour1;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Draw a 2D line (one colour for the entire line)			
**/
//--------------------------------------------------------------------------------------------------

inline void GpBasicPrims2d::DrawLine(float x0, float y0, float x1, float y1, u32 colour)
{
	FW_ASSERT(m_numLines[m_writeBufferIdx] < m_maxLines); 
	
	Vertex*	pVertex = m_pLineCurrentVertexBuffer + (m_numLines[m_writeBufferIdx] * 2);
	SetLineVertices(pVertex, x0, y0, x1, y1,  colour, colour);
	m_numLines[m_writeBufferIdx] += 1;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief	Draw a 2D line (one colour per vertex)
**/
//--------------------------------------------------------------------------------------------------

inline void GpBasicPrims2d::DrawLine(float x0, float y0, float x1, float y1, u32 colour0, u32 colour1)
{
	FW_ASSERT(m_numLines[m_writeBufferIdx] < m_maxLines); 
	
	Vertex*	pVertex = m_pLineCurrentVertexBuffer + (m_numLines[m_writeBufferIdx] * 2);
	SetLineVertices(pVertex, x0, y0, x1, y1,  colour0, colour1);
	m_numLines[m_writeBufferIdx] += 1;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_BASIC_PRIMS_2D_H
