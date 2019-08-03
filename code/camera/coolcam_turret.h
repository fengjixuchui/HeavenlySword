//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Turret.h
//!
//------------------------------------------------------------------------------------------


#ifndef _COOLCAM_TURRET_H
#define _COOLCAM_TURRET_H

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Required Includes
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include "camera/camcool.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// External Classes
/////////////////////////////////////////////////////////////////////////////////////////////////////
class CEntity;
class Transform;



//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Turret
//!	A camera to be used in vehicles/turrets, e.g. Ballistaes or cannons.
//!
//------------------------------------------------------------------------------------------
class CoolCam_Turret : public CoolCamera
{
public:
	CoolCam_Turret(const CamView& view);
	virtual ~CoolCam_Turret();

	void Init(CEntity* pTurret, CHashedString pcTransform, const CDirection& dOff);

	virtual void        Update(float fTimeDelta);
	virtual void        Reset() {;}
	virtual bool        HasFinished() const     {return m_bFinished;}
	virtual CHashedString GetCameraName() const {return CHashedString(HASH_STRING__TURRET_CAM_);}	//"*Turret Cam*"
	virtual CAMTYPE     GetType() const         {return CT_TURRET;}
	virtual bool        AllowShakes() const     {return true;}

#ifndef _RELEASE
	virtual void RenderDebugInfo() {;}
#endif

// Members
protected:
	Transform* m_pTurretTransform;
	CDirection  m_dOff;
};

#endif //_COOLCAM_TURRET_H
