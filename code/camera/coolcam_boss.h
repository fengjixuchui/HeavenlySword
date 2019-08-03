//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Boss.h
//!
//------------------------------------------------------------------------------------------


#ifndef _COOLCAM_BOSS_H
#define _COOLCAM_BOSS_H

#include "camera/camcool.h"

class CEntity;
class Boss;

//------------------------------------------------------------------------------------------
//! 
//!	CoolCam_Boss
//! 
//------------------------------------------------------------------------------------------
class CoolCam_Boss : public CoolCamera
{
public:
	CoolCam_Boss(const CamView& view, const CEntity* pobPlayer, const CEntity* pobBoss);
	virtual ~CoolCam_Boss();

	virtual void        Update(float fTimeDelta);
	virtual void        Reset() {}
	virtual bool        HasFinished() const   {return m_bFinished;}
	virtual CHashedString GetCameraName() const {return "*Boss Cam*";}
	virtual CAMTYPE     GetType() const       {return CT_BOSS;}

#ifndef _RELEASE
	virtual void RenderDebugInfo() {;}
#endif

// Members
protected:
	const CEntity* m_pobPlayer;
	const CEntity* m_pobBoss;
};

#endif // _COOLCAM_BOSS_H
