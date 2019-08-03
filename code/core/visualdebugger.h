//----------------------------------------------------------------
//!
//!	\file core/visualdebugger.h
//! This is a core visual debugger, by default most
//! are unimplemneted and the text is dumped to the log once
//! the renderer is running its will hook in and display
//! any of this stuff on screen.
//! Its a core function so that any library can dispaly stuff
//! on screen without coupling to renderer
//!
//----------------------------------------------------------------

#if !defined(CORE_VISUALDEBUGGER_H)
#define	CORE_VISUALDEBUGGER_H

// TODO Deano err where does graph belong, forward decl for the moment
class CGraph;
class CCamera;
class CHierarchy;
class CPoint;

//! The primitive type (part of the iFlags field).	
enum DEBUG_PRIMITIVE_FLAGS
{
	// lower 8 bits are reserved
	DPF_TRIANGLELIST	= 0x00000100,	//!< The user-supplied primitive is a triangle list (the default).
	DPF_WIREFRAME		= 0x00001000,	//!< Renders in wireframe.
	DPF_NOCULLING		= 0x00002000,	//!< Disable back-face culling for this primitive.
	DPF_NOZCOMPARE		= 0x00004000,	//!< Disables z-testing of the primitives.
	DPF_VIEWPORTSPACE	= 0x00008000,	//!< Uses the space of the current view / render target
	DPF_DISPLAYSPACE	= 0x00010000	//!< Uses the space of the final back buffer
};

enum DEBUG_TEXT_FLAGS
{
	DTF_ALIGN_LEFT		= 0x00000000,	//!< Left-aligns the text.
	DTF_ALIGN_HCENTRE	= 0x00000001,	//!< Centres the text horizontally.
	DTF_ALIGN_RIGHT		= 0x00000002,	//!< Right-aligns the text.
	DTF_ALIGN_TOP		= 0x00000000,	//!< Uses the y co-ordinate as the top of the text.
	DTF_ALIGN_VCENTRE	= 0x00000004,	//!< Centres the text vertically.
	DTF_ALIGN_BOTTOM	= 0x00000008	//!< Uses the y co-ordinate as the bottom of the text.
};

namespace DebugColour
{
	inline uint32_t white( uint8_t i = 255, uint8_t a = 255 )	{ return NTCOLOUR_ARGB(a,i,i,i); }
	inline uint32_t grey( uint8_t i = 127, uint8_t a = 255 )	{ return NTCOLOUR_ARGB(a,i,i,i); }
	inline uint32_t black( uint8_t = 0, uint8_t a = 255 )		{ return NTCOLOUR_ARGB(a,0,0,0); }
	inline uint32_t red( uint8_t i = 255, uint8_t a = 255 )		{ return NTCOLOUR_ARGB(a,i,0,0); }
	inline uint32_t green( uint8_t i = 255, uint8_t a = 255 )	{ return NTCOLOUR_ARGB(a,0,i,0); }
	inline uint32_t blue( uint8_t i = 255, uint8_t a = 255 )	{ return NTCOLOUR_ARGB(a,0,0,i); }
	inline uint32_t purple( uint8_t i = 255, uint8_t a = 255 )	{ return NTCOLOUR_ARGB(a,i,0,i); }
	inline uint32_t cyan( uint8_t i = 255, uint8_t a = 255 )	{ return NTCOLOUR_ARGB(a,0,i,i); }
	inline uint32_t yellow( uint8_t i = 255, uint8_t a = 255 )	{ return NTCOLOUR_ARGB(a,i,i,0); }
};

#define DC_WHITE	DebugColour::white()
#define DC_GREY		DebugColour::grey()
#define	DC_BLACK	DebugColour::black()
#define	DC_RED		DebugColour::red()
#define	DC_GREEN	DebugColour::green()
#define	DC_BLUE		DebugColour::blue()
#define	DC_PURPLE	DebugColour::purple()
#define	DC_CYAN		DebugColour::cyan()
#define	DC_YELLOW	DebugColour::yellow()

class VisualDebuggerInterface
{
public:
	virtual ~VisualDebuggerInterface(){};

	const static int MAX_CHARS = 4096; 
	const static int MAX_PRIMS = 65536; 

