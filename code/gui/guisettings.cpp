//--------------------------------------------------
//!
//!	\file gui/guisettings.cpp
//!	
//!
//--------------------------------------------------

#include "guisettings.h"
#include "objectdatabase/dataobject.h"

START_CHUNKED_INTERFACE(GuiSettingsMenuSelectDef, Mem::MC_MISC)

	PUBLISH_VAR_AS( m_obTextures[0], TopLeftTexture )
	PUBLISH_VAR_AS( m_obTextures[1], TopTexture )
	PUBLISH_VAR_AS( m_obTextures[2], TopRightTexture )
	PUBLISH_VAR_AS( m_obTextures[3], RightTexture )
	PUBLISH_VAR_AS( m_obTextures[4], BottomRightTexture )
	PUBLISH_VAR_AS( m_obTextures[5], BottomTexture )
	PUBLISH_VAR_AS( m_obTextures[6], BottomLeftTexture )
	PUBLISH_VAR_AS( m_obTextures[7], LeftTexture )
	PUBLISH_VAR_AS( m_obTextures[8], CentreTexture )

	PUBLISH_VAR_AS( m_obBorderPadding, BorderPadding );
	PUBLISH_VAR_AS( m_obBorderSize, BorderSize );

	PUBLISH_VAR_AS( m_obCursorTexture, CursorTexture );
	PUBLISH_VAR_AS( m_obCursorOffset, CursorOffset );

END_STD_INTERFACE

START_CHUNKED_INTERFACE(GuiSettingsNavBarDef, Mem::MC_MISC)

	PUBLISH_VAR_AS( m_obSelectTitleId, SelectTitleId )
	PUBLISH_VAR_AS( m_obBackTitleId, BackTitleId )

	PUBLISH_VAR_AS( m_obBasePosition, BasePosition )

	PUBLISH_VAR_AS( m_fSelectXOffset, SelectXOffset )
	PUBLISH_VAR_AS( m_fBackXOffset, BackXOffset )
	PUBLISH_VAR_AS( m_fDescriptionXOffset, DescriptionXOffset )

	PUBLISH_VAR_AS( m_obScriptLeftTexture, ScriptLeftTexture );
	PUBLISH_VAR_AS( m_obScriptRightTexture, ScriptRightTexture );
	PUBLISH_VAR_AS( m_obScriptSize, ScriptSize );
	PUBLISH_VAR_AS( m_obScriptOffset, ScriptOffset );

	PUBLISH_VAR_AS( m_obTextFont, TextFont );

	PUBLISH_VAR_AS( m_fFadeTime, FadeTime );

END_STD_INTERFACE

START_CHUNKED_INTERFACE(GuiSettingsDef, Mem::MC_MISC)

	PUBLISH_VAR_AS( m_obDefaultBodyFont, BodyFont )
	PUBLISH_VAR_AS( m_obDefaultTitleFont, TitleFont )

	PUBLISH_VAR_AS( m_obDefaultTextColour, DefaultTextColour );
	PUBLISH_VAR_AS( m_obDefaultTextSelectedColour, DefaultTextSelectedColour );
	PUBLISH_VAR_AS( m_obDefaultTextDisabledColour, DefaultTextDisabledColour );

	PUBLISH_VAR_AS( m_obFilterTexture, FilterTexture )

	PUBLISH_VAR_AS( m_fScreenFadeTime, ScreenFadeTime );

	PUBLISH_PTR_AS( m_pobMenuSelect, MenuSelect )
	PUBLISH_PTR_AS( m_pobNavBar, NavBar )

END_STD_INTERFACE
