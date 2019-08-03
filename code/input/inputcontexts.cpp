//------------------------------------------------------------
//!
//! \file input\inputcontexts.cpp
//! Static data for the game contexts
//!
//------------------------------------------------------------
			
#include "input/inputcontexts.h"

// The textual version of contexts
// NOTE: This MUST match the INPUT_CONTEXT enum in keybinder.h
char const* g_apcContextTitleStrings[] = 
{
//	CONTEXT
	"game",			
	"rendering",	
	"camera",		
	"dynamics",		
	"sound",		
	"combat",		
	"ai",	
	"script",
	"profiling"
};

// The textual version of context descriptions
// NOTE: This MUST match the INPUT_CONTEXT enum in keybinder.h
char const* g_apcContextDescriptionStrings[] = 
{
//	DESCRIPTION
	"All default keys",			// game
	"Rendering related keys",	// rendering
	"Camera related keys",		// camera
	"Dynamics related keys",	// dynamics
	"Sound related keys",		// sound
	"Combat related keys",		// combat
	"AI related keys",			// AI
	"Script key pokes",			// script
	"Profile related keys"		// profile
};







