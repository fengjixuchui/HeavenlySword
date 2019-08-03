//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file CamTrans_Lerp.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef CAMTRANS_LERP_INC
#define CAMTRANS_LERP_INC


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camtransition.h"


//------------------------------------------------------------------------------------------
// External/Forward Declarations
//------------------------------------------------------------------------------------------
class CamTransition;


//------------------------------------------------------------------------------------------
//!
//!	CamTrans_LerpDef                                                                        
//!	Definition of a simple lerp transition
//!
//------------------------------------------------------------------------------------------
class CamTrans_LerpDef : public CamTransitionDef
{
public:
	CamTrans_LerpDef(float fTime, float fHoldTime) {m_fTime = fTime; m_fHoldTime = fHoldTime;}

	virtual CamTransition* Create(const CameraInterface* pSrc, const CameraInterface* pDst) const;
	virtual const char*	GetTransTypeName() const {return "Sigmoid Lerp Transition";}

public:
	float GetTime() const {return m_fTime;}
	float GetHoldTime() const {return m_fHoldTime;}

protected:
	CamTrans_LerpDef() {m_fTime = 1.f; m_fHoldTime = 0.5f;}

private:
	float m_fTime;
	float m_fHoldTime;

	friend class CamTrans_LerpDefInterface;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamTrans_Lerp                                                                           
//!	A simple lerp transition                                                                
//!                                                                                         
//------------------------------------------------------------------------------------------
class CamTrans_Lerp : public CamTransition
{
public:
	CamTrans_Lerp(const CameraInterface* pSrc, const CameraInterface* pDst, const CamTrans_LerpDef* pDef)
		: CamTransition(pSrc, pDst)
	{
		m_fTime = 0.f;
		m_fHoldTime = pDef->GetHoldTime();
		m_fTransTime = pDef->GetTime();
		m_fTotalTime = m_fHoldTime + m_fTransTime;
		m_bStarted = false;
	}

	virtual void Update(float fTimeChange);
	virtual const char*	GetTypeName() const	         {return "TRANS_LERP";}

// Members
protected:
	float		m_fTime;
	float		m_fHoldTime;
	float       m_fTransTime;
	float		m_fTotalTime;
	bool		m_bStarted;
};


#endif // CAMTRANS_LERP_INC
