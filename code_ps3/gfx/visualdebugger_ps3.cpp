//--------------------------------------------------
//!
//!	\file gfx/visualdebugger_ps3.cpp
//! This implements the PS3 specific version of the
//! visual debugger interface. This goes through the
//! ATG debug libraries to save us some work...
//!
//--------------------------------------------------
#include "core/visualdebugger.h"
#include "core/gfxmem.h"

#ifndef _GOLD_MASTER

#include "gfx/renderersettings.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/camera.h"
#include "gfx/graphing.h"
#include "gfx/screenline_ps3.h"
#include "gfx/display.h"
#include "anim/transform.h"			// im thinking this should be in core dean...

#include "Gp/GpBasicText/GpBasicText.h"
#include "Gp/GpBasicPrims/GpBasicPrims.h"

// unfortunately the scan out on our dells looses 10% of the frame on the LHS. This
// define silently shifts and squashes VP text and lines to the right in component mode
// and does something equally bad in VGA mode

class PS3VisualDebugger : public VisualDebuggerInterface
{
public:

	PS3VisualDebugger(int iDebugDisplayWidth, int iDebugDisplayHeight) :
		m_screenLines( _R( iDebugDisplayWidth ), _R( iDebugDisplayHeight ), MAX_PRIMS * 2 ), m_iCharsThisFrame(0)
	{
		// store the old debugger for replacement when we have finished.
		m_pPreviousDebugger = g_VisualDebug;
		g_VisualDebug = this; // now we are the visual debugger

		m_pDefaultCamera = 0;

		GpBasicText::Initialise( MAX_CHARS );
		GpBasicPrims::Initialise( Mem::GpMem::GP_PRIMS_PB_SIZE );

		m_iDebugDisplayWidth = iDebugDisplayWidth;
		m_iDebugDisplayHeight = iDebugDisplayHeight;

		m_fRCPDispWidth = 1.0f / _R( m_iDebugDisplayWidth );
		m_fRCPDispHeight = 1.0f / _R( m_iDebugDisplayHeight );
	}

	~PS3VisualDebugger()
	{
		g_VisualDebug = m_pPreviousDebugger;
		GpBasicText::Shutdown();
		GpBasicPrims::Shutdown();
	}

	//! This conversion is really just an unpack on PS3, as NT colour matches Gc
	//! colours on PS3, and DX colours on PC.
	//! Format		LittleEndian				BigEndian
	//!	DX			(A<<24)|(R<<16)|(G<<8)|B	(B<<24)|(G<<16)|(R<<8)|A
	//!	NT			(A<<24)|(R<<16)|(G<<8)|B	(R<<24)|(G<<16)|(B<<8)|A
	//! Gc			(A<<24)|(B<<16)|(G<<8)|R	(R<<24)|(G<<16)|(B<<8)|A

	static inline GcColour	ConvertColour( uint32_t iNTColour )
	{
		float r,g,b,a;
		NTCOLOUR_EXTRACT_FLOATS( iNTColour, r, g, b, a );
		return GcColour( r, g, b, a );
	}

