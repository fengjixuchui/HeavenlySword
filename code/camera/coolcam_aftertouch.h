//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_AfterTouch.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_CoolCam_AfterTouch_INC
#define	_CoolCam_AfterTouch_INC


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camcool.h"


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_AfterTouchDef
//!	The definition for an aftertouch camera.
//!
//------------------------------------------------------------------------------------------
class CoolCam_AfterTouchDef
{
public:

	CoolCam_AfterTouchDef () :
		m_fRollSpeed_Editor(25.0f),
		m_fPitch_Editor(11.25f),
		m_fMaxRoll_Editor(15.0f),
		m_fFOV_Multiplier(1.5f),
		m_fTimeScalar_Min(0.05f),
		m_fTimeScalar_Max(0.5f),
		m_fTimeScalar_SlowPeriod(1.0f),
		m_fTimeScalar_SpeedUpPeriod(3.0f),
		m_fPitch(0.f),
		m_dirLookAtOffset(0.f, 0.25f, 0.f),
		m_dirPositionOffset(0.f, 0.25f, 0.f),
		m_bLookAtOffsetEntityRel(false),
		m_fRollSpeed(0.f),
		m_fMaxRoll(0.f),
		m_fVelocityThreshold(2.5f),
		m_fResumeThreshold(5.0f),
		m_fHaltTime(0.1f),
		m_fNormalHitHoldTime(0.5f),
		m_fEnemyHitHoldTime(0.1f),
		m_fReboundHitHoldTime(0.15f),
		m_fNormalHitTimeScalar(-1.f),
		m_fEnemyHitTimeScalar(0.1f),
		m_fReboundlHitTimeScalar(0.2f),
		m_fHitInterpTime(.2f),
		m_fEndInterpTime(1.f),
		m_fEndInterpHoldTime(0.2f),
		m_bEntityTimeScaleAdjust( false ),
		m_fEntityTimeScaleSweep( 20.0f ),
		m_fEntityTimeScaleRange( 30.0f ),
		m_fEntityTimeScaleMin( 0.05f ),
		m_fEntityTimeScaleMax( 0.5f ),
		m_fEntityTimeScaleAcc( 0.01f ),
		m_fEntityTimeScaleScoreModifier( 1.0f / 300.0f ),
		m_fEntityTimeScaleScoreSmoother( 2.0f )
	{
	}

// Editing Interface - Convert degrees to radians...
private:
	void Init() {m_fRollSpeed = m_fRollSpeed_Editor * DEG_TO_RAD_VALUE;
				 m_fPitch     = m_fPitch_Editor     * DEG_TO_RAD_VALUE;
				 m_fMaxRoll   = m_fMaxRoll_Editor   * DEG_TO_RAD_VALUE;}

	bool ReInit(CallBackParameter, CallBackParameter) {Init(); return true;}

	float m_fRollSpeed_Editor;
	float m_fPitch_Editor;
	float m_fMaxRoll_Editor;
	friend class CoolCam_AfterTouchDefInterface;


// Public Interface.
public:
	float      m_fFOV_Multiplier;           // We change the FOV from the old cameras FOV by this multiplier...  May change this to a fixed value...
	float      m_fTimeScalar_Min;           // Timescalar for the initial slow time speed.
	float      m_fTimeScalar_Max;           // Timescalar for the final normal time speed.
	float      m_fTimeScalar_SlowPeriod;    // Time the camera is in slow time speed.
	float      m_fTimeScalar_SpeedUpPeriod; // Duration of the blend from slow to normal time speed.
	float      m_fPitch;                    // Initial camera pitch.
	CDirection m_dirLookAtOffset;           // Offset the point that the camera looks at.
	CDirection m_dirPositionOffset;         // Offset the position of the camera.
	bool       m_bLookAtOffsetEntityRel;    // Do we offset the lookat position relative to the entitys transform or direction of travel.
	float      m_fRollSpeed;                // Speed of camera roll.
	float      m_fMaxRoll;                  // Maximum camera roll.
	float      m_fVelocityThreshold;        // Halt the camera if the velocity falls below this value.
	float      m_fResumeThreshold;          // Resume the camera if the velocity goes back above this value.
	float      m_fHaltTime;					// The period of time to hold the camera after the camera comes to rest.
	float      m_fNormalHitHoldTime;		// * Hold the camera after it hits walls etc.
	float      m_fEnemyHitHoldTime;         // * Hold the camera after it hits an enemy.
	float      m_fReboundHitHoldTime;       // * Hold the camera after it rebounds.
	float      m_fNormalHitTimeScalar;      // - Time scalar for normal hit holds
	float      m_fEnemyHitTimeScalar;       // - Time scalar for enemy hit holds
	float      m_fReboundlHitTimeScalar;    // - Time scalar for rebound hit holds
	float      m_fNormalHitZoomOutDist;     // * Zoom out distance for normal hit holds
	float      m_fEnemyHitZoomOutDist;      // * Zoom out distance for enemy hit holds
	float      m_fReboundlHitZoomOutDist;   // * Zoom out distance for rebound hit holds
	float      m_fHitInterpTime;		    // Interpolate back into the aftertouch camera after it hits.
	float      m_fEndInterpTime;            // 
	float      m_fEndInterpHoldTime;        // 

