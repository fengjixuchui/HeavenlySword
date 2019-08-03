/***************************************************************************************************
*
*	DESCRIPTION		This is a bar rendered between elements of the list
*
*	NOTES			
*
***************************************************************************************************/

#include "cslistseperator.h"
#include "gfx/renderer.h"
#include "gfx/renderstates.h"

#define LINE_WIDTH				857.0f 
#define LINE_END_WIDTH			6.0f
#define LINE_MIDDLE_WIDTH		LINE_WIDTH - LINE_END_WIDTH - LINE_END_WIDTH
#define LINE_MIDDLE_X			LINE_WIDTH/2.0f
#define LINE_END_TEXTURE_SIZE	8.0f
#define LINE_END_TEXTURE_OFFSET LINE_MIDDLE_X - 6.0f//+ ( LINE_END_TEXTURE_SIZE  / 2.0f )

#define NUMLINES 10

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::CSListSeperator
*
*	DESCRIPTION		Construction.
*
***************************************************************************************************/
CSListSeperator::CSListSeperator()
: m_LeftBit( NUMLINES )
, m_MiddleBit( NUMLINES )
, m_RightBit( NUMLINES )
{

}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::~CSListSeperator
*
*	DESCRIPTION		Destruction.
*
***************************************************************************************************/
CSListSeperator::~CSListSeperator()
{

}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::Init
*
*	DESCRIPTION		Creates the 3 sprites based upon the position, default is 0,0,0
*
***************************************************************************************************/
void CSListSeperator::Init()
{
	m_LeftBit.SetTexture(	"gui/frontend/textures/comboscreen/ow_combolineleft_colour_alpha_nomip.dds" );
	m_MiddleBit.SetTexture( "gui/frontend/textures/comboscreen/ow_combolinemiddle_colour_alpha_nomip.dds" );
	m_RightBit.SetTexture(	"gui/frontend/textures/comboscreen/ow_combolinerightside_colour_alpha_nomip.dds" );
	m_LeftBit.SetSizeAll( 0.0f );
	m_MiddleBit.SetSizeAll( 0.0f );
	m_RightBit.SetSizeAll( 0.0f );
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::SetPosition
*
*	DESCRIPTION		Sets the positions of the sprites.
*
***************************************************************************************************/
void CSListSeperator::SetPosition( const CPoint& pointPos , int iLine)
{
	ntError( iLine < NUMLINES );
	ntError( iLine >= 0 );
	//calc the cap positions
	CPoint	REndCapCentre = pointPos + CPoint( LINE_END_TEXTURE_OFFSET , 0.0f , 0.0f );
	CPoint	LEndCapCentre = pointPos - CPoint( LINE_END_TEXTURE_OFFSET , 0.0f , 0.0f );
	//position the line
	m_LeftBit.SetPosition( LEndCapCentre, iLine );
	m_RightBit.SetPosition( REndCapCentre, iLine );
	m_MiddleBit.SetPosition( pointPos, iLine );

}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::Render
*
*	DESCRIPTION		This draws the line.
*
***************************************************************************************************/
void CSListSeperator::Render()
{
	//render all the lines
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	m_LeftBit.Render();
	m_MiddleBit.Render();
	m_RightBit.Render();

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::SetNumberToRender
*
*	DESCRIPTION		Thes sets the size of those that are renedered to the right size and the others to 0
*
***************************************************************************************************/
void CSListSeperator::SetNumberToRender( int iNumRender )
{
	ntError( iNumRender < NUMLINES );
	ntError( iNumRender >= 0 );

	//for all the lines we want to render, set their size
	for( int j = 0 ; j < iNumRender ; ++j )
	{
		m_LeftBit.SetSize(		LINE_END_TEXTURE_SIZE , j );
		m_RightBit.SetSize(		LINE_END_TEXTURE_SIZE , j );
		m_MiddleBit.SetHeight(	LINE_END_TEXTURE_SIZE,	j );
		m_MiddleBit.SetWidth(	LINE_MIDDLE_WIDTH,		j );
	}
	//make all the rest have a size of 0 so they are not visible. Paul F's idea^_^
	for( int j = iNumRender ; j < NUMLINES ; ++j )
	{
		m_LeftBit.SetSize(		0.0f, j );
		m_MiddleBit.SetSize(	0.0f, j );
		m_RightBit.SetSize(		0.0f, j );
	}
}
