//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file motioncontroller.h                                                                
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef MOTION_CONTROLLER_INC
#define MOTION_CONTROLLER_INC

#ifndef _RELEASE
#define MCNTRLER_DEBUG
#endif

//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// External Declarations
//------------------------------------------------------------------------------------------
class MotionController;
class SmootherDef;
class ConvergerDef;
class CPointSmoother;
class PointTransformDef;
class PointTransform;
class CPointConverger;
class CConverger;
class CCurveRail;
class CurveRailDef;
class Transform;
class CameraInterface;
class CEntity;



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MotionControllerDef                                                                     
//!	Base defintion for motion controllers.                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
class MotionControllerDef
{
public:
	MotionControllerDef();
	virtual ~MotionControllerDef(){};

public:
	MotionController* Create(const CameraInterface* pParent, CEntity* pEntParent);

private:
	virtual class MotionController* Create(const CameraInterface* pParent) = 0;

public:
	PointTransformDef* m_pPOITransDef;
	SmootherDef*       m_pPOISmootherDef;
	ConvergerDef*      m_pPosConvergerDef;
	PointTransformDef* m_pBoundingTransDef;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MotionController                                                                        
//!	Abstract base class for motion controllers.                                             
//!                                                                                         
//------------------------------------------------------------------------------------------
class	MotionController
{
public:
	MotionController(const CameraInterface* pParent, const MotionControllerDef& def);
	virtual ~MotionController();

	//virtual void PostConstruct();

	// The entire public interface must be virtual to allow an editing wrapper with the same interface
	virtual CPoint		Update(float fTimeDelta) = 0;
	virtual void		Render();
	virtual void		RenderInfo(int iX, int iY);
	virtual void		Reset();
	virtual void		SetCombatCamera(class CCombatCamDef *pobCombatCamDef);

	virtual void		SetPushVec(const CDirection& obOffset)
	{
		if(m_bReqPush) // we've already had one push req this frame, add on the next
		{
			m_obPushOffset += obOffset;
		}
		else
		{
			m_bReqPush = true;
			m_obPushOffset = obOffset;
		}		
	}

	virtual void		SetRotation(const float fRotation) 
	{
		m_bRotation = true;
		m_fRotation = fRotation; 
	}

	virtual void MaintainRotation()	{m_bRotation = true;}

	virtual const CPoint&	GetLastTracked()      const {ntAssert(m_bLastTrackedValid);    return m_obLastTracked;}
	virtual const CPoint&	GetLastUnModified()   const {ntAssert(m_bLastUnmodifiedValid); return m_obLastUnModified;}
	virtual const CPoint&	GetLastPoint()        const {ntAssert(m_bLastPointValid);      return m_obLastPoint;}
	virtual const CPoint&	GetLastNoZoom()       const {ntAssert(m_bLastNoZoomValid);     return m_obLastNoZoom;}
	virtual const CPoint&	GetLastNoRotation()   const {ntAssert(m_bLastNoRotationValid); return m_obLastNoRotation;}
	virtual const CPoint&	GetLastPartRotation() const {ntAssert(m_bLastPointValid);      return m_obLastPartRotation;}

	virtual	float			GetPOIHeadingCompensateFactor()		  const	{return 0.0f;}
// Helper Functions
protected:
	void			CalcLastTracked(float fTimeChange);
	CPoint			ModifyPos(const CPoint& obPos, float fTimeChange);
	void			SetLastTracked(const CPoint& obPos)	{m_bLastTrackedValid = true; m_obLastTracked = obPos;}
	PointTransform*	GetPOITrans()						{return m_pPOITrans;}

// Members
protected:
	const CameraInterface* m_pParent;         // Remember where we came from.
	CEntity*               m_pParentEntity;

	CPointSmoother*	       m_pPOISmooth;    // POI smoother
	PointTransform*	       m_pPOITrans;     // POI transform
	CPointConverger*	   m_pPosConv;      // Final pos converger
	PointTransform*	       m_pFinalTrans;   // Final position transform (for clamping)
	CConverger*	           m_pRotationConv;

	bool	       	       m_bReqPush;        // has a push been requested
	CDirection	           m_obPushOffset;    // push amount

