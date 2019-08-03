/***************************************************************************************************
*
*	$Header:: /game/debugprimitives.h 10    13/08/03 10:39 Simonb                                  $
*
*	Renders debug primitives. 
*
*	CHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#ifndef _DEBUGPRIMITIVE_H
#define _DEBUGPRIMITIVE_H

#include "gfx/shader.h"
#include "gfx/vertexdeclaration.h"

// forward declarations
class CCamera;
class CGraph;

//! The primitive type (part of the iFlags field).	
enum DEBUG_PRIMITIVE_TYPE
{
	DEBUG_PRIMITIVE_SPHERE			= 0x00000000,	//!< Renders a sphere.
	DEBUG_PRIMITIVE_CUBE			= 0x00000001,	//!< Renders a cube.
	DEBUG_PRIMITIVE_CAPSULE			= 0x00000002,	//!< Renders a capsule.
	DEBUG_PRIMITIVE_LINE			= 0x00000003,	//!< Renders a line segment.
	DEBUG_PRIMITIVE_POINT			= 0x00000004,	//!< Renders a point.
	DEBUG_PRIMITIVE_DIRECTEDLINE	= 0x00000005,	//!< Renders a directed (animated) line.
	DEBUG_PRIMITIVE_GRAPH			= 0x00000006,	//!< Renders a graph using a callback.
	DEBUG_PRIMITIVE_USER			= 0x000000ff,	//!< Renders from a user-supplied set of vertices.
	
	DEBUG_PRIMITIVE_TRIANGLELIST	= 0x00000100,	//!< The user-supplied primitive is a triangle list (the default).
	
	DEBUG_PRIMITIVE_WIREFRAME		= 0x00001000,	//!< Renders in wireframe.
	DEBUG_PRIMITIVE_NOCULLING		= 0x00002000,	//!< Disable back-face culling for this primitive.
	DEBUG_PRIMITIVE_NOZCOMPARE		= 0x00004000,	//!< Disables z-testing of the primitives.
	
	// these two flags mean the same thing as far as this system is concerned.
	// there is only a distinction within the visual debugger itself
	// they both use a screen space transform
	DEBUG_PRIMITIVE_VIEWPORTSPACE	= 0x00008000,
	DEBUG_PRIMITIVE_DISPLAYSPACE	= 0x00010000,
	DEBUG_PRIMITIVE_SCREENSPACE		= DEBUG_PRIMITIVE_VIEWPORTSPACE | DEBUG_PRIMITIVE_DISPLAYSPACE,
};

/***************************************************************************************************
*
*	CLASS			CDebugPrimitives
*
*	DESCRIPTION		Caches and renders debug primitives.
*
***************************************************************************************************/

//! Caches and renders debug primitives.
class DebugPrimitives
{
public:
	//! Creates a new debug primitive handler of the given maximum length.
    explicit DebugPrimitives(int iBufferLength);


	//! Queues a sphere to be rendered.
	void RenderSphere(CMatrix const& obLocalTransform, uint32_t dwColour, int iFlags);

	//! Queues a cube to be rendered.
	void RenderCube(CMatrix const& obLocalTransform, uint32_t dwColour, int iFlags);

	//! Queues a capsule to be rendered.
	void RenderCapsule(CMatrix const& obLocalTransform, float fLocalLength, uint32_t dwColour, int iFlags);

	//! Queues a line to be rendered.
	void RenderLine(CPoint const& obWorldStart, CPoint const& obWorldEnd, uint32_t dwColour, int iFlags);

	//! Queues an arc to be rendered
	void RenderArc( CMatrix const& obLocalTransform, float fRadius, float fSweep, uint32_t dwColour, int iFlags );

	//! Queues a point to be rendered.
	/*! The point size /c fPointSize is assumed to be in pixels.
	*/
	void RenderPoint(CPoint const& obWorldPos, float fPointSize, uint32_t dwColour, int iFlags);

	//! Queues a directed line to be rendered.
	/*! \param fSpeed	This is how many times the gap will travel along the line per second.
	*/
	void RenderDirectedLine(CPoint const& obWorldStart, CPoint const& obWorldEnd, float fSpeed, uint32_t dwColour, int iFlags);

	//! Queues a user-defined primitive to be rendered.
	void RenderPrimitive(const CPoint* pobVertices, int iNumVertices, CMatrix const& obLocalTransform, uint32_t dwColour, int iFlags);

	//!	Queues a graph to be rendered.
	void RenderGraph(const CGraph* pobGraph, CPoint const& obTopLeft, CPoint const& obBottomRight, int iFlags = 0);

	//! Renders the debug queue
	void Draw(const CCamera* pobCamera, CDirection const& obScale, CDirection const& obOffset);

	//! same as above, but empties the queue at the same time
	void Flush(const CCamera* pobCamera, CDirection const& obScale, CDirection const& obOffset);
	
	//! Reset the state of the draw queue
	void Reset() { m_iNumPrimitives = 0; }

private:
	//! The internal primitive queue element.
	struct PRIMITIVE
	{
		union NUMVERTICES_OR_FLOAT
		{
			int iNumVertices;			//!< The number of user-defined vertices.
			float fFloatData;			//!< A floating point tag.
		};

		CMatrix obLocalTransform;		//!< The local transformation for this primitive.
		int iType;						//!< The primitive type and associated flags from DEBUG_PRIMITIVE_TYPE.
		uint32_t dwColour;					//!< The colour including alpha.
		const CPoint* pobVertices;		//!< A pointer to user-defined vertices.
		NUMVERTICES_OR_FLOAT uData;		//!< The user-defined vertex count, or a floating point value.
	};

	//! Helper for drawing arcs
	void RenderHalfArc( CMatrix const& obLocalTransform, float fRadius, float fHalfSweep, uint32_t dwColour, int iFlags, bool bPositiveX );

	//! Class for sorting primitives before a flush.
	class CPrimitiveSorter
	{
	public:
		explicit CPrimitiveSorter(CDirection const& obViewDirection) : m_obViewDirection(obViewDirection) {}

		bool operator()(const PRIMITIVE* pstLeft, const PRIMITIVE* pstRight) const;

	private:
		CDirection const& m_obViewDirection; //!< A reference since the STL parameter passing is retarded (be careful with stack usage here).
	};

	//! Information for the optimised built-in primitives.
	struct STANDARD_PRIMITIVE
	{
		int iVertexOffset;
		int iNumVertices;
		int iIndexOffset;
		int iNumIndices;
	};

	DebugShader m_obVertexShader;		//!< The vertex shader used for rendering.
	DebugShader m_obGraphVertexShader;	//!< Damned graph shader.
	DebugShader m_obPixelShader;		//!< The pixel shader used for rendering.

	VBHandle m_pobPositions;			//!< The standard debug vertex positions.
	IBHandle m_pobIndices;				//!< The standard debug indices.

	CVertexDeclaration m_pobVertexDeclaration;		//!< The vertex declaration used for rendering.
	CVertexDeclaration m_pobGraphVertexDeclaration;	//!< The vertex declaration used for graph rendering.

	CScopedArray<STANDARD_PRIMITIVE> m_astStandardPrimitives;	//!< How to render each standard debug mesh. 

	CScopedArray<PRIMITIVE> m_aobPrimitives;	//!< The primitives queue.
	CScopedArray<const PRIMITIVE*> m_aobQueue;	//!< The sort queue used during rendering.
	int m_iMaxNumPrimitives;					//!< The maximum number of primitives queueable.
	int m_iNumPrimitives;						//!< The number of primitives queued.
};

#endif // ndef _DEBUGPRIMITIVE_H
