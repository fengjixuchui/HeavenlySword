//-------------------------------------------------------------------------------------------------
//!
//!	\file	/game/guiskinfader.h
//!
//!	Definition of class to manage fading of gui elements.
//!
//-------------------------------------------------------------------------------------------------
#ifndef _GUISKINFADER_H
#define _GUISKINFADER_H

//! Includes
#include "gui/guitext.h"

//------------------------------------------------------------------------------------------------
//!
//! CGuiSkinFader::CGuiSkinFader
//!
//! Gui element fade utility.
//!
//------------------------------------------------------------------------------------------------
class CGuiSkinFader
{
public:
	enum { ALPHA_INDEX = 3 };

	static void FadeStringObject( CString* pobString, const float fAlphaValue ); 
	static void FadeSpriteObject( ScreenSprite& obSprite, const float fAlphaValue );
};

#endif // _GUISKINFADER_H

