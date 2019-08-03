/***************************************************************************************************
*
*	$Header:: /game/lenscontroller.h 2     13/08/03 14:40 Wil                                      $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef LENS_CONTROLLER_H
#define LENS_CONTROLLER_H


//------------------------------------------------------------------------------------------
// External declarations
//------------------------------------------------------------------------------------------
class BasicCamera;
class ConvergerDef;
class CConverger;
class CPointConverger;
class MotionController;
class LookAtController;
class CCombatCamDef;


//------------------------------------------------------------------------------------------
//!
//!	LensControllerDef
//!	Definition of a Lens Controller
//!
//------------------------------------------------------------------------------------------
class LensControllerDef
{
public:
	LensControllerDef() :
	  m_bFOVDynamicZoom(false),
	  m_fIdealFOV(35.f),
	  m_fIdealInterestingRatio(0.5f),
	  m_fMaxInterestingRatio(0.75f),
	  m_fMinInterestingRatio(0.25f)
	{
	}

	virtual ~LensControllerDef(){};
	virtual class LensController* Create(const BasicCamera* pParent);

public:
	bool			m_bFOVDynamicZoom;
	float			m_fIdealFOV;
	float			m_fIdealInterestingRatio;
	float			m_fMaxInterestingRatio;
	float			m_fMinInterestingRatio;
};


//------------------------------------------------------------------------------------------
//!
//!	LensController
//!	Controls our current FOV and DOF
//!
//------------------------------------------------------------------------------------------
class LensController
{
public:	
	LensController(const BasicCamera* pParent, const LensControllerDef& def);
	virtual ~LensController();

	void				Reset();
	float				Update(MotionController* pobMC, LookAtController* pobLAC, float fTimeDelta);
	void				SetCombatCamera(CCombatCamDef *pobCombatCamDef);
	void				SetIdealFOV(float f) {m_fIdealFOV = f;}

	void				Render() {};
#ifdef _DEBUG
	virtual void		RenderInfo(int iX, int iY);
#endif

	

protected:
	// Check to see if the player is visible on a ray
	bool IsPlayerVisible(const CPoint &obPos, const CPoint &obTarg);

private:
	HAS_INTERFACE(LensController);

	const BasicCamera*       m_pParent; // Where we come from.
	const LensControllerDef& m_def;    // Our serialised definition.

	// Convergers
	CPointConverger* m_pobPushConverger;
	CConverger*      m_pobFOVConverger;

	// Copied out of defintion
	float            m_fIdealFOV;

	CPoint			 m_obLastLook;
	float			 m_fMaxRotation;

	// Debug only
	CPoint			 m_obLOSStart;
	CPoint			 m_obLOSEnd;
};

#endif // LENS_CONTROLLER_H