	bool	       	       m_bLastTrackedValid;
	bool	       	       m_bLastUnmodifiedValid;
	bool	       	       m_bLastPointValid;
	bool	       	       m_bLastNoZoomValid;
	bool	       	       m_bLastNoRotationValid;

	CPoint	       	       m_obLastTracked;      // last POI before virtual transforms..
	CPoint	       	       m_obLastUnModified;   // last Pos with no offset or convergence
	CPoint	       	       m_obLastPoint;        // last returned point
	CPoint	       	       m_obLastNoZoom;       // last pos with no offset or convergence but with rotation
	CPoint	       	       m_obLastNoRotation;   // last pos but with no rotation but with the zoom
	CPoint	       	       m_obLastPartRotation; // last Pos with only partial rotation (for visibility checks to see if we can return to unrotated state)

	float	       	       m_fRotation;          // Rotate the view if the player is obscured
	bool 	       	       m_bRotation;
	float	       	       m_fRotationHoldTime;  // Hold the rotation this long before returning to default
	float	       	       m_fRotationTimeOut;
	float	       	       m_fLastRotation;

	friend class           MotionControllerDef;
};

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCFixedPosDef                                                                           
//!	Definition of a MCFixedPos.                                                             
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCFixedPosDef : public MotionControllerDef
{
public:
	CPoint m_ptPosition;

	virtual class MotionController* Create(const CameraInterface* pParent);
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCFixedPos                                                                              
//!	This motion controller just sits at a fixed position.                                   
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCFixedPos : public MotionController
{
public:
	MCFixedPos(const CameraInterface* pParent, const MCFixedPosDef& def);

	virtual void		Reset()
	{
		MotionController::Reset();
		SetLastTracked(m_def.m_ptPosition);
	}

	virtual CPoint			Update(float fTimeChange);
	virtual void			Render();

private:
	const MCFixedPosDef& m_def;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCBoomDef                                                                               
//!	The defintion of a MCBoom.                                                              
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCBoomDef : public MotionControllerDef
{
public:
	CPoint		m_ptPosition;
	float		m_fDistance;
	float		m_fBoomHeight;
	bool		m_bBoomIgnoreY;
	bool        m_bBoomReverse;

	virtual class MotionController* Create(const CameraInterface* pParent);
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCBoom                                                                                  
//!	This motion controller rotates about a fixed point.    
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCBoom : public MotionController
{
public:
	MCBoom(const CameraInterface* pParent, const MCBoomDef& def);	

	virtual CPoint			Update(float fTimeChange);
	virtual void			Render();
	
protected:
	const MCBoomDef& m_def;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCPOIRelDef                                                                             
//!	The defintion of a MCPOIRel.                                                            
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCPOIRelDef : public MotionControllerDef
{
public:
	CDirection	m_direction;

	virtual class MotionController* Create(const CameraInterface* pParent);
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCPOIRel                                                                                
//!	This motion controller follows the camera staying at a fixed vector offset from the POI.
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCPOIRel : public MotionController
{
public:
	MCPOIRel(const CameraInterface* pParent, const MCPOIRelDef& def);

	virtual CPoint			Update(float fTimeChange);
	virtual void			Render();
	
protected:
	const MCPOIRelDef& m_def;

	CDirection m_direction;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCPOIChaseDef                                                                           
//!	The definition of a MCPOIChase.                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCPOIChaseDef : public MotionControllerDef
{
public:
	MCPOIChaseDef()
	{
		m_fMaxDistance = 5.f; m_fMinDistance = 4.f;
		m_fStartAngle = 10.f; m_fMoveSpeed = 180.f;
	}

	float		m_fMaxDistance;
	float		m_fMinDistance;
	float		m_fStartAngle;
	float		m_fMoveSpeed;

	virtual class MotionController* Create(const CameraInterface* pParent);
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCPOIChase                                                                              
//!	This motion controller chases the POI and can change angle.                             
//! It also doubles up as debug MC, really there should be a separate debug MC              
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCPOIChase : public MotionController
{
public:
	MCPOIChase(const CameraInterface* pParent, const MCPOIChaseDef& def);
	
	virtual void   Reset();
	virtual CPoint Update(float fTimeChange);
	virtual void   Render();

private:
	const MCPOIChaseDef& m_def;

	float                m_fLongditude;
	float                m_fLatitude;
	bool                 m_bInvertX;
	bool                 m_bInvertY;

//-------------------------------------------------------------
// Debug Cam Extras
//-------------------------------------------------------------
public:
	void           SetFixedMode(bool b)     {m_bFixed = b;}
	void           SetEasyTweakMode(bool b) {m_bEasyTweak = b;}
	void           SetInvertX(bool b)       {m_bInvertX = b;}
	void           SetInvertY(bool b)       {m_bInvertY = b;}

private:
	bool m_bFixed;
	bool m_bEasyTweak;
	friend class DebugChaseCamera;
	CDirection m_dirLast;
}; 


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCPOIStrafeDef                                                                          
//!	The defintion of a MCPOIStrafe.                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCPOIStrafeDef : public MotionControllerDef
{
public:
	ConvergerDef* m_pobBaseConvergerDef;
	float		  m_fDistance;
	float		  m_fStartAngle;
	float		  m_fMoveSpeed;

	virtual class MotionController* Create(const CameraInterface* pParent);
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCPOIStrafe                                                                             
//!	This motion controller strafes about the POI                                            
//!                                                                                         
//------------------------------------------------------------------------------------------
class	MCPOIStrafe : public MotionController
{
public:
	MCPOIStrafe(const CameraInterface* pParent, const MCPOIStrafeDef& def);
	virtual ~MCPOIStrafe();

	//virtual void PostConstruct();
	
	virtual void			Reset();
	virtual CPoint			Update(float fTimeChange);
	virtual void			Render();

	void		SetInvertX(bool bTrue) {m_bInvertX = bTrue;}
	void		SetInvertY(bool bTrue) {m_bInvertY = bTrue;}

private:
	const MCPOIStrafeDef& m_def;
	// ----------------------------------
	CConverger*   m_pobLongConv;
	CConverger*   m_pobLatConv;
	float		  m_fLongditude;
	float		  m_fLatitude;
	bool		  m_bInvertX;
	bool		  m_bInvertY;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCRailNearFarDef                                                                        
//!	The defintion of a MCRailNearFar.                                                       
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCRailNearFarDef : public MotionControllerDef
{
public:
	CurveRailDef* m_pobRailDef;

	virtual class MotionController* Create(const CameraInterface* pParent);
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCRailNearFar                                                                           
//!	This motion controller stays as close/far as it can from the the POI tracking from a    
//! rail.                                                                                   
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCRailNearFar : public MotionController
{
public:
	MCRailNearFar(const CameraInterface* pParent, const MCRailNearFarDef& def);
	virtual ~MCRailNearFar();

	virtual CPoint			Update(float fTimeChange);
	virtual void			Render();
	virtual void			Reset();
	
private:
	const MCRailNearFarDef& m_def;
	CCurveRail* m_pCurveRail;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCRailGuideDef                                                                          
//!	The defintion of a MCRailGuide.                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCRailGuideDef : public MotionControllerDef
{
public:
	class CurveRailDef*	m_pobRailDef;
	bool				m_bOriginOffset;
	float				m_fPOIMoveFactor;

	virtual class MotionController* Create(const CameraInterface* pParent);
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	MCRailGuide                                                                             
//!	This motioncontroller applies the point transform as a guide to position itself on a    
//! rail.                                                                                   
//!                                                                                         
//------------------------------------------------------------------------------------------
class MCRailGuide : public MotionController
{
public:
	MCRailGuide(const CameraInterface* pParent, const MCRailGuideDef& def);
	virtual ~MCRailGuide();

	virtual CPoint			Update(float fTimeChange);
	virtual void			Render();
	virtual void			Reset();

	float	GetPOIHeadingCompensateFactor() const	{	return m_def.m_fPOIMoveFactor;	}
private:
	const MCRailGuideDef& m_def;
	// ----------------------------------
	float				m_fLastTarget;
	CCurveRail*			m_pobRail;

	friend class MCRailGuideI;
};

#endif // MOTION_CONTROLLER_INC
