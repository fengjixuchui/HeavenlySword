/***************************************************************************************************
*
*	CLASS			CCombatCamDef
*
*	DESCRIPTION		Definition class for combat camera parameters
*
***************************************************************************************************/

#ifndef _COMBAT_CAM_DEF_H
#define _COMBAT_CAM_DEF_H

class CCombatCamDef
{
public:
	CCombatCamDef();
	virtual ~CCombatCamDef(){};

	virtual bool EditorChangeValue(CallBackParameter, CallBackParameter);
	virtual void PostConstruct();

	CDirection m_ptOffset;

	float m_fZoomHoldTime;
    float m_fZoomInSpeed;
	float m_fZoomOutSpeed;
	float m_fZoomSpring;
	float m_fZoomDamp;

	float m_fFoV;
	float m_fFoVBlendTime;

	float m_fIdealInterestingRatio;
	float m_fMaxInterestingRatio;
	float m_fMinInterestingRatio;

	float m_fRotateUpSpeed;
	float m_fRotateDownSpeed;
	float m_fRotateSpring;
	float m_fRotateDamp;
	float m_fMaxRotation;
	float m_fRotationHoldTime;

	float m_fPOILimit;
	bool  m_bAutoActivate;
	bool  m_bCanZoomInCombat;
private:
	float m_fFovExternal;
	HAS_INTERFACE(CCombatCamDef)
};

#endif //_COMBAT_CAM_DEF_H
