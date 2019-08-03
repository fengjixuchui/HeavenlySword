//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_AerialCombo.h
//!
//------------------------------------------------------------------------------------------


#ifndef	_CoolCam_AerialCombo_INC
#define	_CoolCam_AerialCombo_INC


//------------------------------------------------------------------------------------------
// Includes for Inheritance
//------------------------------------------------------------------------------------------
#include "camera/camcool.h"


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_AerialComboDef
//!	
//!
//------------------------------------------------------------------------------------------
class CoolCam_AerialComboDef
{
public:
	CoolCam_AerialComboDef() : dCameraOffset(-2.f, .5f, -2.5f), dPOIOffset(0.f, .5f, .1f) {;}

	void PostConstruct();
	bool EditValue(CallBackParameter, CallBackParameter);

public:
	CDirection dCameraOffset;
	CDirection dPOIOffset;
	float      fFOV;
	bool       bAttackerRelative;
};


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_AerialCombo
//!	
//!
//------------------------------------------------------------------------------------------
class CoolCam_AerialCombo : public CoolCamera
{
public:

	// Construction destruction
	CoolCam_AerialCombo(const CamView& view, const CEntity& obAttacker, const CEntity& obReceiver);
	virtual ~CoolCam_AerialCombo();

	// The stuff that this camera does...
	virtual void        Update(float fTimeDelta);
	virtual void        Reset() {}
	virtual CHashedString GetCameraName() const {return CHashedString(HASH_STRING__AERIAL_COMBO_CAM_); } //"*Aerial Combo Cam*"
	virtual CAMTYPE     GetType() const       {return CT_AERIAL;}

#ifndef _RELEASE
	virtual void RenderDebugInfo() {;}
#endif

protected:
	CPoint	GetPOI() const;

protected:
	// The two characters we are looking at
	const CEntity& m_obAttacker;
	const CEntity& m_obReceiver;

	// Smooth the camera
	CPointSmoother m_obSmoothPOI;
	CPointSmoother m_obSmoothPOS;

protected:
	// Weldered Parameters
	static CDirection m_dCameraOffset;
	static CDirection m_dPOIOffset;
	static float      m_fAerialFOV;
	static bool       m_bAttackerRelative;
	friend class CoolCam_AerialComboDef;
};




#endif