	float      m_fStartingBlur;             // Initial Motion Blur Factor
	float      m_fNormalBlur;               // Standard Motion Blur Factor
	float      m_fBlurUpTime;               // Blurring Fade In Time
	float      m_fBlurDownTime;             // Blurring Fade Out Time


	bool		m_bEntityTimeScaleAdjust;		// Can this camera adjust the time scalar by the position of entities on the screen?
	float		m_fEntityTimeScaleSweep;		// Angle sweep for the cone looking for entities.
	float		m_fEntityTimeScaleRange;		// Range of the cone to sweep for entities.
	float		m_fEntityTimeScaleMin;			// Clamp the time scalar
	float		m_fEntityTimeScaleMax;			// Clamp the time scalar
	float		m_fEntityTimeScaleAcc;			// Accel
	float		m_fEntityTimeScaleScoreModifier;// Modify the score into a time scalar
	float		m_fEntityTimeScaleScoreSmoother;// Allow an amount of smoothing baby
};



//------------------------------------------------------------------------------------------
//!
//!	CoolCam_AfterTouch
//!	This is the camera which tracks thrown objects
//!
//------------------------------------------------------------------------------------------
class CoolCam_AfterTouch : public CoolCamera
{
public:
	CoolCam_AfterTouch(const CamView& view, const CoolCam_AfterTouchDef* pDef, CEntity* pentTarget, const CDirection& dirVel);
	virtual ~CoolCam_AfterTouch();

	virtual void          Update(float fTimeDelta);
	virtual bool          HasFinished()   const;
	virtual CHashedString GetCameraName() const                {return CHashedString(HASH_STRING__AFTERTOUCH_CAM_);}	// "*Aftertouch Cam*"
	virtual CAMTYPE       GetType()       const                {return CT_AFTERTOUCH;}
	virtual void          UpdateTime(float fTimeDelta);
	virtual void          EndCamera();

	// Look at modifiers when the object hits something
	enum HIT_TYPE {HT_NORMAL = 0, HT_ENEMY = 1, HT_REBOUND = 2};
	void LookAt(CEntity* pEnt, HIT_TYPE eHitType);
	void ActivateEndingCloseUp(CPoint& pt);

	// Rolling controls
	void SetRoll(float fRollTarget) {m_fRollTarget = ntstd::Clamp(fRollTarget, -1.0f, 1.0f);}
	void SetRollSpeed(float fRollSpeed) {m_fRollSpeed = fRollSpeed;}

#ifndef _RELEASE
	virtual void RenderDebugInfo();
#endif

private:
	bool Init(CEntity* pobEntity, const CDirection& obVel);
	void CleanUp();

protected:
	const CoolCam_AfterTouchDef* m_pDef;
	CEntity*		 m_pobEntity;		// The entity being chucked about
	CPoint			 m_ptLastPos;		// The last camera position
	float			 m_fDist;			// The distance to keep from the POI
	CDirection		 m_obLastVel;		// Previous velocity of the entity
	CPointSmoother*	 m_pobSmoothPOI;    // Smooth the target POI, ragdolls are very wobbly

	float			 m_fLastX;
	float			 m_fLastY;
	float			 m_fLastdY;

	// An Entity we collided with
	CEntity*		 m_pHitEnt;
	float            m_fHitTime;
	float            m_fHitZoomOutDist;
	float            m_fHitZoomOutTime;
	float            m_fMinTimeScalar;
	HIT_TYPE         m_eHitType;
	CPoint           m_ptHitLookAt;
	CPoint           m_ptHitCamPos;
	bool             m_bCloseUp;
	CPoint           m_ptCloseUpPos;
	CPoint           m_ptCloseUpTarg;

	// Has the camera come to rest?
	bool             m_bHalted;
	bool			 m_bCanBeResumed;
	float            m_fHaltedTime;

	// Camera Rolls
	float		m_fRollTarget;
	float		m_fRollSpeed;
	float		m_fRoll;

	// Remember the original FOV
	float		m_fOrigFOV;

	// Allow the time scalar to hold acceleration info... 
	float		m_fTimeScalarScalar;
};

#endif
