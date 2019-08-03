//----------------------------------------------------------------------------------------------------
//!
//!	\file audiohelper.h
//!	Audio Helper functions, basically the same as the bind functions without the LUA, and can
//! be called from anywhere that includes this file
//!
//----------------------------------------------------------------------------------------------------

#ifndef _AUDIOHELPER_H
#define _AUDIOHELPER_H

class CEntity;

class AudioHelper
{
public:
	static void PlaySound (const char* pcEventGroup,const char* pcEvent,CEntity* pobEntity=0);

	static void PlaySound (const char* pcSound,const CPoint& obPosition);
};

#endif //_AUDIOHELPER_H
