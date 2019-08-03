//--------------------------------------------------
//!
//!	\file gfx/visualdebugger_pc.cpp
//! This implements the PC specific version of the
//! visual debugger interface. 
//!
//--------------------------------------------------
#include "core/visualdebugger.h"
#include "gfx/debugtext_pc.h"
#include "gfx/debugprimitives_pc.h"
#include "gfx/renderersettings.h"
#include "gfx/rendercontext.h"
#include "gfx/renderer.h"

struct Delayed3DText
{
	char		aBuffer[ VisualDebuggerInterface::MAX_CHARS ];
	CPoint		position;
	uint32_t	colour;
	int			iFlags;
	int			iLen;
	float		fX, fY;
};

class PCVisualDebugger : public VisualDebuggerInterface
{
public:

	PCVisualDebugger(int iDebugDisplayWidth, int iDebugDisplayHeight)
	{
		m_pDefaultCamera = 0;

		// allocate our internal objects
		m_pDebugText2D = NT_NEW_CHUNK(Mem::MC_GFX) DebugText( MAX_CHARS );
		m_pDebugPrims2D = NT_NEW_CHUNK(Mem::MC_GFX) DebugPrimitives( MAX_PRIMS>>1 );
		
		m_pDebugText3D = NT_NEW_CHUNK(Mem::MC_GFX) DebugText( MAX_CHARS );
		m_pDebugPrims3D = NT_NEW_CHUNK(Mem::MC_GFX) DebugPrimitives( MAX_PRIMS );

		// store the old debugger for replacement when we have finished.
		m_pPreviousDebugger = g_VisualDebug;
		g_VisualDebug = this;  // now we are the visual debugger

		m_iDebugDisplayWidth = iDebugDisplayWidth;
		m_iDebugDisplayHeight = iDebugDisplayHeight;
	}

