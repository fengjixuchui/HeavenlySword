//------------------------------------------------------------------------------------------
//!
//!	\file osddisplay.h
//!
//! Added JML 09/09/04
//!
//------------------------------------------------------------------------------------------

#ifndef CORE_OSDDISPLAY_H_
#define CORE_OSDDISPLAY_H_

#include "core/visualdebugger.h"

static const int OSD_MAX = 128;

class CEntity;

//------------------------------------------------------------------------------------------
//!
//!	OSD
//!	On Screen Debugging
//!
//------------------------------------------------------------------------------------------
class OSD
{
public:

	// include to build channel list
	// this is a bit gay. change usage of OSD::DEBUG_CHAN
	// to OSD_DEBUG_CHAN then we wont need to do this.
	// Note this is nesecary because you cant forward declare enums with GCC
	#include "game/lua_enum_list.h" //!< \todo DONT reference game for channels

public:
	static void Init();
	static void Add(CHANNEL channel, uint32_t iColour, const char* pcTxt, ...);
	static void Add(CEntity* pEnt, CHANNEL channel, uint32_t iColour, const char* pcTxt, ...);
	static void Log(CHANNEL channel, const char* pcText, ...); // Log a message to the log file but don't onscreen it.
	static void Update(float fTimeDelta);

	static void EnableChannel(CHANNEL c);
	static void DisableChannel(CHANNEL c);
	static bool IsChannelEnabled(CHANNEL c);
	static void SetChannelEnable(CHANNEL c, bool on);
	static void DisableAllChannels();

	static void SetOnScreenTime(float f = 5.0f) {m_fOnscreenTime = f;}

	static const int SLOTS = 20;

private:
	static const int MAXTEXTLEN = 100;

private:
	static float m_fOnscreenTime;
	static char  m_aText[SLOTS][MAXTEXTLEN];
	static long  m_aColour[SLOTS];
	static float m_aTime[SLOTS];
	static int   m_iStart;
	static int   m_iEnd;
	static bool  m_bChannels[];
	static char* m_apcChannels[];
};

#endif // end of _OSDDISPLAY_H_
