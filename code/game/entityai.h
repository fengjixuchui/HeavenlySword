//!----------------------------------------------------------------------------------------------
//!
//!	\file game/entityai.h
//!	Definition of the AI entity object
//!
//!----------------------------------------------------------------------------------------------

#ifndef	_ENTITY_AI_H
#define	_ENTITY_AI_H

#include "game/entitycharacter.h"

//!----------------------------------------------------------------------------------------------
//!  Forward Declarations
//!----------------------------------------------------------------------------------------------
class SpawnPool;
class ArmyRenderable;

//!----------------------------------------------------------------------------------------------
//!
//! EGotoStateAfterUnholster
//! Helper enum to decide which state to goto after
//! the weapon is unholstered.
//!
//!----------------------------------------------------------------------------------------------
enum EGOTOSTATEAFTERUNHOLSTER
{
	UNHOLSTER_GOTO_STATE_INTERACT,
	UNHOLSTER_GOTO_STATE_DEFAULT,
	UNHOLSTER_GOTO_STATE_REACT
};

//!----------------------------------------------------------------------------------------------
//!
//! Class AIFactScoreWeights.
//! 
//!
//!----------------------------------------------------------------------------------------------

class AIFactScoreWeights
{
	HAS_INTERFACE( AIFactScoreWeights )
public:
	
	// Constructor for setting defaults.
	AIFactScoreWeights();

	// Distance from the owner entity to the fact entity
	float		m_FactDistanceWeight;

	// Age of the fact
	float		m_FactAgeWeight;

	// Number of entities attacking the fact entity
	float		m_FactEntAttackedWeight;

	// Visible weights for the entity
	float		m_InnerRangeWeight;
	float		m_CloseRangeWeight;
	float		m_LineOfSightRangeWeight;
	float		m_MainRangeWeight;
	float		m_ShootRangeWeight;
	float		m_OtherPersonsProblemRangeWeight;
};



//!----------------------------------------------------------------------------------------------
//!
//! Class AI.
//! AI entity type
//!
//!----------------------------------------------------------------------------------------------
class AI : public Character
{
	// Declare dataobject interface
	HAS_INTERFACE(AI)

public:
	AI();
	~AI();

	// From CEntity
	virtual void OnLevelStart();

	void OnPostConstruct();
	void OnPostPostConstruct();

	/// Return the enemy state of this AI entity - relative to the players point of view
	bool IsEnemy() const  { return m_bIsEnemy; }

	// Should this entity attack other AI
	bool AttackAI(void) const { return m_bAttackAI; }

	bool           IsSpawned() const    {return m_pSpawnPool!=0;} // Is this entity owned by a spawn point?
	SpawnPool*     GetSpawnPool() const {return m_pSpawnPool;} 
	bool           Respawn(const ntstd::Vector<CEntity*>& vecWeapons);

	
	int BlockSkill(void) const { return m_BlockSkill; }
	int AttackSkill(void) const { return m_AttackSkill; }
	float BlockAdjust(void) const { return m_fBlockSkillAdjust; }
	float AttackAdjust(void) const { return m_fAttackSkillAdjust; }
	float GetVisionUpdateRate(void) const { return m_fVisionUpdateRate; }
	void  SetVisionUpdateRate(float fValue) { m_fVisionUpdateRate = fValue; }

	COMPONENT_ACCESS( CAIComponent,	GetAIComponent,	 m_pobAIComponent );
	

// Helper Methods
public:
	void Ragdoll_StartAimedThrow(CEntity* pController, CHashedString sParams, bool bAftertouch);
	bool m_bRagdollAiming;

	// Camera ID for thrown and KO AT
	int	m_iCameraHandle;

	// Unsafe position code
	int		HasBeenInUnsafePos(void) const { return m_iHasBeenInUnsafePos; }
	void	IsInUnsafePosition(void) const { ++m_iHasBeenInUnsafePos; }
	void	IsInSafePosition(void) const { m_iHasBeenInUnsafePos = 0; }

	// Weapon holstering
	void	SetUnholsterGotoState(EGOTOSTATEAFTERUNHOLSTER eGotoState)	{ m_eUnholsterGotoState = eGotoState; }
	EGOTOSTATEAFTERUNHOLSTER	GetUnholsterGotoState(void)	const	{ return m_eUnholsterGotoState; }


	// List used to house the invisible entities
	typedef ntstd::List< const CEntity * > InvisibleList;

	// Add and clear entities from the list invisible list.
	void AddInvisibleEntity						( const CEntity* pEnt )		{ m_InvisibleList.push_back( pEnt ); }
	void RemoveInvisibleEntity					( const CEntity* pEnt )		{ m_InvisibleList.remove( pEnt ); }
	void FlushInvisibleEntityList				( void )					{ return m_InvisibleList.clear(); }
	const InvisibleList& GetInvisibleList		( void ) const				{ return m_InvisibleList; }

	// Set Visible weight defaults
	const AIFactScoreWeights*	GetVisibleScoreWeights			( void ) const { return m_pVisibleScoreWeights; }
	void						DefaultVisibleScoreWeight		( void );
	void						SetVisibleScoreWeight			( CHashedString obXMLObjectName );


	// Army Gubbins
	void						SetArmyRenderable( ArmyRenderable* pRenderable );

	ArmyRenderable*				GetArmyRenderable()
	{
		return m_pArmyRenderable;
	}

	// ---- Remove from world - if possible ------ 
	virtual bool CanRemoveFromWorld(void);
	virtual bool RemoveFromWorld(bool bSafeRemove = true );

	// ChatterBox: Chat Group ID

	int		GetChatGroupID ( void	) const { return m_iChatGroupID; }
	void	SetChatGroupID ( int i	)		{ m_iChatGroupID = i; }

protected:
	// Description of the entity
	ntstd::String	m_sDescription;
	int				m_AttackSkill;
	int				m_BlockSkill;
	float			m_fAttackSkillAdjust;
	float			m_fBlockSkillAdjust;
	float			m_fVisionUpdateRate;
	CHashedString	m_sInitialSystemState;
	CHashedString	m_sController;
	CHashedString   m_sAIDefinition;
	CHashedString	m_obInitialPatrolRoute;
	//FormationComponent* m_obFormationComponent;

	CAIComponent*	m_pobAIComponent;

	bool			m_bIsEnemy;
	bool			m_bAttackAI;
	SpawnPool*      m_pSpawnPool;			// If we're spawned
	ArmyRenderable*	m_pArmyRenderable;		// if spawned from army code this points back to ourself

	// List of entities this entity can not see
	InvisibleList	m_InvisibleList;

	// Weights that adjust the way visible score are genreated
	const AIFactScoreWeights*	m_pVisibleScoreWeights;

	// Weapon holstering
	EGOTOSTATEAFTERUNHOLSTER	m_eUnholsterGotoState;

	// Quick solution for entities getting into bad areas
	mutable int		m_iHasBeenInUnsafePos;

	// ChatterBox related : Chat Group ID
	int				m_iChatGroupID;
};

LV_DECLARE_USERDATA(AI);

typedef ntstd::List<AI*, Mem::MC_AI> AIEntityList;
#endif //_ENTITY_AI_H
