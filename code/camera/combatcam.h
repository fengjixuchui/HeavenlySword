//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file combatcam.h
//!                                                                                         
//------------------------------------------------------------------------------------------


#ifndef _COMBATCAM_INC
#define _COMBATCAM_INC

class CCombatCamDef;
class BasicCamera;
class CConverger;

class CombatCam
{
public:
	CombatCam(const BasicCamera* parent, CCombatCamDef& def);
	~CombatCam();

	float CalcZoom(float fTimeDelta, const CPoint& ptPosition, const CPoint&ptLookAt, float fFoV, float fMinRatio, float fMaxRatio);
	float UpdateFOV(float fExplorationFoV, float fTimeDelta);

	bool  AutoActivate() const;
	const CDirection& GetCombatOffset() const;
	float GetPOILimit() const;

// Helper Methods
private:
	float ZoomModifierConverger(float fOld, float fTarg, float fTimeDelta) const;
	float ApplyZoomConvergence(float fZoom, float fPlayerMaxZoom, float fTimeDelta);
// Members
private:
	CCombatCamDef&       m_def;
	const BasicCamera*   m_pParent;

	//
	CConverger* m_pobZoomConverger;
	float       m_fLastZoomModifier;
	float       m_fZoomHoldTimer;
	float       m_fFoVBlendTime;
	bool        m_bCombatAware;

};

#endif //_COMBATCAM_INC


