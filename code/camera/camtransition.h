//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camtransition.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef CAMTRANSITION_INC
#define CAMTRANSITION_INC


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camerainterface.h"
#include "editable/enumlist.h"


//------------------------------------------------------------------------------------------
// External/Forward Declarations
//------------------------------------------------------------------------------------------
class CamTransition;
class BasicCameraTemplate;

//------------------------------------------------------------------------------------------
//!
//!	CamTransitionDef                                                                        
//!	A Transition Definition
//!
//------------------------------------------------------------------------------------------
class CamTransitionDef
{
public:
	virtual ~CamTransitionDef()
	{
		ntPrintf("Cleaning");
	};

	// create the approriate transition from ourselves
	virtual CamTransition* Create(const CameraInterface* pSrc, const CameraInterface* pDst) const = 0;

	// Transition type and related debug information
	virtual const char*	GetTransTypeName() const	= 0;
	const BasicCameraTemplate* GetDestination()	const	{return m_pDst;}

	float GetControlTransTotalTime( void ) const { return m_fControlTransitionTotalTime; }
	void SetControlTransitionTime( float fCTTime ) {  m_fControlTransitionTotalTime = fCTTime;  }

	bool SetDestCamera(BasicCameraTemplate* pobDst);

protected:
	BasicCameraTemplate* m_pDst;
	float m_fControlTransitionTotalTime;
};



//------------------------------------------------------------------------------------------
//!
//!	CamTransition                                                                           
//!	Base class for all the real transitions
//!
//------------------------------------------------------------------------------------------
class CamTransition : public CameraInterface
{
public:
	CamTransition(const CameraInterface* pSrc, const CameraInterface* pDst)
		: CameraInterface(pSrc->GetView()), m_fControlTransitionTotalTime(2.0f),
		  m_fControlTransitionTime(0.0f)
	{
		m_pSrc = pSrc;
		m_pDst = pDst;
		m_bActive = true;
	}

	virtual ~CamTransition( void );

	void                   UpdateTransition(float fTimeDelta);

	virtual void           Update(float fTimeDelta)     = 0;
	virtual const char*	   GetTypeName() const	         {return "TRANS_UNNAMED";}

	virtual bool           IsValid(const CPoint&) const     {return m_bActive;}
	virtual bool	       IsActive() const                 {return m_bActive;}
	virtual CHashedString  GetCameraName() const;
	virtual CAMTYPE        GetType() const                  {return CT_TRANSITION;}
	virtual int            GetPriority() const;
	virtual float          GetTimeScalar() const;
	virtual bool           Is(CameraInterface* pCam) const  {return m_pSrc->Is(pCam) || m_pDst->Is(pCam);}

	const CameraInterface* GetDestination() const;

	virtual float			GetControlTransTotalTime( void ) const { return m_fControlTransitionTotalTime; }
	virtual float			GetControlTransTotal( void ) const	{ return m_fControlTransitionTime; }
	virtual CPoint			GetControlTransCamLookAt( void ) const { return m_obControlInterpLookatPoint; }

protected:
	void ControlTransitionUpdate( float fTimeDelta );

	bool m_bActive;
	const CameraInterface* m_pSrc;
	const CameraInterface* m_pDst;

	float m_fControlTransitionTotalTime;
	float m_fControlTransitionTime;

	CPoint m_obControlInterpLookatPoint;
};



class CamTrans_NullDef : public CamTransitionDef
{
public:
	virtual CamTransition* Create(const CameraInterface* pSrc, const CameraInterface* pDst) const;
	virtual const char*	GetTransTypeName() const {return "Null Transition";}
};


class CamTrans_Null : public CamTransition
{
public:
	CamTrans_Null(const CameraInterface* pSrc, const CameraInterface* pDst)
		: CamTransition(pSrc, pDst)
	{
	}

	virtual void Update(float fTimeDelta);
	virtual const char*	GetTypeName() const	         {return "TRANS_NULL";}
};


#endif // CAMTRANSITION_INC
