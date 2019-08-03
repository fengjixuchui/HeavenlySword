//------------------------------------------------------------------------------------------
//!
//!	\file	languages.h
//!
//!			Enumeration of all supported languages.
//!
//------------------------------------------------------------------------------------------

#ifndef	LANGUAGES_H_
#define	LANGUAGES_H_

#ifdef PLATFORM_PS3

	#include <sysutil/sysutil_common.h>

#endif // PLATFORM_PS3

enum AudioLanguage
{
	NT_AUDIO_DANISH			= 0,
	NT_AUDIO_DUTCH,
	NT_AUDIO_EURO_ENGLISH,
	NT_AUDIO_FINNISH,
	NT_AUDIO_FRENCH,
	NT_AUDIO_GERMAN,
	NT_AUDIO_ITALIAN,
	NT_AUDIO_JAPANESE,
	NT_AUDIO_NORWEGIAN,
	NT_AUDIO_PORTUGUESE,
	NT_AUDIO_SPANISH,
	NT_AUDIO_SWEDISH,

	NT_AUDIO_NUM_LANGUAGES
};

enum SubtitleLanguage
{
	NT_SUBTITLE_DANISH			= 0,
	NT_SUBTITLE_DUTCH,
	NT_SUBTITLE_EURO_ENGLISH,
	NT_SUBTITLE_FINNISH,
	NT_SUBTITLE_FRENCH,
	NT_SUBTITLE_GERMAN,
	NT_SUBTITLE_ITALIAN,
	NT_SUBTITLE_JAPANESE,
	NT_SUBTITLE_NORWEGIAN,
	NT_SUBTITLE_PORTUGUESE,
	NT_SUBTITLE_SPANISH,
	NT_SUBTITLE_SWEDISH,
	// Everything up to here also has an equivalent audio language.

	NT_SUBTITLE_US_ENGLISH,			// Equivalent audio is NT_AUDIO_ENGLISH.
	NT_SUBTITLE_KOREAN,				// No equivalent audio.
	NT_SUBTITLE_CHINESE,

	NT_SUBTITLE_NUM_LANGUAGES
};

namespace Language
{
	const char *GetAudioLanguageName( AudioLanguage lang );
	const char *GetSubtitleLanguageName( SubtitleLanguage lang );

#ifdef PLATFORM_PS3
	
	const SubtitleLanguage GetSubtitleLanguageFromSysLanguage( const int iIDSysLanguage );

#endif // PLATFORM_PS3
}

#endif // !LANGUAGES_H_


