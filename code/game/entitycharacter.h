//--------------------------------------------------
//!
//!	\file game/entitycharacter.h
//!	Definition of the Character entity object
//!
//--------------------------------------------------

#ifndef	_ENTITY_CHARACTER_H
#define	_ENTITY_CHARACTER_H

#include "game/entity.h"
#include "game/interactiontarget.h"

class WeaponSetDef;
class EyeBlinker;

//--------------------------------------------------
//!
//! Class Character
//! Base character entity type
//!
//--------------------------------------------------
class Character : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Character)

public:
	Character();	
	~Character();

	// From CEntity
	virtual void OnLevelStart();

	void OnPostConstruct();

	// Overidden Virtuals
	virtual void Show();
	
	// Enum for the type of the character
	enum CharacterType
	{
		CT_Invalid	= 0x0000,
		CT_Player	= 0x0001,
		CT_AI		= 0x0002,
		CT_Boss	= 0x0004
	};

	// CharacterType accessor
	CharacterType GetCharacterType() const
	{
		//ntError_p((m_eCharacterType!=Invalid), ("CharacterType has not been set for this entity"))
		return m_eCharacterType;
	}

	// Components
	COMPONENT_ACCESS( CHierarchy,			GetRagdollHierarchy,		m_pobRagdollHierarchy );
	//COMPONENT_ACCESS_IMPLEMENTED(CollisionSphereSet)
	COMPONENT_ACCESS_IMPLEMENTED(ArtificialWind)
	COMPONENT_ACCESS_IMPLEMENTED(CollisionSword)
	COMPONENT_ACCESS_IMPLEMENTED(CollisionFloor)
	COMPONENT_ACCESS_IMPLEMENTED(EyeBlinker)

	// Special case - The hero can have two collision sphere sets...
	virtual void SetCollisionSphereSet(CollisionSphereSet *pobCollisionSphereSet) 
	{
		if (m_pobCollisionSphereSet1 == 0)
		{
			m_pobCollisionSphereSet1 = pobCollisionSphereSet;
		}
		else
		{
			m_pobCollisionSphereSet2 = pobCollisionSphereSet;
		}
	}

	// Interaction Target Accessors
	void     	SetInteractionTarget(CEntity* pEnt) 								{m_obInteractionTarget.m_pobInteractingEnt = pEnt; m_obInteractionTarget.m_pobClosestUsePoint = 0;}
	void     	SetInteractionTargetP(CEntity* pEnt) 								{m_obInteractionTarget.m_pobInteractingEnt = pEnt; m_obInteractionTarget.m_pobClosestUsePoint = 0;}
	void     	SetInteractionTarget(const CInteractionTarget& obInteractionTarg) 	{m_obInteractionTarget = obInteractionTarg;}
	CEntity* 	GetInteractionTarget() const        								{return m_obInteractionTarget.m_pobInteractingEnt;}
	CUsePoint* 	GetInteractionTargetUsePoint() const        						{return m_obInteractionTarget.m_pobClosestUsePoint;}

	// Other Character Accessors
	void     SetOtherCharacter(Character* pEnt) {m_pOther = pEnt;}
	Character* GetOtherCharacter() const        {return m_pOther;}

	// Weapon Methods
	void RemoveWeapon(CEntity* pWeapon, bool bPush = false);
	void DropWeapons();
	void DropRangedWeapon();
	void HideWeapons();
	void ShowWeapons();
	void SheathWeapons();
	void DrawWeapons();


	const WeaponSetDef* GetWeaponsDef() const				{return m_pWeaponsDef;}
	CEntity*            GetLeftWeapon() const				{return m_pLeftWeapon;}
	CEntity*            GetRightWeapon() const				{return m_pRightWeapon;}
	CEntity*            GetRangedWeapon() const				{return m_pRangedWeapon;}
	void                SetLeftWeapon(CEntity* pWeapon)		{m_pLeftWeapon = pWeapon;}
	void                SetRightWeapon(CEntity* pWeapon)	{m_pRightWeapon = pWeapon;}
	void                SetRangedWeapon(CEntity* pWeapon)	{m_pRangedWeapon = pWeapon;}


	// Exit the interacting state?
	bool     ExitOnMovementDone() {return m_bExitOnMovementDone;}
	void     SetExitOnMovementDone(bool b) {m_bExitOnMovementDone = b;}

	// Helper functions
	NinjaLua::LuaFunction HelperGetLuaFunc(CHashedString obFunctionName);
	NinjaLua::LuaFunction HelperGetLuaFunc(CHashedString obTableName, CHashedString obFunctionName);

	// Importance (if yes it will be not passed through some invisible walls)
	bool IsImportant() const {return m_bImportant;};

	// Return the description for the entity
	const ntstd::String& GetDescription(void) const { return m_obDescription; }

	// Return the description for the entity
	const ntstd::String& GetIgnoredInteractions(void) const { return m_obIgnoredInteractions; }

	//	scee.sbashow : Current simple routine which starts the nav anim off for navigating to a use point.
	bool SetOffNavigationToUsePoint(CharacterType eType, bool bWantsToRun);

	// External Contro State Accessors
	virtual void SetExternalControlState ( bool b )		{ bIsInExternalControlState = b; }
	virtual bool GetExternalControlState ( void ) const { return bIsInExternalControlState; }

	// ---- Remove from world - if possible ------ 
	virtual bool CanRemoveFromWorld(void);
	virtual bool RemoveFromWorld(bool bSafeRemove = true);

	void SetKillInNextFrame(bool kill) {m_bKillInNextFrame = kill;};
	bool GetKillInNextFrame() const {return m_bKillInNextFrame;};

	// Method that allows a character to modifiy it's holding physics shape. 
	virtual void ModifyPhysicsHoldingShape( CPoint& rptA, CPoint& rptB, float& fRadius ) { UNUSED( rptA ); UNUSED( rptB ); UNUSED( fRadius ); }

	bool GetCanUseCheapCC() { return m_bCanUseCheapCC; };

