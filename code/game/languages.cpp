//------------------------------------------------------------------------------------------
//!
//!	\file	languages.cpp
//!
//!			Enumeration of all supported languages.
//!
//------------------------------------------------------------------------------------------

#include "game/languages.h"

namespace
{
	static const uint32_t NumLanguages = 15;
	static_assert_in_class( NumLanguages == NT_SUBTITLE_NUM_LANGUAGES, Array_size_out_of_date_Needs_fixing );
	static_assert_in_class( uint32_t( NT_AUDIO_NUM_LANGUAGES ) <= uint32_t( NT_SUBTITLE_NUM_LANGUAGES ), Why_do_we_have_languages_that_are_audio_only );

	static const char *LanguageNames[ NumLanguages ] =
	{
		"danish",
		"dutch",
		"english",		// call euro-english plain old "english".
		"finnish",
		"french",
		"german",
		"italian",
		"japanese",
		"norwegian",
		"portuguese",
		"spanish",
		"swedish",		// last audio and subtitle language.

		"us_english",	//	These last two are subtitle only.
		"korean",		//
		"chinese"
	};
}

const char *Language::GetSubtitleLanguageName( SubtitleLanguage lang )
{
	ntError( uint32_t( lang ) < NT_SUBTITLE_NUM_LANGUAGES );
	return LanguageNames[ lang ];
}

const char *Language::GetAudioLanguageName( AudioLanguage lang )
{
	ntError( uint32_t( lang ) < NT_AUDIO_NUM_LANGUAGES );
	return LanguageNames[ lang ];
}

#ifdef PLATFORM_PS3
//---------------------------------------------------------------------------------------------
//!
//! Language::GetSubtitleLanguageFromSysLanguage
//!
//! Language id's obtained using cellSysutilGetSystemParamInt are in a different order to ours
//! so need to re-map.
//!
//---------------------------------------------------------------------------------------------
const SubtitleLanguage Language::GetSubtitleLanguageFromSysLanguage( const int iIDSysLanguage )
{
	// Sysutil does not offer support for these languages.
	//
	// NT_SUBTITLE_DANISH
	// NT_SUBTITLE_FINNISH
	// NT_SUBTITLE_NORWEGIAN
	// NT_SUBTITLE_SWEDISH

	switch	( iIDSysLanguage )
	{
	case CELL_SYSUTIL_LANG_JAPANESE:
		return NT_SUBTITLE_JAPANESE;

	case CELL_SYSUTIL_LANG_ENGLISH:
		return NT_SUBTITLE_EURO_ENGLISH;
	
	case CELL_SYSUTIL_LANG_FRENCH:
		return NT_SUBTITLE_FRENCH;

	case CELL_SYSUTIL_LANG_SPANISH:
		return NT_SUBTITLE_SPANISH;

	case CELL_SYSUTIL_LANG_GERMAN:
		return NT_SUBTITLE_GERMAN;

	case CELL_SYSUTIL_LANG_ITALIAN:
		return NT_SUBTITLE_ITALIAN;

	case CELL_SYSUTIL_LANG_DUTCH:
		return NT_SUBTITLE_DUTCH;

	case CELL_SYSUTIL_LANG_PORTUGUESE:
		return NT_SUBTITLE_PORTUGUESE;

	case CELL_SYSUTIL_LANG_RUSSIAN:		// Not supported by game.
		return NT_SUBTITLE_NUM_LANGUAGES;

	case CELL_SYSUTIL_LANG_KOREAN:		// Not supported by game.
		return NT_SUBTITLE_NUM_LANGUAGES;

	case CELL_SYSUTIL_LANG_CHINESE_T:	// Not supported by game.
		return NT_SUBTITLE_NUM_LANGUAGES;

	case CELL_SYSUTIL_LANG_CHINESE_S:	// Not supported by game.
		return NT_SUBTITLE_NUM_LANGUAGES;
	
	default:
		return NT_SUBTITLE_NUM_LANGUAGES;
	}
}
#endif // PLATFORM_PS3
