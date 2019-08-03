//--------------------------------------------------
//!
//!	\file game/entityinteractablecatapult.h
//!	Definition of the catapult entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_CATAPULT_H
#define	_ENTITY_CATAPULT_H

#include "game/entityinteractable.h"

#include "game/entitycatapultrock.h"

#include "game/catapultcontroller.h"
#include "movement.h"

#include "fsm.h"

class CAnimation;
class Character;
class Att_Catapult_Rock;
class Catapult_BreakySection;

#define CATAPULT_BREAKY_COUNT	(6)

#define CATAPULT_LEFT		0	//	clump_catapult_section_001 = Left side
#define CATAPULT_BASE		1	//	clump_catapult_section_002 = Base and wheels
#define CATAPULT_RIGHT		2	//	clump_catapult_section_003 = Right Side
#define CATAPULT_TOP		3	//	clump_catapult_section_004 = Top tusks
#define CATAPULT_CORE		4	//	clump_catapult_section_005 = Main Big Wheel and catapults
#define CATAPULT_TAIL		5	//	clump_catapult_section_006 = Catapult tail

class Att_User
{
	// Declare dataobject interface
	HAS_INTERFACE(Att_User)

public:
	CHashedString	m_obAnimationIdleLoop;
	CHashedString	m_obAnimationMoveLoop;
	CHashedString	m_obAnimationFire;
	CHashedString	m_obAnimationReload;
	CHashedString	m_obTransform;
	CPoint			m_obTranslation;

	Att_User()
	:	m_obAnimationIdleLoop ( "" )
	,	m_obAnimationMoveLoop ( "" )
	,	m_obAnimationFire ( "" )
	,	m_obAnimationReload ( "" )
	,	m_obTransform ( ""  )
	,	m_obTranslation( CONSTRUCT_CLEAR ) 
	{};
};

typedef ntstd::List<Att_User*, Mem::MC_ENTITY>	UserDefList;
typedef ntstd::List<Att_Ammo*, Mem::MC_ENTITY>	AmmoDefList;
typedef	ntstd::List<float, Mem::MC_ENTITY>		VolleyTimeList;


class Att_Catapult
{
	// Declare dataobject interface
	HAS_INTERFACE(Att_Catapult)

public:
	bool			m_bAIAvoid;						// Sets if the AI will try to avoid this object
	float			m_fAIAvoidRadius;				// Sets the radius that the AI will try to avoid the object by
	CHashedString	m_obMoveAnim;
	float			m_fMaxRotationPerSecond;
	float			m_fMaxNormalPerSecond;
	float			m_fMaxAcceleration;
	float			m_fMaxHeightPerSecond;

	UserDefList		m_aobUserDefList;
	AmmoDefList		m_aobAmmoDefList;
	VolleyTimeList	m_aobVolleyTimeList;

	// These are each of the breaky sections of the catapult, they really should get names
	// that match their real world description.
	ntstd::String	m_strBreakySection1;	
	ntstd::String	m_strBreakySection2;
	ntstd::String	m_strBreakySection3;
	ntstd::String	m_strBreakySection4;
	ntstd::String	m_strBreakySection5;
	ntstd::String	m_strBreakySection6;

	int				m_iStrikesForBreakable;

	Att_Catapult_Rock*	m_pobAmmoAtts;

	Att_Catapult()
	:	m_obMoveAnim( "" )
	,	m_pobAmmoAtts ( 0 )
	{};
};

typedef AmmoDefList::iterator AmmoDefIter;
typedef UserDefList::iterator UserDefIter;
typedef ntstd::Vector<Character*>::iterator CharacterIter;
typedef ntstd::List<CPoint*>::iterator PointIter;
typedef ntstd::Vector<CEntity*>::iterator RockIter;

//--------------------------------------------------
//!
//! Class Object_Catapult.
//! Catapult entity type
//!
//--------------------------------------------------
class Object_Catapult : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_Catapult)

//BREAKYBITS - Begin
	// Sections of catapult cannon
	enum 
	{
		SECTION_LEFT	= 1 << CATAPULT_LEFT,
		SECTION_BASE	= 1 << CATAPULT_BASE,
		SECTION_RIGHT	= 1 << CATAPULT_RIGHT,
		SECTION_TOP		= 1 << CATAPULT_TOP,
		SECTION_CORE	= 1 << CATAPULT_CORE,
		SECTION_TAIL	= 1 << CATAPULT_TAIL,
	};
//BREAKYBITS - End