	//! render a sphere
	virtual void RenderSphere(CMatrix const& matrix, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			SetATGMatrixStack( matrix );
			GpBasicPrims::DrawSphere( FwPoint( 0.0f, 0.0f, 0.0f ), 1.0f, ConvertColour(iColour) );
		}
	};

	//! render a cube
	virtual void RenderCube(CMatrix const& matrix, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			SetATGMatrixStack( matrix );
			GpBasicPrims::DrawCuboid( 1.0f, 1.0f, 1.0f, ConvertColour(iColour) );
		}
	}

	virtual void RenderCube(CMatrix const& matrix, uint32_t iColour, float size, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			SetATGMatrixStack( matrix );
			GpBasicPrims::DrawCuboid( size, size, size, ConvertColour(iColour) );
		}
	}


	// render a capsule. The capsule length lies along the Z axis
	virtual void RenderCapsule(CMatrix const& matrix, float fLength, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			SetATGMatrixStack( matrix );
			GpBasicPrims::DrawCapsule( 1.0f, fLength * 0.5f, ConvertColour(iColour) );
		}
	}

	//  Renders a section of a circle - centred around the z axis of the given matrix
	//	and drawn in the x/z plane.
	virtual void RenderArc( CMatrix const& matrix, float fRadius, float fSweep, uint32_t iColour, int iFlags = 0)
	{
		// Draw each side using our helper
		RenderHalfArc( matrix, fRadius, fSweep / 2.0f, iColour, iFlags, true );
		RenderHalfArc( matrix, fRadius, fSweep / 2.0f, iColour, iFlags, false );
	};

	//! Queues a line to be rendered.
	virtual void RenderLine(CPoint const& start, CPoint const& end, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			if (iFlags & (DPF_VIEWPORTSPACE | DPF_DISPLAYSPACE))
			{
				m_screenLines.AddLine( start.X(), start.Y(), end.X(), end.Y(), iColour );
			}
			else
			{
				FwPoint point0( *(FwPoint*)&start );
				FwPoint point1( *(FwPoint*)&end );

				SetATGMatrixStack( CVecMath::GetIdentity() );
				GpBasicPrims::DrawLine( point0, point1, ConvertColour( iColour ) );
			}
		}
	};

	//! Queues a point to be rendered.
	/*! The point size /c fPointSize is assumed to be in pixels.
	*/
	virtual void RenderPoint(CPoint const& position, float fSize, uint32_t iColour, int iFlags = 0) {}

	//! Queues a directed line to be rendered.
	/*! \param fSpeed	This is how many times the gap will travel along the line per second.
	*/
	virtual void RenderDirectedLine(CPoint const& start, CPoint const& end, float fSpeed, uint32_t iColour, int iFlags = 0)
	{
		ntAssert_p( !(iFlags & DPF_VIEWPORTSPACE), ("Screen space directed lines unsupported on PS3") );

		// OZZ: disabled as GpBasicPrims::DrawDirectedLine expects FpTimer singleton to be initialised
		ntAssert_p( false, ("RenderDirectedLine is not supported on PS3\n") );
		if( false && CRendererSettings::bEnableDebugPrimitives ) 
		{
			FwPoint point0( *(FwPoint*)&start );
			FwPoint point1( *(FwPoint*)&end );

			SetATGMatrixStack( CVecMath::GetIdentity() );
			GpBasicPrims::DrawDirectedLine( point0, point1, ConvertColour( iColour ) );
		}
	}

	// render a user defined primitive. this MUST be a triangle list
	virtual void RenderPrimitive(const CPoint* pVerts, int iNumVerts, CMatrix const& matrix, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
			ntPrintf( "WARNING! Arbitary debug prims not supported on PS3 yet!\n" );
	}

	//! Screen print at a particular location (single frame only)
	virtual void Printf2D( float fX, float fY, uint32_t iColour, int iFlags, const char* pcTxt, ... )
	{
		char pcBuffer[ MAX_CHARS ];

		// Format the text.
		va_list	stArgList;
		va_start(stArgList, pcTxt);
		vsnprintf(pcBuffer, MAX_CHARS-1, pcTxt, stArgList);
		va_end(stArgList);

		// convert to normalised coords
		fX *= m_fRCPDispWidth;
		fY *= m_fRCPDispHeight;
		
		char* pSlashN = strstr( pcBuffer, "\n" );
		while (pSlashN)
		{
			*pSlashN = ' ';
			pSlashN = strstr( pcBuffer, "\n" );
		};

		m_iCharsThisFrame += strlen( pcBuffer );

		if( CRendererSettings::bEnableDebugPrimitives && m_iCharsThisFrame < MAX_CHARS) 
			GpBasicText::Draw2D( pcBuffer, fX, fY, ConvertColour( iColour ), GpBasicText::kFontMonoRegular );
	}

	virtual void Printf3D( CPoint const& worldPos, uint32_t colour, int iFlags, const char* pcTxt, ... )
	{
		CPoint screen;
		if (WorldToScreen(worldPos,screen))
		{
			char pcBuffer[ MAX_CHARS ];
			// Format the text.
			va_list	stArgList;
			va_start(stArgList, pcTxt);
			vsnprintf(pcBuffer, MAX_CHARS-1, pcTxt, stArgList);
			va_end(stArgList);

			Printf2D( screen.X(), screen.Y(), colour, iFlags, pcBuffer, 0 );
		}
	}

	virtual void Printf3D( CPoint const& worldPos, float fXOffset, float fYOffset, uint32_t colour, int iFlags, const char* pcTxt, ... )
	{
		CPoint screen;
		if (WorldToScreen(worldPos,screen))
		{
			char pcBuffer[ MAX_CHARS ];
			// Format the text.
			va_list	stArgList;
			va_start(stArgList, pcTxt);
			vsnprintf(pcBuffer, MAX_CHARS-1, pcTxt, stArgList);
			va_end(stArgList);

			Printf2D( screen.X()+fXOffset, screen.Y()+fYOffset, colour, iFlags, pcBuffer, 0 );
		}
	}

	struct GraphBuffer
	{
		const CGraph*	pGraph;
		CMatrix			localTransform;
	};

	// render a screen space graph
	virtual void RenderGraph(const CGraph* pGraph, CPoint const& topLeft, CPoint const& bottomRight, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{	
			GraphBuffer* pNewEntry = NT_NEW_CHUNK( Mem::MC_GFX ) GraphBuffer;
			pNewEntry->pGraph = pGraph;

			// compute the line transform
			pNewEntry->localTransform.SetIdentity();
			pNewEntry->localTransform[0][0] = bottomRight.X() - topLeft.X();
			pNewEntry->localTransform[1][1] = topLeft.Y() - bottomRight.Y();
			pNewEntry->localTransform[2][2] = 0.0f;
			pNewEntry->localTransform.SetTranslation(CPoint(topLeft.X(), bottomRight.Y(), 0.0f));

			m_graphEntries.push_front( pNewEntry );
		}
	}
	
	//! Get default display width (useful for prettying up the debug stuff)
	virtual float GetDebugDisplayWidth() { return _R(m_iDebugDisplayWidth); }

	//! Get default display height (useful for prettying up the debug stuff)
	virtual float GetDebugDisplayHeight() { return _R(m_iDebugDisplayHeight); }

	virtual void Draw3D()
	{
		// now draw atg debug prims
		GpBasicPrims::Flush();
	};

	virtual void Draw2D(){};

	//! Flush any buffering
	virtual void Reset()
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

		// draw graphs first, incase they use debug lines in render
		float fWidth = GetDebugDisplayWidth();
		float fHeight = GetDebugDisplayHeight();

		// compute the viewport-space transform
		CMatrix viewTrans(CONSTRUCT_IDENTITY);

		viewTrans[0][0] = 2.0f/fWidth;
		viewTrans[3][0] = -1.0f;
		viewTrans[1][1] = -2.0f/fHeight;
		viewTrans[3][1] = 1.0f;

		// now draw atg debug text
		GpBasicText::Flush();
		m_iCharsThisFrame = 0;

		// We have to draw our graphs here now.
		while (!m_graphEntries.empty())
		{
			GraphBuffer* pCurr = m_graphEntries.back();
			pCurr->pGraph->Render( pCurr->localTransform, viewTrans );
			NT_DELETE_CHUNK( Mem::MC_GFX, pCurr );
			m_graphEntries.pop_back();
		}

		// have to set these again as the graph renderer overides render states
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

		// draw our screen space lines
		m_screenLines.FlushPrims();

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_NORMAL );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );
		Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );

		// clear our frame camera
		m_pDefaultCamera = 0;
	}

