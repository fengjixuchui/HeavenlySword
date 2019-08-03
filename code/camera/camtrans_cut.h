//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file CamTrans_Cut.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef CAMTRANS_CUT_H
#define CAMTRANS_CUT_H

//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camtransition.h"


//------------------------------------------------------------------------------------------
// External/Forward Declarations
//------------------------------------------------------------------------------------------
class CamTransition;


//------------------------------------------------------------------------------------------
//!
//!	CamTrans_CutDef                                                                        
//!	Definition of a camera cut transition
//!
//------------------------------------------------------------------------------------------

class CamTrans_CutDef : public CamTransitionDef
{
	public:
		CamTrans_CutDef( void ) {}

		virtual CamTransition* Create(const CameraInterface* pSrc, const CameraInterface* pDst) const;
		virtual const char*	GetTransTypeName() const {return "Cut Transition";}

	protected:

	private:
		friend class CamTrans_CutDefInterface;

};

//------------------------------------------------------------------------------------------
//!
//!	CamTrans_Cut
//!	Simple switch to new matrix when triggered
//!
//------------------------------------------------------------------------------------------
class CamTrans_Cut : public CamTransition
{
public:
	CamTrans_Cut(const CameraInterface* pSrc, const CameraInterface* pDst, const CamTrans_CutDef* pDef)
		: CamTransition(pSrc, pDst)
	{
		m_fControlTransitionTotalTime = pDef->GetControlTransTotalTime();
	}

	virtual void Update(float fTimeDelta);
	virtual const char*	GetTypeName() const	         {return "TRANS_CUT";}

	// Members
	private:
};

#endif

