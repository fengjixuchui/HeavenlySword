/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_PAD_H
#define ICE_PAD_H


#include "icebase.h"


namespace Ice
{
	enum
	{
		kMaxPadCount		= 2
	};
	
	
	enum
	{
		kPadLUp			= 0x0001,
		kPadLDown		= 0x0002,
		kPadLLeft		= 0x0004,
		kPadLRight		= 0x0008,
		kPadRUp			= 0x0010,
		kPadRDown		= 0x0020,
		kPadRLeft		= 0x0040,
		kPadRRight		= 0x0080,
		kPadL1			= 0x0100,
		kPadL2			= 0x0200,
		kPadR1			= 0x0400,
		kPadR2			= 0x0800,
		kPadStart		= 0x1000,
		kPadSelect		= 0x2000,
		kPadL3			= 0x4000,
		kPadR3			= 0x8000
	};
	
	
	struct PadInfo
	{
		float		m_leftX;
		float		m_leftY;
		float		m_rightX;
		float		m_rightY;
		U8			m_rawLeftX;
		U8			m_rawLeftY;
		U8			m_rawRightX;
		U8			m_rawRightY;
		U16			m_keyPressed;   // 1 if being held
		U16			m_keyUp;        // "just released".  PadUpdate will set and clear this.  Game logic must be called inbetween calls to PadUpdate
		U16			m_keyDown;      // "just hit"          or else hit/release status will be missed by game.
		bool		m_on;           // true if connected & recognized.  If false then ignore all other data (which is undefined)!!!
	};
	
	
	extern PadInfo g_padInfo[kMaxPadCount];
	
	
	int PadInit(void);
	int PadShutdown(void);
	int PadUpdate(void);
}


#endif