protected:
	virtual const CCamera* GetDrawParameters( float& fWidth, float& fHeight )
	{
		const CCamera* pResult = 0;
		if (RenderingContext::GetIndex()>=0)
		{
			fWidth = _R( Renderer::Get().m_targetCache.GetPrimaryColourTarget()->GetWidth() );
			fHeight = _R( Renderer::Get().m_targetCache.GetPrimaryColourTarget()->GetHeight() );
			pResult = RenderingContext::Get()->m_pViewCamera;
		}
		else
		{
			// ntPrintf("WARNING! Calling WorldToScreen outside of a valid render context!\n");
			fWidth = GetDebugDisplayWidth();
			fHeight = GetDebugDisplayHeight();
			pResult = m_pDefaultCamera;
		}
		return pResult;
	}

	void	SetATGMatrixStack( const CMatrix& world )
	{
		float fWidth, fHeight;
		const CCamera* pCamera = GetDrawParameters(fWidth,fHeight);
		ntAssert_p( pCamera, ("Must have a valid camera set here") );

		CMatrix	proj;
		pCamera->GetProjection( fWidth/fHeight, proj );

		CMatrix	view = pCamera->GetViewTransform()->GetWorldMatrix().GetAffineInverse();

		// use some cheeky casts till we have a maths conversion lib
		FwMatrix44	worldMat( *(FwMatrix44*)&world );
		FwMatrix44	viewMat( *(FwMatrix44*)&view );
		FwMatrix44	projMat( *(FwMatrix44*)&proj );

		GpBasicPrims::SetWorldMatrix(worldMat);			
		GpBasicPrims::SetViewMatrix(viewMat);			
		GpBasicPrims::SetProjectionMatrix(projMat);
	}

