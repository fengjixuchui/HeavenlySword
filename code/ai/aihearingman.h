//! -------------------------------------------
//! aihearingman.h
//!
//! AI Hearing Manager
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIHEARINGMAN_H
#define _AIHEARINGMAN_H

#include "lua/ninjalua.h"

class CEntity;
class AI;

typedef struct _SDebugSoundInfo
{
	_SDebugSoundInfo() :	m_pEnt(NULL), m_fDbgEntityRadius(0.0f), m_fDbgSoundRadius(0.0f), m_fDbgSoundVolume(0.0f), 
							m_bDbgSoundConstant(true), m_fTimer(0.0f), m_bWasItHeard(false) {}
	_SDebugSoundInfo(CEntity* pE, float fSR, float fSV, bool b) 
	{	m_pEnt = pE, m_fDbgEntityRadius = 0.0f ; m_fDbgSoundRadius = fSR; m_fDbgSoundVolume = fSV; 
		m_bDbgSoundConstant = true; m_fTimer = 0.0f; m_bWasItHeard = b; }
	
	CEntity* m_pEnt;
	float m_fDbgEntityRadius;
	float m_fDbgSoundRadius;
	float m_fDbgSoundVolume;
	float m_bDbgSoundConstant;
	float m_fTimer;
	float m_bWasItHeard;
} SDebugSoundInfo;

//!--------------------------------------------
//! CAIHearingMan
//!--------------------------------------------
class CAIHearingMan : public Singleton<CAIHearingMan>
{
	public:
		
		CAIHearingMan();

		HAS_LUA_INTERFACE();

		void		TriggerSound			( CEntity*, float, float, bool = true );
		CEntity*	GetSoundSourceEntity	( AI* );

		// Update ( for DebugRender Only )
		void Update		( float );

	public:
	
		bool m_bSoundInfoRender;

	private:
		SDebugSoundInfo m_SDebugInfo;
		
};

LV_DECLARE_USERDATA(CAIHearingMan);

#endif //_AIHEARINGMAN_H




