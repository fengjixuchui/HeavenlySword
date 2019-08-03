/***************************************************************************************************
*
*	DESCRIPTION		This is a bar rendered between elements of the list
*
*	NOTES	This is just a wrapper for drawing and update 10( well see define in cpp ) sperator lines		
*
***************************************************************************************************/

#ifndef CSLISTSEPERATOR_H
#define CSLISTSEPERATOR_H

#include "effect/screensprite.h"


class CSListSeperator
{
public:
	CSListSeperator();
	~CSListSeperator();

	//create all the bitmaps
	void Init();
	//set the positions of the seperator bars
	void SetPosition( const CPoint& pointPos , int iLine);
	//draw them
	void Render();
	//how many of the bars that I manage do you want to draw
	void SetNumberToRender( int iNumRender );

protected:
	ScreenSprite	m_LeftBit;		///< The left cap of the bar
	ScreenSprite	m_MiddleBit;	///< The middle of the bar
	ScreenSprite	m_RightBit;		///< The right of the bar
};

#endif //CSLISTSEPERATOR_H
