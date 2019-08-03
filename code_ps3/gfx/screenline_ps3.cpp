//--------------------------------------------------
//!
//!	\file gfx/screenline_ps3.cpp
//! Class that draws 2D lines in screen space,
//! used by the PS3 Visual Debugger
//!
//--------------------------------------------------

#include "gfx/screenline_ps3.h"
#include "gfx/renderer.h"

//--------------------------------------------------
//!
//!	ScreenLines::ctor
//!
//--------------------------------------------------
ScreenLines::ScreenLines(	float fScreenWidth,
							float fScreenHeight,
							uint32_t iMaxLines )
{
	ntAssert( fScreenWidth > 0.0f );
	ntAssert( fScreenHeight > 0.0f );
	ntAssert( iMaxLines > 0 );

	m_fRCPScreenWidth = 1.0f / fScreenWidth;
	m_fRCPScreenHeight = 1.0f / fScreenHeight;
	m_iMaxLines = iMaxLines;
	m_iCurrLine = 0;

	m_vertexShader = DebugShaderCache::Get().LoadShader( "passthrough_pos_col_vp.sho" );
	m_pixelShader = DebugShaderCache::Get().LoadShader( "simplecolour_fp.sho" );

	GcStreamField	desc[] = 
	{
		GcStreamField( FwHashedString( "IN.position" ), 0, Gc::kFloat, 2 ),	// 2 * 4 bytes
		GcStreamField( FwHashedString( "IN.colour" ), 8, Gc::kUByte, 4, true ),	// 4 * 1 bytes
	};

	m_pLineBuffer = RendererPlatform::CreateVertexStream(	m_iMaxLines * 2,
															sizeof( LineVertex ), 2,
															desc, Gc::kScratchBuffer );
	m_pLines = NT_NEW_CHUNK( Mem::MC_GFX ) Line [m_iMaxLines];
}

ScreenLines::~ScreenLines()
{
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, m_pLines );
}

//--------------------------------------------------
//!
//!	ScreenLines::AddLine
//! Add a line to the line list for this frame.
//!
//--------------------------------------------------
void ScreenLines::AddLine(	float fStartX, float fStartY,
							float fEndX, float fEndY,
							uint32_t iNTColour )
{
	if (m_iCurrLine >= m_iMaxLines)
		return;

	// we must convert from screen space to VP space:
	//---------------------------------------------------------

	// screen space top left = (0,0)
	// screen space bottom right = (1280,720) (for instance)
	
	// viewport space top left = (-1,1)
	// screen space bottom right = (1,-1)

	m_pLines[m_iCurrLine].m_start.m_pos[0] = (fStartX * m_fRCPScreenWidth * 2.0f) - 1.0f;
	m_pLines[m_iCurrLine].m_start.m_pos[1] = ((1.0f-(fStartY * m_fRCPScreenHeight)) * 2.0f) - 1.0f;

	m_pLines[m_iCurrLine].m_end.m_pos[0] = (fEndX * m_fRCPScreenWidth * 2.0f) - 1.0f;
	m_pLines[m_iCurrLine].m_end.m_pos[1] = ((1.0f-(fEndY * m_fRCPScreenHeight)) * 2.0f) - 1.0f;

	m_pLines[m_iCurrLine].m_start.m_col = iNTColour;
	m_pLines[m_iCurrLine].m_end.m_col = iNTColour;

	m_iCurrLine++;
}

//--------------------------------------------------
//!
//!	ScreenLines::FlushPrims
//! Write our lines to the draw buffer and draw them
//!
//--------------------------------------------------
void ScreenLines::FlushPrims()
{
	if (m_iCurrLine)
	{
		if (m_pLineBuffer->QueryGetNewScratchMemory())
		{
			m_pLineBuffer->GetNewScratchMemory();
			m_pLineBuffer->Write( m_pLines, 0, sizeof( Line ) * m_iCurrLine );
			
			Renderer::Get().SetVertexShader( m_vertexShader );
			Renderer::Get().SetPixelShader( m_pixelShader );

			Renderer::Get().m_Platform.SetStream( m_pLineBuffer );
			Renderer::Get().m_Platform.DrawPrimitives( Gc::kLines, 0, 2*m_iCurrLine );
			Renderer::Get().m_Platform.ClearStreams();
		}

		m_iCurrLine = 0;
	}
}
