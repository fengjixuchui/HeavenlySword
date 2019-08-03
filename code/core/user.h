//------------------------------------------------------------------------------------------
//!
//! Macros to define user specific code		
//!
//------------------------------------------------------------------------------------------

#ifndef	_RELEASE

	#include "core/singleton.h"
	#include "game/shellconfig.h"

	#define user_code_start(name) if (g_ShellOptions->m_obUserName == #name) {
	#define user_code_end() }

#else	//_RELEASE


	#define user_code_start(name) if (0) {
	#define user_code_end() }

#endif	//_RELEASE