private:
	// shamelessly copied from debugprimitives_pc, we should probably promote this to 
	// the base VisualDebuggerInterface class
	void	RenderHalfArc(	CMatrix const&	matrix, 
							float			fRadius, 
							float			fHalfSweep, 
							uint32_t		iColour, 
							int				iFlags, 
							bool			bPositiveX )
	{

		// Points on a unit radius circle - every 10 degrees
		static const int iPointsPerHalf = 18;
		static const float fSweepPerLine = 10.0f;

		// The points we will need for z point values
		static const float afZCirclePoints[iPointsPerHalf + 1] = 
		{	
			1.0f,	0.984808f,	0.939693f,	0.866025f,	0.766044f,	0.642788f,	0.5f,		0.342020f,	0.173648f,	0.0f,
					-0.173648f,	-0.342020f,	-0.5f,		-0.642788f,	-0.766044f,	-0.866025f,	-0.939693f,	-0.984808f,	-1.0f
		};

		// The points we will need for x point values
		static const float afXCirclePoints[iPointsPerHalf + 1] = 
		{	
			0.0f,	0.173648f,	0.342020f,	0.5f,		0.642788f,	0.766044f,	0.866025f,	0.939693f,	0.984808f,	1.0f,
					0.984808f,	0.939693f,	0.866025f,	0.766044f,	0.642788f,	0.5f,		0.342020f,	0.173648f,	0.0f
		};

		// Set up a multiplier for the x values
		float fXMultiplier = ( bPositiveX ) ? 1.0f : -1.0f;

		// Set up the 'starting' start point
		CPoint start( 0.0f, 0.0f, fRadius );

		// Loop through and draw the lines about each side	
		float fDegrees = 0.0f;
		for ( int iLine = 0; iLine < iPointsPerHalf; ++iLine )
		{
			// Add on the degrees for this line
			fDegrees += fSweepPerLine;

			// If we have gone past our sweep then we need to calculate a point
			if ( fDegrees > ( fHalfSweep * RAD_TO_DEG_VALUE ) )
			{
				// Calculate the end point in local space
				CPoint end(	fsinf( fHalfSweep ) * fRadius * fXMultiplier, 
							0.0f, 
							fcosf( fHalfSweep ) * fRadius );

				// Transform to world space in the renderline call
				RenderLine( ( start * matrix ), ( end * matrix ), iColour, iFlags );
			}

			// ...otherwise we can just reference points from our array
			else
			{
				// Calculate the end point in local space
				CPoint end(	afXCirclePoints[iLine + 1] * fRadius * fXMultiplier, 
							0.0f, 
							afZCirclePoints[iLine + 1] * fRadius );

				// Transform to world space in the renderline call
				RenderLine( ( start * matrix ), ( end * matrix ), iColour, iFlags );

				// Save the end point for the next start point
				start = end;
			}
		}
	}

	VisualDebuggerInterface*	m_pPreviousDebugger;
	int							m_iDebugDisplayWidth, m_iDebugDisplayHeight;
	float						m_fRCPDispWidth, m_fRCPDispHeight;
	typedef ntstd::List<GraphBuffer*, Mem::MC_GFX> GraphBufferList;
	GraphBufferList				m_graphEntries;
	ScreenLines					m_screenLines;
	int							m_iCharsThisFrame;
};

void RegisterPS3VisualDebugger()
{
	NT_NEW_CHUNK( Mem::MC_GFX ) PS3VisualDebugger(	(int)DisplayManager::Get().GetInternalWidth(),
													(int)DisplayManager::Get().GetInternalHeight() );
}

#endif