// Helper Methods
protected:
	void ConstructDynamicsState();

// External Contro State flag
protected:
	bool bIsInExternalControlState;

protected:
	
	// Description of the entity
	ntstd::String	m_obDescription;

	// Ignored interactions for this entity
	ntstd::String	m_obIgnoredInteractions;

	// Position
	CPoint m_obPosition;

	// Orientation
	CQuat m_obOrientation;

	// Held position
	CPoint m_obHeldPosition;

	// Held orientation
	CQuat m_obHeldOrientation;

	// Holding position
	CPoint m_obHoldingPosition;

	// Holding orientation
	CQuat m_obHoldingOrientation;

	// The Ragdoll Clump
	CClumpHeader*			m_pobRagdollClumpHeader;
	CHierarchy*				m_pobRagdollHierarchy;
	ntstd::String			m_RagdollClump;

	// BSClump
	ntstd::String m_obBSClump;
	// Blendshape animation container
	ntstd::String m_obBSAnimContainer;

	// Controlled by AI?
	bool m_bAIControlled;

	// Collision height
	float m_fCollisionHeight;

	// Collision radius
	float m_fCollisionRadius;

	// Health value
	int32_t	m_iHealth;

	// Reaction matrix
	CHashedString m_obReactionMatrix;

	// Life clock worth
	float m_fLifeClockWorth;

	// The audio radius for this entity - if the listener is outside this radius, the entity will not emit sounds
	float m_fAudioRadius;

	// Interaction Target (switches, throwables, ranged weapons etc.)
	CInteractionTarget m_obInteractionTarget;

	// The type of the character
	CharacterType m_eCharacterType;


	// Components
	ntstd::String m_sAnimationContainer;
	CHashedString m_sAttackDefinition;
	CHashedString m_sAwareDefinition;
	CHashedString m_sSceneElementDefinition;
	CHashedString m_sLookAtComponentDefinition;
	CHashedString m_sFootstepDefinition;
	CHashedString m_sEyeBlinkerDefinition;

	// Construction scripts
	ntstd::String m_sHairConstruction;

	// Other Character in Interactions...
	Character* m_pOther;

	// Weapons
	WeaponSetDef* m_pWeaponsDef;         // Definition
	CEntity*      m_pLeftWeapon;         // }
	CEntity*      m_pRightWeapon;        // } Weapons
	CEntity*      m_pRangedWeapon;       // } 
	CEntity*      m_pChainmanRenderable; // Make this nicer... Why should everyone have one of these?
	bool          m_bWeaponsHidden;

	// Is important (if yes it will be not passed through some invisible walls)
	bool m_bImportant; 

	// Character has to be killed in next frame. Probably because of thread safety he was not killed immediately
	bool m_bKillInNextFrame; 

	// Exit the interacting state when movement done?
	bool m_bExitOnMovementDone;

	friend class Weapons;

	// Components
	CollisionSphereSet* m_pobCollisionSphereSet1;
	CollisionSphereSet* m_pobCollisionSphereSet2;
	ArtificialWind*		m_pobArtificialWind;
	CollisionSword*		m_pobCollisionSword;
	CollisionFloor*		m_pobCollisionFloor;
	EyeBlinker*			m_pobEyeBlinker;

	// Flag for whether we can switch this guy into a cheaper character controller when it's not doing important combat stuff
	bool m_bCanUseCheapCC;

public:
	// TODO:MB
	// Transported from EntityInfo...

	// Management for the health of a character
	float	GetCurrHealth( void ) const		{ return m_fCurrHealth; }
	float	GetStartHealth( void ) const	{ return m_fStartHealth; }
	virtual bool	IsDead( void ) const;
	virtual void	ChangeHealth( float fDelta, const char* );
	void	SetHealth( float fHealth, const char* );
	void	SetHealthPerc( float fHealth );
	void	SetDead( bool bDead );

	virtual void	UpdateDerivedComponents(float fTimeStep);

	// Deadness Methods
	static void KillEnt(CEntity* pEnt);
	void Kill();

	// Helpful debug stuff for combat
	void DebugDisplayHealthHistory( float fTimeStep );
	void DebugUpdateHealthHistory( float fHealthChange, const char* );

	bool IsInvulnerable(void) const { return m_bIsInvulnerable; }
	void SetInvulnerable(bool bState) { m_bIsInvulnerable = bState; }

	float m_fDamage[3];
	const char* m_apcDamageReason[3];
	float m_fDamageDisplayTimer[3];
	int   m_iDamageDisplayCycle;

	// is this entity a super hero?
	bool			m_bIsInvulnerable;

	// Managing the health of the player
	float			m_fCurrHealth;
	float			m_fStartHealth;
	float			m_fLastHealth;

};

LV_DECLARE_USERDATA(Character);


#endif //_ENTITY_CHARACTER_H
