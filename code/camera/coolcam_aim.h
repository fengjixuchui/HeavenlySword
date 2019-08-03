//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Aim.h
//!
//------------------------------------------------------------------------------------------


#ifndef _COOLCAM_AIM_H
#define _COOLCAM_AIM_H

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Required Includes
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include "camera/camcool.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// External Classes
/////////////////////////////////////////////////////////////////////////////////////////////////////
class CEntity;
class TimeScalarCurve;


//------------------------------------------------------------------------------------------
//! 
//!	CoolCam_AimDef
//!	A definition for the aiming camera
//! 
//------------------------------------------------------------------------------------------
class CoolCam_AimDef 
{
	HAS_INTERFACE( CoolCam_AimDef );

public: 

	CoolCam_AimDef();

	// Offsets
	CDirection m_dOffset1;
	CDirection m_dOffset2;
	CDirection m_dTargetOffset;

	/// Normal FOV value
	float m_fFOV;
	float m_fFocalDepth;

	// Depth of Field Settings
	bool  m_bUseDoF;
	float m_fMaxFocalDist;
	float m_fNearOffset;
	float m_fFarOffset;
	float m_fLerpFactor;

	// Time scaling
	TimeScalarCurve* m_pTimeCurve;

	// Transition attributes
	float m_fTransitionTimeIn;
	float m_fTransitionTimeOut;
	
	// Upside down cam?
	bool m_bUpsideDownCam;
};

//------------------------------------------------------------------------------------------
//! 
//!	CoolCam_Aim
//!	A camera for precision aiming of ranged weaponry.  e.g. Spears or sniper bows.
//! 
//------------------------------------------------------------------------------------------
class CoolCam_Aim : public CoolCamera
{
public:
	CoolCam_Aim(const CamView& view, const CEntity& ent, CoolCam_AimDef& def);
	virtual ~CoolCam_Aim();

	virtual void          Update(float fTimeDelta);
	virtual void          Reset() {;}
	virtual bool          HasFinished() const   {return m_bFinished;}
	virtual CHashedString GetCameraName() const {return CHashedString(HASH_STRING__AIM_CAM_);} //  "*Aim Cam*"
	virtual CAMTYPE       GetType() const       {return CT_AIM;}

	void SetPitch(float fPitch)                 {m_fPitch = fPitch;}

	void ResumeCamera()                         {m_bFinished = false;}
	void SetFoV(float fFoV, float fTime)        {m_fFovBlendTotalTime = m_fFoVBlendTime = fTime; m_fFoVBlendTarget = fFoV; m_fFoVBlendOriginal = m_fFOV;}
	void RestoreFoV(float fTime)				{m_fFovBlendTotalTime = m_fFoVBlendTime = fTime; m_fFoVBlendTarget = m_fFoVBlendOriginal; }

	// Access the Camera def
	const CoolCam_AimDef& GetCamDef( void ) const { return m_def; }

#ifndef _RELEASE
	virtual void RenderDebugInfo() {;}
#endif

// Members
protected:
	const CoolCam_AimDef& m_def;
	const CEntity& m_parentEnt;

	float m_fPitch;
	float m_fFovBlendTotalTime;
	float m_fFoVBlendTime;
	float m_fFoVBlendTarget;
	float m_fFoVBlendOriginal;
};


#endif //_COOLCAM_AIM
