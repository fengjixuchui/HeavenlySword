//-----------------------------------------------------------------------------
//!
//!	\file /game/guiskinfader.cpp
//!
//! Implementation of class to manage fading of gui elements.
//!
//-----------------------------------------------------------------------------

//! Includes
#include "game/guiskinfader.h"

//------------------------------------------------------------------------------------------------
//!
//! CGuiSkinFader::FadeStringObject
//!
//! Apply the alpha value to the string object.
//!
//------------------------------------------------------------------------------------------------
void CGuiSkinFader::FadeStringObject( CString* pobString, const float fAlphaValue )
{
	ntAssert_p( pobString != NULL, ("CGuiSkinFader::FadeStringObject() -> pobString was NULL.\n") );

	if	( pobString )
	{
		// Get current colour.
		CVector obColour = pobString->GetColour();
		
		// Update alpha.
		obColour[ ALPHA_INDEX ] = fAlphaValue;
		pobString->SetColour( obColour );
	}
}

//------------------------------------------------------------------------------------------------
//!
//! CGuiSkinFader::FadeSpriteObject
//!
//! Apply the alpha value to the sprite object.
//!
//------------------------------------------------------------------------------------------------
void CGuiSkinFader::FadeSpriteObject( ScreenSprite& obSprite, const float fAlphaValue )
{
	CVector obColour;

	// Get current colour.
	obSprite.GetColourAll( obColour );
	
	// Update alpha.
	obColour[ ALPHA_INDEX ] = fAlphaValue;
	obSprite.SetColourAll( obColour );
}

