//------------------------------------------------------------------------------------------
//!
//!	\file osddisplay.cpp
//!
//------------------------------------------------------------------------------------------


//--------------------------------------
// Includes
//--------------------------------------
#include "game/shelldebug.h"
#include "core/osddisplay.h"
#include "core/visualdebugger.h"
#include "input/inputhardware.h"

//--------------------------------------
// Constants
//--------------------------------------
static const float OSD_TOP  = sfOSDTopBorder;
static const float OSD_LEFT = sfDebugLeftBorder;
static const float OSD_INC  = sfDebugLineSpacing;

//--------------------------------------
// Statics
//--------------------------------------
float OSD::m_fOnscreenTime = 5.0f;
char  OSD::m_aText[SLOTS][100];
long  OSD::m_aColour[SLOTS];
float OSD::m_aTime[SLOTS];
int   OSD::m_iStart = 0;
int   OSD::m_iEnd   = 0;
bool  OSD::m_bChannels[COUNT] = {false};
char* OSD::m_apcChannels[COUNT] = {"AI Combat", "Net", "Movement", "Cam", "Objects", "Hair", "OSD", "AI", "Formations", "Script"};


//--------------------------------------
// Macros
//--------------------------------------

// BINDCHANNEL - Bind a channel to a ctrl+key
// ------------------------------------------
#define BINDCHANNEL(channel, key)												\
	if(CInputHardware::Get().GetKeyboard().IsKeyPressed(KEYC_##key, KEYM_CTRL))	\
	{																			\
		if(!m_bChannels[channel])												\
		{																		\
			m_bChannels[channel] = true;										\
			OSD::Add(channel, 0xff00ff00, #channel " OSD Enabled.");			\
		}																		\
		else if(m_bChannels[channel])											\
		{																		\
			OSD::Add(channel, 0xff00ff00, #channel " OSD Disabled.");			\
			m_bChannels[channel] = false;										\
		}																		\
	}																			

void OSD::Init()
{
#if defined(_DEBUG)
	EnableChannel(DEBUG_CHAN);
#endif
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	OSD::Add
//! Add a message to the On Screen Debug
//!                                                                                         
//------------------------------------------------------------------------------------------
void OSD::Add(CHANNEL channel, unsigned int iColour, const char* pcTxt, ...)
{
	if(channel >= COUNT || channel < 0 || !m_bChannels[channel])
		return;

	// Format the text.
	va_list	stArgList;
	va_start(stArgList, pcTxt);
	int iLen = vsnprintf(m_aText[m_iEnd], MAXTEXTLEN-1, pcTxt, stArgList);
	va_end(stArgList);

	// Null terminate it,
	m_aText[m_iEnd][iLen] = 0;

	// Add it to the log too...
	ntPrintf("%s: %s\n", m_apcChannels[channel], m_aText[m_iEnd]);

	// Set colour and Time.
	m_aColour[m_iEnd] = iColour;
	m_aTime[m_iEnd++] = m_fOnscreenTime;

	// Change start and end.
	m_iEnd = m_iEnd % SLOTS;
	if(m_iEnd == m_iStart)
		m_iStart = (m_iStart+1)%SLOTS;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	OSD::Add
//! Add a message to the On Screen Debug pertinent to a particular entity
//!                                                                                         
//------------------------------------------------------------------------------------------
void OSD::Add(CEntity* pEnt, CHANNEL channel, unsigned int iColour, const char* pcTxt, ...)
{
	UNUSED(pEnt);
	UNUSED(iColour);

	if(channel >= COUNT || channel < 0 || !m_bChannels[channel])
		return;

	char acCaption[MAXTEXTLEN];

	// Format the text.
	va_list	stArgList;
	va_start(stArgList, pcTxt);
	int iLen = vsnprintf(acCaption, MAXTEXTLEN-1, pcTxt, stArgList);
	va_end(stArgList);

	// Null terminate it,
	acCaption[iLen] = 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	OSD::Log
//! Output a message to the log file if the channel is enabled but don't put it on screen.
//!                                                                                         
//------------------------------------------------------------------------------------------
void OSD::Log(CHANNEL channel, const char* pcTxt, ...)
{
	if(channel >= COUNT || channel < 0 || !m_bChannels[channel])
		return;

	// Format the text.
	va_list	stArgList;
	va_start(stArgList, pcTxt);
	int iLen = vsnprintf(m_aText[m_iEnd], MAXTEXTLEN-1, pcTxt, stArgList);
	va_end(stArgList);

	// Null terminate it,
	m_aText[m_iEnd][iLen] = 0;

	// Add it to the log too...
	ntPrintf("%s: %s\n", m_apcChannels[channel], m_aText[m_iEnd]);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	OSD::Update
//! Update the OSD Display
//!                                                                                         
//------------------------------------------------------------------------------------------
void OSD::Update(float fTimeDelta)
{
	ntAssert(m_iStart >= 0);
	ntAssert(m_iEnd < SLOTS);

	// Bind channels to their keys.
	BINDCHANNEL(AICOMBAT,		A);
	BINDCHANNEL(AISTATES,		I);
	BINDCHANNEL(NETWORK,		N);
	BINDCHANNEL(MOVEMENT,		M);
	BINDCHANNEL(CAMERA,			C);
	BINDCHANNEL(OBJECTS,		O);
	BINDCHANNEL(HAIR,			H);
	BINDCHANNEL(DEBUG_CHAN,		D);
	BINDCHANNEL(AIFORMATIONS,	F);
	BINDCHANNEL(SCRIPT,			S);
	BINDCHANNEL(ARMY,			R);

	// Update display.
	float y = OSD_TOP;
	for(int i = m_iStart;((m_iStart <= m_iEnd)? (i < m_iEnd) : (i < m_iEnd || i >= m_iStart)); i = (i+1) % SLOTS)
	{
		y += OSD_INC;

		if(m_aTime[i] < 0.0f) // Remove old messages
		{
			m_iStart = (i + 1) % SLOTS;
			y -= OSD_INC;
			continue;
		}
		
		if(m_aTime[i] <= 0.5f) // Scroll up messages before removing
		{
			y -= (0.5f - m_aTime[i]) * OSD_INC * 2.0f;
		}
		if(m_aTime[i] < 1.0f) // Fade out messages about to be removed
		{
			long A = (char)(0xff * 2.0f * ( (m_aTime[i]>0.5f) ? (1.0f) : (m_aTime[i]-0.5f)) ) << 24;
			m_aColour[i] &= 0x00ffffff;
			m_aColour[i] |= A;
		}

		// Render a message
#ifndef _GOLD_MASTER
		g_VisualDebug->Printf2D(OSD_LEFT, y, m_aColour[i], 0, m_aText[i]);
#endif
		m_aTime[i] -= fTimeDelta;
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	OSD::EnableChannel
//! Display messages from this channel.
//!                                                                                         
//------------------------------------------------------------------------------------------
void OSD::EnableChannel(CHANNEL c)
{
	if(c >= COUNT || c < 0) 
		return; 
	m_bChannels[c] = true;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	OSD::DisableChannel
//! Don't display messages from this channel.
//!                                                                                         
//------------------------------------------------------------------------------------------
void OSD::DisableChannel(CHANNEL c)
{
	if(c >= COUNT || c < 0) 
		return; 
	m_bChannels[c] = false;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	OSD::IsChannelEnabled
//! Do we display messages from this channel.
//!                                                                                         
//------------------------------------------------------------------------------------------
bool OSD::IsChannelEnabled(CHANNEL c)
{
	if(c >= COUNT || c < 0) 
		return false; 
	return m_bChannels[c];
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	OSD::IsChannelEnabled
//! Don't display messages from any channel.
//!                                                                                         
//------------------------------------------------------------------------------------------
void OSD::DisableAllChannels()
{
	// Disable all OSD
	for (int i = 0; i<SLOTS; i++)
		DisableChannel((CHANNEL)i);
}

void OSD::SetChannelEnable(CHANNEL c, bool on)
{
	if(c >= COUNT || c < 0) 
		return; 
	m_bChannels[c] = on;
}
