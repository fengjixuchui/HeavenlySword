//--------------------------------------------------
//!
//!	\file functiongraph.h
//!	Helper graphing class
//!
//--------------------------------------------------

#ifndef _FGRAPH_H
#define _FGRAPH_H

#ifndef _GOLD_MASTER

//--------------------------------------------------
//!
//!	FunctionGraph
//!
//--------------------------------------------------
class FunctionGraph
{
public:
	enum TEXT_POS
	{
		OVER,
		LEFT,
		RIGHT,
		ABOVE,
		BELOW,
	};

	FunctionGraph( float fminX, float fmaxX, float fminY, float fmaxY );

	void DrawBounds( uint32_t dwCol );
	void DrawLine( const CPoint& p1, const CPoint& p2, uint32_t dwCol );
	void DrawCross( const CPoint& p1, uint32_t dwCol, float fHalfwidth );
	void DrawText( const char* pText, const CPoint& pos, uint32_t dwCol, TEXT_POS where );

	void GetPixelWidth(float&, float&);

private:
	CPoint GetScreenPos( const CPoint& pos )
	{
		return (CPoint( pos.X() * m_fGraphSizeX, pos.Y() * m_fGraphSizeY, 0.0f ) + m_bottomLeft);
	}

	float m_fGraphMinX, m_fGraphSizeX;
	float m_fGraphMinY, m_fGraphSizeY;
	CPoint	m_bottomLeft;
};

#endif

#endif // _FGRAPH_H
