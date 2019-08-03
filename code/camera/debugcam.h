//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file debugcam.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef DEBUGCAM_H
#define DEBUGCAM_H

//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/basiccamera.h"


//------------------------------------------------------------------------------------------
// External Declarations
//------------------------------------------------------------------------------------------
class MCPOIChase;
class LACPOIRel;
class LensController;


//------------------------------------------------------------------------------------------
//!
//!	DebugChaseCamera                                                                                 
//!	Camera used when there aren't any alternatives or in debug mode.
//!
//------------------------------------------------------------------------------------------
class DebugChaseCamera : public BasicCamera
{
public:
	DebugChaseCamera(const CamView& view, DebugCameraTemplate dct);
	virtual ~DebugChaseCamera();

	virtual void		Update(float fTimeDelta);
	virtual void		Render() {};
	virtual void		Reset();
	virtual bool		IsValid(const CPoint&) const {return true;}
	virtual bool		IsActive() const             {return true;}
	virtual CAMTYPE		GetType() const              {return CT_DEBUG;}
	virtual CHashedString GetCameraName() const      {return CHashedString(HASH_STRING_DEBUG_CHASE_CAM);}
	virtual int         GetPriority() const          {return 0;}
	virtual bool        AllowShakes() const          {return true;}

	void				Zoom(float f);
	void				SetZoom(float f);
	void				SetFOV(float fFOV);
	void				SetTopDownMode(bool b)		{m_bTopDown = b;}
	void				Fix(bool b);

private:
	bool				m_bTopDown;
	float				m_fTopDownDist;
};


#endif // DEBUGCAM_H
