//--------------------------------------------------
//!
//!	\file functiongraph.cpp
//!	Helper graphing class
//!
//--------------------------------------------------

#include "functiongraph.h"
#include "core/visualdebugger.h"

#ifndef _GOLD_MASTER

//--------------------------------------------------
//!
//!	construct the graph drawing helper
//!
//--------------------------------------------------
FunctionGraph::FunctionGraph( float fminX, float fmaxX, float fminY, float fmaxY )
{
	// min and max are in % of viewport
	float fVPWidth	= g_VisualDebug->GetDebugDisplayWidth();
	float fVPHeight	= g_VisualDebug->GetDebugDisplayHeight();

	m_fGraphMinX = fminX * fVPWidth;
	m_fGraphSizeX = (fmaxX - fminX) * fVPWidth;

	m_fGraphMinY = fmaxY * fVPHeight;
	m_fGraphSizeY = (fminY - fmaxY) * fVPHeight;

	m_bottomLeft = CPoint( m_fGraphMinX, m_fGraphMinY, 0.0f );
}

//--------------------------------------------------
//!
//!	draw graph extents
//!
//--------------------------------------------------
void FunctionGraph::DrawBounds( uint32_t dwCol )
{
	CPoint aCorners[3];
	aCorners[0] = GetScreenPos( CPoint(1.0f, 0.0f, 0.0f) );
	aCorners[1] = GetScreenPos( CPoint(1.0f, 1.0f, 0.0f) );
	aCorners[2] = GetScreenPos( CPoint(0.0f, 1.0f, 0.0f) );

	g_VisualDebug->RenderLine( m_bottomLeft, aCorners[0], dwCol, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( aCorners[0], aCorners[1], dwCol, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( aCorners[1], aCorners[2], dwCol, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( aCorners[2], m_bottomLeft, dwCol, DPF_DISPLAYSPACE );
}

//--------------------------------------------------
//!
//!	draw a line in graph space
//!
//--------------------------------------------------
void FunctionGraph::DrawLine( const CPoint& p1, const CPoint& p2, uint32_t dwCol )
{
	CPoint curr( GetScreenPos(p1) );
	CPoint next( GetScreenPos(p2) );

	g_VisualDebug->RenderLine( curr, next, dwCol, DPF_DISPLAYSPACE );
}

//--------------------------------------------------
//!
//!	draw a cross in graph space
//!
//--------------------------------------------------
void FunctionGraph::DrawCross( const CPoint& p1, uint32_t dwCol, float fHalfwidth )
{
	CPoint curr( GetScreenPos(p1) );

	g_VisualDebug->RenderLine( curr - CPoint(fHalfwidth,0.0f,0.0f), curr + CPoint(fHalfwidth,0.0f,0.0f), dwCol, DPF_DISPLAYSPACE );
	g_VisualDebug->RenderLine( curr - CPoint(0.0f,fHalfwidth,0.0f), curr + CPoint(0.0f,fHalfwidth,0.0f), dwCol, DPF_DISPLAYSPACE );
}

//--------------------------------------------------
//!
//!	draw some text in graph space
//!
//--------------------------------------------------
void FunctionGraph::DrawText( const char* pText, const CPoint& pos, uint32_t dwCol, TEXT_POS where )
{
	CPoint drawPos(	GetScreenPos(pos) );
	int iLen = strlen( pText );

	switch( where )
	{
	case OVER:	drawPos.X() -= iLen * 5.0f;		drawPos.Y() -= 5.0f;		break;
	case LEFT:	drawPos.X() -= iLen * 10.0f;	drawPos.Y() -= 5.0f;		break;
	case RIGHT:	drawPos.X() += iLen * 10.0f;	drawPos.Y() -= 5.0f;		break;
	case ABOVE:	drawPos.X() -= iLen * 5.0f;		drawPos.Y() -= 10.0f;		break;
	case BELOW:	drawPos.X() -= iLen * 5.0f;		drawPos.Y() += 10.0f;		break;
	}

	g_VisualDebug->Printf2D( drawPos.X(), drawPos.Y(), dwCol, 0, pText );
}

//--------------------------------------------------
//!
//!	get screen pixel size in graph scale
//!
//--------------------------------------------------
void FunctionGraph::GetPixelWidth(float& x, float& y)
{
	x = 1.0f / m_fGraphSizeX;
	y = 1.0f / m_fGraphSizeY;
}

#endif
