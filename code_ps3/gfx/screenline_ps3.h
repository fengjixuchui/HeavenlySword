//--------------------------------------------------
//!
//!	\file gfx/screenline_ps3.h
//! 2D drawing utils
//!
//--------------------------------------------------

#ifndef GFX_SCREENLINE_PS3_H
#define GFX_SCREENLINE_PS3_H

#ifndef GFX_SHADER_H
#include "gfx/shader.h"
#endif

//--------------------------------------------------
//!
//!	ScreenLines
//! Class that draws 2D lines in screen space,
//! used by the PS3 Visual Debugger
//!
//--------------------------------------------------
class	ScreenLines
{
public:
	ScreenLines(	float fScreenWidth,
					float fScreenHeight,
					uint32_t iMaxLines );

	~ScreenLines();

	void AddLine(	float fStartX, float fStartY,
					float fEndX, float fEndY,
					uint32_t iColour );

	void FlushPrims();

private:
	struct LineVertex
	{
		float		m_pos[2];
		uint32_t	m_col;
	};
	struct	Line
	{
		LineVertex	m_start;
		LineVertex	m_end;
	};

	DebugShader	*	m_vertexShader;
	DebugShader	*	m_pixelShader;

	VBHandle		m_pLineBuffer;
	float			m_fRCPScreenWidth;
	float			m_fRCPScreenHeight;
	uint32_t		m_iMaxLines;
	uint32_t		m_iCurrLine;
	Line*			m_pLines;
};

#endif // GFX_SCREENLINE_PS3_H