	// methods must be provided by platform implementations
	virtual void RenderSphere(CMatrix const& matrix, uint32_t iColour, int iFlags = 0) = 0;
	virtual void RenderCube(CMatrix const& matrix, uint32_t iColour, int iFlags = 0) = 0;
	virtual void RenderCapsule(CMatrix const& matrix, float fLength, uint32_t iColour, int iFlags = 0) = 0;
	virtual void RenderArc(CMatrix const& matrix, float fRadius, float fSweep, uint32_t iColour, int iFlags = 0) = 0;
	virtual void RenderLine(CPoint const& start, CPoint const& end, uint32_t iColour, int iFlags = 0) = 0;
	virtual void RenderPoint(CPoint const& position, float fSize, uint32_t iColour, int iFlags = 0) = 0;
	virtual void RenderDirectedLine(CPoint const& start, CPoint const& end, float fSpeed, uint32_t iColour, int iFlags = 0) = 0;
	virtual void RenderPrimitive(const CPoint* pVerts, int iNumVerts, CMatrix const& matrix, uint32_t iColour, int iFlags = 0) = 0;
	virtual void RenderGraph(const CGraph* pobGraph, CPoint const& obTopLeft, CPoint const& obBottomRight, int iFlags = 0) = 0;
	virtual void Printf2D( float fX, float fY, uint32_t colour, int iFlags, const char* pcTxt, ... ) = 0;
	virtual void Printf3D( CPoint const& worldPos, uint32_t colour, int iFlags, const char* pcTxt, ... ) = 0;	
	virtual void Printf3D( CPoint const& worldPos, float fXOffset, float fYOffset, uint32_t colour, int iFlags, const char* pcTxt, ... ) = 0;	
	virtual void Draw3D() = 0;
	virtual void Draw2D() = 0;
	virtual void Reset() = 0;

#ifdef PLATFORM_PS3
	virtual void RenderCube(CMatrix const& matrix, uint32_t iColour, float size, int iFlags = 0) = 0;
#endif

	virtual float GetDebugDisplayWidth() = 0;
	virtual float GetDebugDisplayHeight() = 0;

	//! Queues an axis to be rendered
	void RenderHierarchy(const CHierarchy& h, float fAxisSize, const CDirection& offset, int iFlags = 0);
	
	//! Queues an axis to be rendered
	inline void RenderAxis(const CMatrix& affine, float fSize, int iFlags = 0)
	{
		const CPoint& o = affine.GetTranslation();
		RenderLine(o,o+affine.GetXAxis()*fSize,NTCOLOUR_ARGB(255,255,0,0),iFlags);
		RenderLine(o,o+affine.GetYAxis()*fSize,NTCOLOUR_ARGB(255,0,255,0),iFlags);
		RenderLine(o,o+affine.GetZAxis()*fSize,NTCOLOUR_ARGB(255,0,0,255),iFlags);
	}

	//! Queues a sphere to be rendered.
	inline void RenderSphere(CQuat const& orientation, CPoint const& worldPos, float fRadius, uint32_t iColour, int iFlags = 0)
	{
		CMatrix matrix(orientation, worldPos);
		for(int iRow = 0; iRow < 3; ++iRow)
			matrix[iRow] *= fRadius;

		RenderSphere(matrix, iColour, iFlags);
	}

	//! Queues an axis aligned bounding box
	inline void RenderAABB(CPoint const& min, CPoint const& max, uint32_t iColour, int iFlags = 0)
	{
		// compute the local space to world space transform
		CMatrix matrix(CONSTRUCT_IDENTITY);
		CPoint scale = (max - min)*0.5f;
		for(int iRow = 0; iRow < 3; ++iRow)
			matrix[iRow][iRow] = scale[iRow];

		CPoint offset = (max + min)*0.5f;
		matrix.SetTranslation(offset);

		// render a unit cube with this transform
		RenderCube(matrix, iColour, iFlags);
	}

	//! Queues an OBB to be rendered.
	inline void RenderOBB(CQuat const& orientation, CPoint const& position, CDirection const& halfExtents, uint32_t iColour, int iFlags = 0)
	{
		RenderOBB( CMatrix(orientation,position), halfExtents, iColour, iFlags );
	}

	inline void RenderOBB(CMatrix const& rotPos, CDirection const& halfExtents, uint32_t iColour, int iFlags = 0)
	{
		// compute the local space to world space transform
		CMatrix matrix = rotPos;
		for(int iRow = 0; iRow < 3; ++iRow)
			matrix[iRow] *= halfExtents[iRow];

		// render a unit cube with this transform
		RenderCube(matrix, iColour, iFlags);
	}

	//! Queues a capsule to be rendered.
	inline void RenderCapsule(CQuat const& orientation, CPoint const& position, float fCapRadius, float fLength, uint32_t iColour, int iFlags = 0)
	{
		// build the local transform
		CMatrix matrix(orientation, position);
		for(int iRow = 0; iRow < 3; ++iRow)
			matrix[iRow] *= fCapRadius;

		RenderCapsule( matrix, fLength, iColour, iFlags );
	}

	//! Convert world space to screen space, return false if point is behind screen
	bool WorldToScreen( const CPoint& world, CPoint& screen );

	//! Register the defualt camera to use when not within a render context
	void SetDefaultCamera( CCamera* pCam ) { m_pDefaultCamera = pCam; }

protected:
	virtual const CCamera* GetDrawParameters( float& fWidth, float& fHeight ) = 0;
	CCamera* m_pDefaultCamera;
};

#ifdef _GOLD_MASTER
#define g_VisualDebug	DO_NOT_USE_DEBUG_PRIMS_ON_MASTER
#else
extern VisualDebuggerInterface* g_VisualDebug; 
#endif

#endif // end CORE_VISUALDEBUGGER_H
