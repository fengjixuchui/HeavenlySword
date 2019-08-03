//--------------------------------------------------
//!
//!	\file game/entitymovingplatform.h
//!	Definition of the moving platform entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_MOVING_PLATFORM_H
#define	_ENTITY_MOVING_PLATFORM_H

#include "game/entityinteractable.h"

#include "fsm.h"

class Interactable_Breaky_CableCar;


//--------------------------------------------------
//!
//! Class Interactable_Moving_Platform.
//! Moving platform entity type (keyframed)
//!
//--------------------------------------------------
class Interactable_Moving_Platform : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Moving_Platform)

public:
	// Constructor
	Interactable_Moving_Platform();

	// Destructor
	~Interactable_Moving_Platform();

	// Post Construct
	void OnPostConstruct();

	//Functions
	void PlayAnim(CHashedString anim, float fPercentage = 0.0f);

	//Accessors.
	LuaAttributeTable* GetSharedAttributes(void) { return m_pSharedAttributes; }
	const CHashedString& GetAnimationContainer() const { return m_AnimationContainer; }

	// Public Variables
	Character*	m_pOther;
	bool		m_bAtStart;
	bool		m_bAtStartNext;
	bool		m_bPlayerInside;
	bool		m_bPlayerWait;
	bool		m_bActive;
	CKeyString	m_EnterAnimation;
	CKeyString	m_ExitAnimation;
	CHashedString	m_AnimationContainer;
	CVector		m_MoveToPositionIn;
	CQuat		m_MoveToRotationIn;
	CVector		m_MoveToPositionOut;
	CQuat		m_MoveToRotationOut;

	//The script to notify when a platform is done moving (so the script can un-parent etc)
	CEntity*	m_pobNotifyScriptEntity;
	bool		m_bNotifyMovementDone;
protected:
	// Object description.
	CKeyString	m_Description;
	
	LuaAttributeTable* m_pSharedAttributes;
};

//--------------------------------------------------
//!
//! Class Interactable_Breaky_CableCar_Panel
//!
//!
//!
//--------------------------------------------------
class Interactable_Breaky_CableCar_Panel : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Breaky_CableCar_Panel)

public:

	Interactable_Breaky_CableCar_Panel(void);

	virtual ~Interactable_Breaky_CableCar_Panel(void) {}
	virtual void OnPostConstruct();

	// Name of the parent - 
	CHashedString	m_hParentName;

	// Animations for the entity
	CHashedString	m_AnimationContainer;

	// Animations name for the break away part to the animation
	CHashedString	m_BreakAwayAnimationName;

	// Each of the panel states has a mesh
	CHashedString	m_PanelMesh1,m_PanelMesh2,m_PanelMesh3,m_PanelMesh4,m_PanelMesh4b;

	// Sound stuff
	CKeyString		m_obSoundBank;
	CKeyString		m_obSoundCueStage1, m_obSoundCueStage2;

	// Signal the hit from an arrow
	void SignalHit(void);

	// Allow control of the paused hit detection state. 
	void PauseHitDetection(bool bState) { m_bPauseHitDetection = bState; m_bNoToLocalPauseState = !bState; }

	// Cache a local copy of the parent, just to save me typing
	Interactable_Breaky_CableCar* m_pParentCableCar;

	// Number of times the breaky section has been hit
	int				m_iHitCount;

	// Breaky thresholds
	int				m_iInToState2, m_iInToState3, m_iInToState4;

	// When the counter reaches this value - then the hit detection is paused. 
	int				m_iPauseHitDetectionCount;

	// Pause the hit detection
	bool			m_bPauseHitDetection;

	// Disallow the local paused state
	bool			m_bNoToLocalPauseState;

	// Is the panel the front or the rearend
	bool			m_bIsFront;
};


//--------------------------------------------------
//!
//! Class Interactable_Breaky_CableCar.
//! Moving platform entity type (keyframed)
//!
//--------------------------------------------------
class Interactable_Breaky_CableCar : public Interactable_Moving_Platform
{
	// Declare dataobject interface
	HAS_INTERFACE(Interactable_Breaky_CableCar)

public:
	// Constructor
	Interactable_Breaky_CableCar();

	// Destructor
	virtual ~Interactable_Breaky_CableCar();

	// Post Construct
	virtual void OnPostConstruct();

	// Method the child panel's call when they're hit. 
	void ChildPanelHit( bool bFront );

protected:

	// Hit tally counter for the child panels
	int		m_iFrontPanelHitCounter, m_iRearPanelHitCounter;

	// The threshold into the paused state for each of the child panels.
	int		m_iFrontThresholdToPausedState;
	
	// The threshold to out of the paused state for each of the child panels.
	int		m_iFrontThresholdOutPausedState;

	// The threshold into the paused state for each of the child panels.
	int		m_iRearThresholdToPausedState;
	
	// The threshold to out of the paused state for each of the child panels.
	int		m_iRearThresholdOutPausedState;

};


#endif // _ENTITY_MOVING_PLATFORM_H
