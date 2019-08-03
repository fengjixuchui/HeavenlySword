/***************************************************************************************************
*
*	DESCRIPTION		Input Contexts
*
*	NOTES			The list of all possible input contexts for the game. The shell is responsible
*					for changing input context
*
***************************************************************************************************/

#ifndef	_INPUTCONTEXTS_H
#define	_INPUTCONTEXTS_H

extern char const* g_apcContextTitleStrings[];
extern char const* g_apcContextDescriptionStrings[];

// If you update this, please also update g_apcContextTitleStrings and g_apcContextDescriptionStrings
enum	INPUT_CONTEXT
{
	INPUT_CONTEXT_GAME = 0,					

	INPUT_CONTEXT_RENDER_DEBUG,
	INPUT_CONTEXT_CAMERA_DEBUG,
	INPUT_CONTEXT_DYNAMICS_DEBUG,
	INPUT_CONTEXT_SOUND_DEBUG, 
	INPUT_CONTEXT_COMBAT,
	INPUT_CONTEXT_AI,
	INPUT_CONTEXT_SCRIPT,
	INPUT_CONTEXT_PROFILING,

	INPUT_CONTEXT_MAX,				// this one must be last
};

// NEW 29.03.04 due to using up all the pad buttons in the game
enum	PAD_CONTEXT
{
	PAD_CONTEXT_GAME = 0,
	PAD_CONTEXT_DEBUG,
	PAD_CONTEXT_MAX,
};

#endif	// _INPUTCONTEXTS_H
