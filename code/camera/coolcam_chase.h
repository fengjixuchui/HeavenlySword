//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Chase.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_CoolCam_Chase_INC
#define	_CoolCam_Chase_INC


/////////////////////////////////
// Required Includes
/////////////////////////////////
#include "camera/camcool.h"


/////////////////////////////////
// External Decls.
/////////////////////////////////
class CPointSmoother;
class CConverger;
class Transform;
class TimeScalarCurve;


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_ChaseDef
//!	Definition for a chasing camera.
//!
//------------------------------------------------------------------------------------------
class CoolCam_ChaseDef
{
public:
	HAS_INTERFACE(CoolCam_ChaseDef);

	CoolCam_ChaseDef() : 
		dCameraOffset(0.f, 0.4f, -2.f), 
		dLookAtOffset(-0.5f, 0.f, 0.f), 
		fMaxAngle(100.f), 
		fFOV(40.0f), 
		bEnabled(true), 
		pTimeCurve(0), 
		m_fInitAngleCheck(0.9f),
		m_fInitRangeCheck(4.0f){}

	CDirection			dCameraOffset;
	CDirection			dLookAtOffset;
	float				fMaxAngle;
	float				fFOV;
	bool				bEnabled;
	TimeScalarCurve*	pTimeCurve;

	// Values that determine if the camera blends from the game camera
	float				m_fInitAngleCheck;
	float				m_fInitRangeCheck;
};


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Chase
//!	A chasing camera to follow the player while wielding a ranged weapon.
//!
//------------------------------------------------------------------------------------------
class CoolCam_Chase : public CoolCamera
{
public:
	CoolCam_Chase(const CamView& view, const CEntity& entity, const CoolCam_ChaseDef& def);
	virtual ~CoolCam_Chase();

	void Init();

	// Virtuals
	virtual void        Update(float fTimeDelta);
	virtual void        Reset() {;}
	virtual CHashedString GetCameraName() const {return CHashedString(HASH_STRING__CHASE_CAM_);} // *Chase Cam*
	virtual CAMTYPE     GetType() const       {return CT_CHASE;}
	bool				Enabled(void) const		{return m_def.bEnabled; }

#ifndef _RELEASE
	virtual void RenderDebugInfo() {;}
#endif

// Helper Funcs
protected:
	CPoint CalcIdealLookAt() const;
	CPoint CalcIdealPos()    const;

// Members
protected:
	const CEntity*    m_pEntity;
	const Transform* m_pEntityRoot;

	const CoolCam_ChaseDef& m_def;

	float             m_fAngle;
	float             m_fMaxAngle;

	CPointSmoother*   m_pSmoother;
};



#endif //_CoolCam_Chase_INC
