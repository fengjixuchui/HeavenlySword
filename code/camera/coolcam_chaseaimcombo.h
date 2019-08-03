//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Aim.h
//!
//------------------------------------------------------------------------------------------


#ifndef _COOLCAM_CHASEAIMCOMBO_H
#define _COOLCAM_CHASEAIMCOMBO_H

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Required Includes
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include "camera/camcool.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// External Classes
/////////////////////////////////////////////////////////////////////////////////////////////////////
class CEntity;
class CoolCam_AimDef;
class CoolCam_ChaseDef;
class CoolCam_Chase;
class CoolCam_Aim;


//------------------------------------------------------------------------------------------
//! 
//!	CoolCam_ChaseAimCombo
//!	A camera that combines a chasing camera, an aiming camera and a mechanism to move from
//! one to the other.
//! 
//------------------------------------------------------------------------------------------
class CoolCam_ChaseAimCombo : public CoolCamera
{
public:
	// Mode
	enum MODE {AIM, CHASE, AIM_NEVER_CHASE};

public:
	CoolCam_ChaseAimCombo(const CamView& view, const CEntity& ent, 
		                  CoolCam_ChaseDef& chaseDef, CoolCam_AimDef& aimDef);
	virtual ~CoolCam_ChaseAimCombo();

	virtual void        Update(float fTimeDelta);
	virtual void        Reset() {;}
	virtual bool        HasFinished()   const  {return m_bFinished;}
	virtual bool		IsActive()      const  {return m_bActive && !m_bFinished;}
	virtual CHashedString GetCameraName() const  {return CHashedString(HASH_STRING__CHASE_AIM_COMBO_CAM_);}		// "*Chase/Aim Combo Cam*"
	virtual CAMTYPE     GetType()       const  {return CT_CHASEAIMCOMBO;}

	void SetPitch(float fPitch);

	//void ActivateChase() {m_eMode = CHASE;}
	//void ActivateAim()   {m_eMode = AIM;}

	// Return the mode for the chase cam
	MODE GetMode(void) const { return m_eMode; }

	// Access  to Child Cameras ( req in Aiming controller )
	CoolCam_Chase* GetChaseCam() { return m_pChaseCam; };
	CoolCam_Aim*   GetAimCam()   { return m_pAimCam; };

	// Has the camera transitioned in yet?
	bool IsTransitionedIn		()			const	{ return m_fTransitionTime >= m_fTotalTransitionTimeIn; }
	bool IsTransitionAt			(float fP)	const	{ return m_fTotalTransitionTimeIn > 0.0f ? (m_fTransitionTime / m_fTotalTransitionTimeIn) >= fP : false; }


#ifndef _RELEASE
	virtual void RenderDebugInfo() {;}
#endif

// Members
protected:
	// Target Entity
	const CEntity* m_pEntity;

	// Child Cameras
	CoolCam_Chase* m_pChaseCam;
	CoolCam_Aim*   m_pAimCam;

	MODE m_eMode;
	bool m_bActive;

	// Transition data
	float m_fTransitionTime;

	float m_fTotalTransitionTimeIn;
	float m_fTotalTransitionTimeOut;
	float m_fInOutRatio;
};

#endif //_COOLCAM_CHASEAIMCOMBO_H
