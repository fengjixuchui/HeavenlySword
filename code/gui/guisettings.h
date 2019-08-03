//--------------------------------------------------
//!
//!	\file gui/guisettings.h
//!	
//!
//--------------------------------------------------

#ifndef	_GAUISETTINGS_H
#define	_GAUISETTINGS_H

#include "core/nt_std.h"

//--------------------------------------------------
//!
//! Class GuiSettingsMenuSelectDef
//! 
//!
//--------------------------------------------------
class GuiSettingsMenuSelectDef
{
	HAS_INTERFACE(GuiSettingsMenuSelectDef);

public:
	const char* TopLeftTexture() const  { return ntStr::GetString(m_obTextures[0]); }
	const char* TopTexture() const  { return ntStr::GetString(m_obTextures[1]); }
	const char* TopRightTexture() const  { return ntStr::GetString(m_obTextures[2]); }
	const char* RightTexture() const  { return ntStr::GetString(m_obTextures[3]); }
	const char* BottomRightTexture() const  { return ntStr::GetString(m_obTextures[4]); }
	const char* BottomTexture() const  { return ntStr::GetString(m_obTextures[5]); }
	const char* BottomLeftTexture() const  { return ntStr::GetString(m_obTextures[6]); }
	const char* LeftTexture() const  { return ntStr::GetString(m_obTextures[7]); }
	const char* CentreTexture() const  { return ntStr::GetString(m_obTextures[8]); }

	const CVector& BorderPadding() const  { return m_obBorderPadding; }
	const CVector& BorderSize() const  { return m_obBorderSize; }

	const char* CursorTexture() const  { return ntStr::GetString(m_obCursorTexture); }
	const CVector& CursorOffset() const  { return m_obCursorOffset; }

protected:

	ntstd::String m_obTextures[9];

	CVector m_obBorderPadding;
	CVector m_obBorderSize;

	ntstd::String m_obCursorTexture;
	CVector m_obCursorOffset;
};

//--------------------------------------------------
//!
//! Class GuiSettingsNavBarDef
//! 
//!
//--------------------------------------------------
class GuiSettingsNavBarDef
{
	HAS_INTERFACE(GuiSettingsNavBarDef);

public:

	const char* SelectTitleId() const  { return ntStr::GetString(m_obSelectTitleId); }
	const char* BackTitleId() const  { return ntStr::GetString(m_obBackTitleId); }
	const CVector& BasePosition() const  { return m_obBasePosition; }
	float SelectXOffset() const  { return m_fSelectXOffset; }
	float BackXOffset() const  { return m_fBackXOffset; }
	float DescriptionXOffset() const  { return m_fDescriptionXOffset; }

	const char* ScriptLeftTexture() const  { return ntStr::GetString(m_obScriptLeftTexture); }
	const char* ScriptRightTexture() const  { return ntStr::GetString(m_obScriptRightTexture); }
	const CVector& ScriptSize() const  { return m_obScriptSize; }
	const CVector& ScriptOffset() const  { return m_obScriptOffset; }

	const char* TextFont() const  { return ntStr::GetString(m_obTextFont); }

	float FadeTime() const  { return m_fFadeTime; }

protected:

	ntstd::String m_obSelectTitleId;
	ntstd::String m_obBackTitleId;
	CVector m_obBasePosition;
	float m_fSelectXOffset;
	float m_fBackXOffset;
	float m_fDescriptionXOffset;

	ntstd::String m_obScriptLeftTexture;
	ntstd::String m_obScriptRightTexture;
	CVector m_obScriptSize;
	CVector m_obScriptOffset;

	ntstd::String m_obTextFont;

	float m_fFadeTime;
};

//--------------------------------------------------
//!
//! Class GuiSettingsDef
//! 
//!
//--------------------------------------------------
class GuiSettingsDef
{
	HAS_INTERFACE(GuiSettingsDef);

public:

	const char* BodyFont() const { return ntStr::GetString(m_obDefaultBodyFont); }
	const char* TitleFont() const { return ntStr::GetString(m_obDefaultTitleFont); }

	const CVector& DefaultTextColour() const { return m_obDefaultTextColour; }
	const CVector& DefaultTextSelectedColour() const { return m_obDefaultTextSelectedColour; }
	const CVector& DefaultTextDisabledColour() const { return m_obDefaultTextDisabledColour; }

	const char* FilterTexture() const { return ntStr::GetString(m_obFilterTexture); }

	float ScreenFadeTime() const  { return m_fScreenFadeTime; }

	const GuiSettingsMenuSelectDef* MenuSelect() const { return m_pobMenuSelect; }
	const GuiSettingsNavBarDef* NavBar() const { return m_pobNavBar; }

protected:

	ntstd::String m_obDefaultBodyFont;
	ntstd::String m_obDefaultTitleFont;

	CVector m_obDefaultTextColour;
	CVector m_obDefaultTextSelectedColour;
	CVector m_obDefaultTextDisabledColour;

	ntstd::String m_obFilterTexture;

	float m_fScreenFadeTime;

	GuiSettingsMenuSelectDef* m_pobMenuSelect;
	GuiSettingsNavBarDef* m_pobNavBar;
};

#endif // _GAUISETTINGS_H