	~PCVisualDebugger()
	{
		// delete any other bits left over
		Reset();

		// restore the visual debugger
		g_VisualDebug = m_pPreviousDebugger;

		// free up our memory
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_pDebugPrims2D );
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_pDebugPrims3D );
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_pDebugText2D );
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_pDebugText3D );
	}

	//! render a sphere
	virtual void RenderSphere(CMatrix const& matrix, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			if (iFlags & DPF_DISPLAYSPACE)
				m_pDebugPrims2D->RenderSphere(matrix, iColour, iFlags);
			else
				m_pDebugPrims3D->RenderSphere(matrix, iColour, iFlags);
		}
	}

	//! render a cube
	virtual void RenderCube(CMatrix const& matrix, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			if (iFlags & DPF_DISPLAYSPACE)
				m_pDebugPrims2D->RenderCube(matrix, iColour, iFlags);
			else
				m_pDebugPrims3D->RenderCube(matrix, iColour, iFlags);
		}
	}

	// render a capsule. The capsule length lies along the Z axis
	virtual void RenderCapsule(CMatrix const& matrix, float fLength, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			if (iFlags & DPF_DISPLAYSPACE)
				m_pDebugPrims2D->RenderCapsule( matrix, fLength, iColour, iFlags );
			else
				m_pDebugPrims3D->RenderCapsule( matrix, fLength, iColour, iFlags );
		}
	}

	//  Renders a section of a circle - centred around the z axis of the given matrix
	//	and drawn in the x/z plane.
	virtual void RenderArc( CMatrix const& matrix, float fRadius, float fSweep, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			if (iFlags & DPF_DISPLAYSPACE)
				m_pDebugPrims2D->RenderArc( matrix, fRadius, fSweep, iColour, iFlags );
			else
				m_pDebugPrims3D->RenderArc( matrix, fRadius, fSweep, iColour, iFlags );
		}
	}

	// render a world space line
	virtual void RenderLine(CPoint const& start, CPoint const& end, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			if (iFlags & DPF_DISPLAYSPACE)
				m_pDebugPrims2D->RenderLine(start, end, iColour, iFlags);
			else
				m_pDebugPrims3D->RenderLine(start, end, iColour, iFlags);
		}
	}
	
	// render a world space point
	virtual void RenderPoint(CPoint const& position, float fSize, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			if (iFlags & DPF_DISPLAYSPACE)
				m_pDebugPrims2D->RenderPoint(position, fSize, iColour, iFlags);
			else
				m_pDebugPrims3D->RenderPoint(position, fSize, iColour, iFlags);
		}
	}

	//! Queues a directed line to be rendered.
	//! \param fSpeed	This is how many times the gap will travel along the line per second.
	virtual void RenderDirectedLine(CPoint const& start, CPoint const& end, float fSpeed, uint32_t iColour, int iFlags = 0)
	{
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			if (iFlags & DPF_DISPLAYSPACE)
				m_pDebugPrims2D->RenderDirectedLine( start, end, fSpeed, iColour, iFlags );
			else
				m_pDebugPrims3D->RenderDirectedLine( start, end, fSpeed, iColour, iFlags );
		}
	}

	// render a user defined primitive. this MUST be a triangle list
	virtual void RenderPrimitive(const CPoint* pVerts, int iNumVerts, CMatrix const& matrix, uint32_t iColour, int iFlags)
	{
		ntAssert( iFlags & DPF_TRIANGLELIST );
		if( CRendererSettings::bEnableDebugPrimitives ) 
		{
			if (iFlags & DPF_DISPLAYSPACE)
				m_pDebugPrims2D->RenderPrimitive( pVerts, iNumVerts, matrix, iColour, iFlags );
			else
				m_pDebugPrims3D->RenderPrimitive( pVerts, iNumVerts, matrix, iColour, iFlags );
		}
	}

	//! print to the screen at the desired coordinates
	virtual void Printf2D( float fX, float fY, uint32_t colour, int iFlags, const char* pcTxt, ... )
	{
		char pcBuffer[ MAX_CHARS ];
		// Format the text.
		va_list	stArgList;
		va_start(stArgList, pcTxt);
		int iLen = vsnprintf(pcBuffer, MAX_CHARS-1, pcTxt, stArgList);
		va_end(stArgList);

		if( CRendererSettings::bEnableDebugPrimitives ) 
			m_pDebugText2D->Printf( fX, fY, colour, iFlags, pcBuffer, iLen );
	}

	//! delay the print untill Draw3D is called
	virtual void Printf3D( CPoint const& worldPos, uint32_t colour, int iFlags, const char* pcTxt, ... ) 
	{
		Delayed3DText* pNewText = NT_NEW_CHUNK(Mem::MC_GFX) Delayed3DText;

		// Format the text.
		va_list	stArgList;
		va_start(stArgList, pcTxt);
		pNewText->iLen = vsnprintf( pNewText->aBuffer, MAX_CHARS-1, pcTxt, stArgList);
		va_end(stArgList);

		// store the other parameters
		pNewText->position = worldPos;
		pNewText->colour = colour;
		pNewText->iFlags = iFlags;
		pNewText->fX = 0.0f;
		pNewText->fY = 0.0f;

		// add to the list
		m_delayedText.push_back( pNewText );
	}

	//! delay the print untill Draw3D is called
	virtual void Printf3D( CPoint const& worldPos, float fXOffset, float fYOffset, uint32_t colour, int iFlags, const char* pcTxt, ... ) 
	{
		Delayed3DText* pNewText = NT_NEW_CHUNK(Mem::MC_GFX) Delayed3DText;

		// Format the text.
		va_list	stArgList;
		va_start(stArgList, pcTxt);
		pNewText->iLen = vsnprintf( pNewText->aBuffer, MAX_CHARS-1, pcTxt, stArgList);
		va_end(stArgList);

		// store the other parameters
		pNewText->position = worldPos;
		pNewText->colour = colour;
		pNewText->iFlags = iFlags;
		pNewText->fX = fXOffset;
		pNewText->fY = fYOffset;

		// add to the list
		m_delayedText.push_back( pNewText );
	}

	// render a screen space graph
	virtual void RenderGraph(const CGraph* pGraph, CPoint const& topLeft, CPoint const& bottomRight, int iFlags = 0)
	{
		// graphs are always drawn in display space, hence the use of m_pDebugPrims2D
		if( CRendererSettings::bEnableDebugPrimitives ) 
			m_pDebugPrims2D->RenderGraph( pGraph, topLeft, bottomRight, iFlags );
	}

	//! get the width of the display device (back buffer in window mode, screen in fullscreen)
	virtual float GetDebugDisplayWidth() { return _R(m_iDebugDisplayWidth); }

	//! get the height of the display device (back buffer in window mode, screen in fullscreen)
	virtual float GetDebugDisplayHeight() { return _R(m_iDebugDisplayHeight); }

	//! render eveything in our 3D sets. note, this may be called multiple times a frame
	virtual void Draw3D()
	{
		float fWidth,fHeight;
		const CCamera* pCamera = GetDrawParameters(fWidth,fHeight);

		ntError_p( pCamera, ("Must have a camera here") );

		CDirection obScale = CDirection( 2.0f/fWidth, -2.0f/fHeight, 1.0f );
		CDirection obOffset = CDirection(-1.0f, 1.0f, 0.0f);

		// dump our 3D text into the primitve list
		for (	ntstd::List<Delayed3DText*>::iterator it = m_delayedText.begin();
				it != m_delayedText.end(); ++it )
		{
			Delayed3DText& text = *(*it);

			CPoint screen;
			if (WorldToScreen(text.position,screen))
			{
				m_pDebugText3D->Printf(	screen.X() + text.fX,
										screen.Y() + text.fY,
										text.colour,
										text.iFlags,
										text.aBuffer,
										text.iLen );				
			}
		}

		// flush it
		m_pDebugText3D->Flush( obScale, obOffset );

		// draw this list without a flush
		m_pDebugPrims3D->Draw( pCamera, obScale, obOffset );
	}

	//! this draws all our debug primitives.
	virtual void Draw2D()
	{
		float fWidth,fHeight;
		const CCamera* pCamera = GetDrawParameters(fWidth,fHeight);

		CDirection obScale = CDirection( 2.0f/fWidth, -2.0f/fHeight, 1.0f );
		CDirection obOffset = CDirection(-1.0f, 1.0f, 0.0f);

		m_pDebugText2D->Flush( obScale, obOffset );
		
		if (pCamera)
			m_pDebugPrims2D->Flush( pCamera, obScale, obOffset );
	}

	//! signal we've finished drawing for this frame.
	virtual void Reset()
	{
		// only one that hasnt been flushed will be this one:
		m_pDebugPrims3D->Reset();

		// still need to delete our 3D text entries
		while (!m_delayedText.empty())
		{
			NT_DELETE_CHUNK(Mem::MC_GFX,  m_delayedText.back() );
			m_delayedText.pop_back();
		}

		// clear our frame camera
		m_pDefaultCamera = 0;

	}

protected:
	virtual const CCamera* GetDrawParameters( float& fWidth, float& fHeight )
	{
		const CCamera* pResult;
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

private:

	// world space primitives and text, which can be drawn multiple times in differing viewports
	DebugText*					m_pDebugText3D;
	DebugPrimitives*			m_pDebugPrims3D;
	ntstd::List<Delayed3DText*>	m_delayedText;

	// screen space primitives and text, drawn only once on the final buffer
	DebugText*					m_pDebugText2D;
	DebugPrimitives*			m_pDebugPrims2D;

	VisualDebuggerInterface*	m_pPreviousDebugger;
	int							m_iDebugDisplayWidth, m_iDebugDisplayHeight;
};

void RegisterPCVisualDebugger(int iDebugDisplayWidth,int iDebugDisplayHeight)
{
	NT_NEW_CHUNK(Mem::MC_GFX) PCVisualDebugger(iDebugDisplayWidth,iDebugDisplayHeight);
}


