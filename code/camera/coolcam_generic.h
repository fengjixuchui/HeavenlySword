//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Generic.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_CoolCam_Generic_INC
#define	_CoolCam_Generic_INC


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camcool.h"



//------------------------------------------------------------------------------------------
//!
//!	GenericCoolCamProps
//!
//! Generic Cool Camera Properties edited through XML
//!	
//------------------------------------------------------------------------------------------
class GenericCoolCamProps : public Singleton<GenericCoolCamProps>
{
public:
	float fFOV;
	float fMinElevation;
	float fMaxElevation;
	float fRangeElevation;
	float fAttackerPOIBias;
	float fPreferredAngle;
	bool  bEnableDoF;
	float fDoFNearOffset;
	float fDoFFarOffset;
	float fDoFNearMin;
	float fDOFMayaNear;
	float fDOFMayaFocal;
	float fDOFMayaFar;

// Welder Private Interface
private:
	GenericCoolCamProps() {bEnableDoF=false; fDoFNearMin=0.f;}

	void Init() {fFOV = fFOV_Editor;// * DEG_TO_RAD_VALUE;
				 fMinElevation = fMinElevation_Editor * DEG_TO_RAD_VALUE; 
				 fMaxElevation = fMaxElevation_Editor * DEG_TO_RAD_VALUE;
				 fRangeElevation = fMaxElevation - fMinElevation;
				 fPreferredAngle = fPreferredAngle_Editor * DEG_TO_RAD_VALUE;
				 ntAssert(fRangeElevation >= 0.f); fRangeElevation = ntstd::Min(fRangeElevation, 0.f);
				 ntAssert(fDoFNearMin >= 0.f); fDoFNearMin = ntstd::Min(fDoFNearMin, 0.f);}

	bool ReInit(CallBackParameter, CallBackParameter) {Init(); return true;}

	// Edited values
	float fFOV_Editor;
	float fMinElevation_Editor;
	float fMaxElevation_Editor;
	float fPreferredAngle_Editor;
	friend class GenericCoolCamPropsInterface;
};


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Generic
//!	This is the generic cool camera, it cuts to a view of the player affects time while a 
//! combat move is carried out and interpolates back to a default view.
//!
//------------------------------------------------------------------------------------------
class CoolCam_Generic : public CoolCamera
{
public:
	CoolCam_Generic(const CamView& view);
	CoolCam_Generic(const CamView& view, float fTime, const CEntity *pobAttacker, const CEntity *pobAttackee);
	virtual ~CoolCam_Generic() {;}

	bool			Init(float fTime, const CEntity* pAttacker, const CEntity* pAttackee);
	bool			Init(float fTime, const CEntity* pAttacker, const CEntity* pAttackee, float fRadians);

	virtual void        Update(float fTimeDelta);
	virtual void        Reset();
	virtual bool	    HasFinished() const;
	virtual CHashedString GetCameraName() const {return CHashedString(HASH_STRING__GENERIC_COOLCAM_);}  //"*Generic CoolCam*"
	virtual CAMTYPE     GetType() const       {return CT_ROTATING;}

#ifndef _RELEASE
	virtual void RenderDebugInfo();
#endif

	const CDirection& GetOffset() const       {return m_obOffset;}
	void  SetOffset(const CDirection& offset) {m_obOffset = offset; m_obSmoothPOS.Reset(); Update(0.0f);}
	const CEntity*	  GetAttacker() const     {return m_pobAttacker;}
	const CEntity*	  GetAttackee() const     {return m_pobAttackee;}

protected:
	CPoint			GetPOI();
	CPoint			GetPOS(const CPoint &obPOI);

	virtual bool	FindGoodRandomOffset();
	

private:
	const CEntity*	m_pobAttacker;
	const CEntity*	m_pobAttackee;
	CDirection	    m_obOffset;		// Direction from POI
	bool            m_bFirstFrame;

	CSmootherDef   m_obSmoothDef;
	CPointSmoother m_obSmoothPOI;    // Smooth the target POI, ragdolls are very wobbly
	CPointSmoother m_obSmoothPOS;

	//Rotating Cam Extras
	bool		    m_bRotating;
	float		    m_fRadians;

	// Scratch Data
	CPoint  m_ptAttacker;
	CPoint  m_ptAttackee;

// Debugging info
private:
	bool	   m_bOK[19];
	CDirection m_obDir[19];
};


#endif //CoolCam_Generic
