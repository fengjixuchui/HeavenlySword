//------------------------------------------------------------------------------------------
//!
//!	\file CamCool.h
//!
//------------------------------------------------------------------------------------------

#ifndef _CAMCOOL_H
#define _CAMCOOL_H


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camerainterface.h"
#include "camera/smoother.h"


//------------------------------------------------------------------------------------------
// External Classes
//------------------------------------------------------------------------------------------
class CEntity;
class CamView;
class CConverger;
class CPointConverger;
class Transform;
class CamTransitionDef;
class TimeScalarCurve;


//------------------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------------------
static const float ROTATING_CAM_TIME_SCALAR = 0.4f;
static const float AFTERTOUCH_CAM_FOV_MULTIPLIER = 1.5f;
static const float AFTERTOUCH_CAM_MIN_TIME_SCALAR = 0.05f; // Alterered temporarily for E3 shots
static const float AFTERTOUCH_CAM_MAX_TIME_SCALAR = 0.5f;
static const float AFTERTOUCH_CAM_TIME_SLOW_PERIOD = 1.0f;
static const float AFTERTOUCH_CAM_TIME_SPEED_UP_PERIOD = 3.0f;
static const float AFTERTOUCH_CAM_DEFAULT_ROLL_SPEED = 25.0f * DEG_TO_RAD_VALUE; // 45.0f * DEG_TO_RAD_VALUE;
static const float AFTERTOUCH_CAM_PITCH = PI / 16.0f;
static const float AFTERTOUCH_CAM_HIT_ZOOM = 3.0f;
static const float AFTERTOUCH_CAM_HIT_ZOOM_TIME  = 1.0f;
static const float AFTERTOUCH_CAM_HIT_ZOOM_SPEED = AFTERTOUCH_CAM_HIT_ZOOM / AFTERTOUCH_CAM_HIT_ZOOM_TIME;
static const float AFTERTOUCH_CAM_HIT_ZOOM_ACC   = 0.01f;
static const float AFTERTOUCH_CAM_HIT_PITCH = PI / 8.0f;
static const float AFTERTOUCH_CAM_HIT_FOV_MULTIPLIER = 1.5f;
static const float AFTERTOUCH_CAM_HIT_TIME_SCALAR = 0.05f;
static const float AFTERTOUCH_CAM_HIT_INTERP_TIME = 1.0f;
static const float AFTERTOUCH_CAM_HIT_ROTATE_MAX_ACC = 1.0f;
static const float AFTERTOUCH_CAM_Y_OFFSET = 0.25f;
static const float AFTERTOUCH_CAM_MAX_ROLL = 15.0f * DEG_TO_RAD_VALUE; // 25.0f * DEG_TO_RAD_VALUE;
static const float THROWCAM_VELOCITY_THRESHOLD = 2.5f; // M/S
static const float THROWCAM_RESUME_THRESHOLD   = 5.0f; // M/S


//------------------------------------------------------------------------------------------
//!
//!	CoolCamera
//!	The base class for cool cameras, includes functionality for time curves
//!
//------------------------------------------------------------------------------------------
class CoolCamera : public CameraInterface
{
public:
	CoolCamera(const CamView& view);
	virtual ~CoolCamera();

	// Update
	virtual void	   Update(float fTimeDelta) = 0;

	// ID & Priority
	virtual int        GetID() const       {return m_iID;}
	virtual int        GetPriority() const {return m_iPriority;}

	// Timing
	float              GetTimeScalar() const                {return m_fTimeScalar;}
	float              GetTime() const                      {return m_fTime;}
	float	           GetTimeRemaining() const	            {return m_fTotalTime - m_fTime;}

	// Life-cycle
	virtual bool       HasFinished() const                  {return m_fTime > m_fTotalTime || m_bFinished;} // Has the coolcam outlived it's usefulness?
	virtual bool       IsActive() const                     {return !HasFinished();}                        // Usually when a camera is inactive it is finished, not always necessarily true for all camera types though...  Finshed is a subset of inactive.
	virtual void       EndCamera()                          {m_bFinished = true;}

	void               SetEndingTransition(CamTransitionDef* pDef) {m_pEndTrans = pDef;}
	CamTransitionDef*  GetEndingTransition() const          {return m_pEndTrans;}
	void               RemovingFromView(bool b)             {m_bRemovingFromView = b;}      // Mark the camera as being removed from the view.  Delete as soon as it's not used in any transitions etc.
	bool               RemovingFromView()                   {return m_bRemovingFromView;}

	// Access the time curve. 
	const TimeScalarCurve*	GetTimeCurve() const { return m_pCurve; }

// Helper methods
//------------------
protected:
	virtual void	SetTimeCurve(TimeScalarCurve *pCurve);
	virtual void	UpdateTime(float fTimeDelta);
	virtual void	ResetTime(float fTotalTime, float fTimeIn = 0.0f);

// Members
//------------------
protected:
	float                      m_fTime;             // Time we've been running for with scaling
	float                      m_fTotalTime;        // Total time the move runs for. In real time
	float                      m_fRealTime;         // Time we've been running for with no scaling
	bool                       m_bFinished;         // Is this camera finished and ready to be removed?
	bool                       m_bRemovingFromView; // Are we in the process of removing the camera?
	mutable TimeScalarCurve*   m_pCurve;            // Slow down or speed up time.
	CamTransitionDef*          m_pEndTrans;         // A definition of the transition to play when the camera finishes

	float                      m_fTimeScalar;


// Meta Data
//------------------
private:
	// IDs
	int        m_iID;
	static int m_iNextID;

protected:
	int        m_iPriority;
};


//------------------------------------------------------------------------------------------
//!
//!	NullCam
//!	This camera just mimics the game camera, it's just used to get a tscurve on an attack
//! move.
//!
//------------------------------------------------------------------------------------------
class CoolCam_Null : public CoolCamera
{
public:
	CoolCam_Null(const CamView& view, TimeScalarCurve* pCurve, float fDuration);
	virtual ~CoolCam_Null() {;}

	void SetFinished();

	virtual void        Update(float fTimeDelta);
	virtual void        Reset() {;}
	virtual bool        IsActive() const    {return !m_bFinished;}
	virtual bool        HasFinished() const {return m_bFinished;}
	virtual void        EndCamera();
	virtual CHashedString GetCameraName() const {return CHashedString(HASH_STRING__NULL_CAM__);}	// *Null Cam!*
	virtual CAMTYPE     GetType() const       {return CT_NULL;}

#ifndef _RELEASE
	virtual void RenderDebugInfo() {;}
#endif
};

#endif //_CAMCOOL_H
