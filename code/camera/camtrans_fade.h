//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camtrans_fade.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef CAMTRANS_FADE_INC
#define CAMTRANS_FADE_INC


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
//!	CamTrans_FadeDef                                                                        
//!	Definition of a fade in, fade out transition
//!
//------------------------------------------------------------------------------------------
class CamTrans_FadeDef : public CamTransitionDef
{
public:	
	virtual CamTransition* Create(const CameraInterface* pSrc, const CameraInterface* pDst) const;
	virtual const char*	GetTransTypeName() const {return "Fade Transition";}

public:
	uint32_t	GetColour()			const {return m_dwCol;}
	float		GetFadeUp()			const {return m_fFadeUp;}
	float		GetFadeDown()		const {return m_fFadeDown;}

protected:
	CamTrans_FadeDef() {;}

private:
	uint32_t	m_dwCol;
	float		m_fFadeDown;
	float		m_fFadeUp;

	friend class CamTrans_FadeDefInterface;
};


//------------------------------------------------------------------------------------------
//!
//!	CamTrans_Fade                                                                           
//!	A fade in, fade out transition
//!
//------------------------------------------------------------------------------------------
class CamTrans_Fade : public CamTransition
{
public:
	CamTrans_Fade(const CameraInterface* pSrc, const CameraInterface* pDst, const CamTrans_FadeDef* pDef)
		: CamTransition(pSrc, pDst),
		  m_def(*pDef) 
	{
		m_fTime = 0.0f;
	}
	
	virtual void Update(float fTimeChange);
	virtual const char*	GetTypeName() const	         {return "TRANS_FADE";}

	bool FadingDown() const {return (m_fTime < m_def.GetFadeDown());}
	bool FadingUp() const {return (!FadingDown()) && (m_fTime < (m_def.GetFadeDown() + m_def.GetFadeUp()));}
	bool Finished() const {return (!FadingDown()) && (!FadingUp());}

private:
	float	m_fTime;
	CamTrans_FadeDef m_def;
};


#endif // CAMTRANS_FADE_INC