public:
	// Constructor
	Object_Catapult();

	// Destructor
	~Object_Catapult();

	// From CEntity
	virtual void OnLevelStart();

	// Post Construct
	void OnPostConstruct();
	void OnPostPostConstruct();

	bool GlobalHandlers(Message& msg);
	bool AttachedState(Message& msg);
	bool DefaultState(Message& msg);

	Att_Catapult* GetSharedAttributes() { return m_pobSharedAttributes; };
	CMovementInput* GetMovementInput() { return &m_obMovementInput; };

	bool FireProjectile( u_int iArmId );
	bool ResetArm( u_int iArmId );
	bool ReloadProjectile( u_int iArmId );


	// Some helpers for move and target points
	CPoint* CreatePoint( CPoint* pobPoint );
	CPoint* CreatePoint( CPoint& obPoint );
	void DeletePoint( CPoint* pobPoint );
	CPoint* GetPoint( const Message msg );

	//!	Helper to get armId
	bool GetArmID( const Message msg, u_int& iArmId );

	void GroundInfo( CPoint& obCoords, CPoint& obGround, CDirection& obNormal );

	void UpdateInput( void );

	void MoveSetup ( void );
	void IdleSetup ( void );

	// Attributes accessable to the FSM
	Character*		m_pOther;

	ntstd::Vector<Character*> m_aobUserList;
	ntstd::Vector<ARM_STATE> m_aobArmStateList;
	ntstd::Vector<CEntity*> m_aobRockList;

	u_int m_iNextArmReset;
	u_int m_iNextArmFire;

	ntstd::List<CPoint*> m_aobMoveList;
	ntstd::List<CPoint*> m_aobTargetList;

	bool m_bDoReload;

	float m_fMoveOnProximityThresholdSqu;

	float m_fStopProximityThresholdSqu;

	float m_fSpeed;

	//float m_fWarningTime;

	bool m_bUsePointActive;

	int	m_iProjCollisions;	//We count up a few collisions before the catapult is disabled and becomes breakable.

//BREAKYBITS
	// Used to signal the explosion of a break section, note that the section might not break
	// because it isn't possible yet. 
	void SignalBreakyExplosion( CEntity* pBreaky, u_int iSectionID );
	void ExplodeWholeCatapult();

	void BreakableSetup();

	void AllowCollisionWith(CEntity* pobRock, bool bAllow);

	// There are CATAPULT_BREAKY_COUNT breaky bits at the moment, here is a list of all of them
	Catapult_BreakySection*	m_apobBreakyBits[CATAPULT_BREAKY_COUNT];

	bool					m_bMoving;				// Currently moving so we know which breakable state to move to
//BREAKYBITS - End

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	CHashedString	m_InitialState;

	// The animation container
	CHashedString	m_AnimationContainer;	

	CMovementInput m_obMovementInput;

	Att_Catapult* m_pobSharedAttributes;

	// Cannon catapult - rtti
	bool			m_bCannonCatapult;

	//CHashedString m_obSharedAttributesName;


//BREAKY BITS
	// The breaky section mask is used to determine what sections can be broken off the
	// catapult. 
	u_int					m_BreakySectionMask;

	// Bits that mask out the breaky sections that have been destroyed
	u_int					m_BreakySectionDestroyed;

	bool					m_bDoneBreak;			// So we only do transition once
//BREAKY BITS - END
};


//!------------------------------------------------------------------------------
//!  Catapult_BreakySection
//!  Special class for each of the breaky sections of the catapult. This class 
//!  will forward any collision messages to the parent catapult asking whether
//!  it should break off.
//!  
//!  Base class Interactable 
//!
//!  @author GavB @date 13/10/2006
//!------------------------------------------------------------------------------
class Catapult_BreakySection : public Interactable
{
	// Declare dataobject interface
	HAS_INTERFACE(Catapult_BreakySection)

public:
	// Constructor
	Catapult_BreakySection();

	// Destructor
	~Catapult_BreakySection();

	// From CEntity
	virtual void OnLevelStart();

	// Access the breaky section ID
	int		BreakySectionId(void) const { return m_iBreakySectionId; }
	void	BreakySectionId(int iNewID) { m_iBreakySectionId = iNewID; }

	// Access the parent catapult
//	Object_Cannon_Catapult*		ParentCatapult(void) const					{ return m_pParentCatapult; }
//	void						ParentCatapult(Object_Cannon_Catapult* pOb)	{ m_pParentCatapult = pOb; }
	Object_Catapult*			ParentCatapult(void) const					{ return m_pParentCatapult; }
	void						ParentCatapult(Object_Catapult* pOb)		{ m_pParentCatapult = pOb; }

	// Set the animation container
	void SetAnimationContainer( const CHashedString& rAnimContainer ) { m_AnimationContainer = rAnimContainer; }

	// Post Construct
	void OnPostConstruct();
	void OnPostPostConstruct();

private:

	// Pointer to the parent catapult
//	Object_Cannon_Catapult* m_pParentCatapult;
	Object_Catapult* m_pParentCatapult;

	// Id for the breaky section
	int				m_iBreakySectionId;

	// The animation container
	CHashedString	m_AnimationContainer;	
};

#endif // _ENTITY_CATAPULT_H
