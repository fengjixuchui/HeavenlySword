//------------------------------------------------------------------------------------------
//!
//!	\file formationcompoent.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIFORMATION_COMPONENT_H
#define _AIFORMATION_COMPONENT_H

#include "game/anonymouscomponent.h"
#include "game/keywords.h"
#include "lua/ninjalua.h"
#include "editable/enums_ai.h"
#include "aiformation.h"
#include "aiformationattack.h"

// Forwards Eclares
class CEntity;
class CAttackComponent;
class AIFormationSlot;
class CGromboState;
class CGromboAttack;
class CGromboEntity;
class CGromboInstance;
class CGromboAttackPattern;
class CGromboAttackList;

//------------------------------------------------------------------------------------------
//!
//!	FormationSquad
//!	
//!
//!
//------------------------------------------------------------------------------------------
class FormationSquad
{
public:

	// Returns the number of types this squad has
	///////////////////////////////////////////////
	int TypeCount( const char* pcType );

	// Used for the sort operator
	///////////////////////////////////////////////
	bool operator>( const FormationSquad& Other ) const;

	// Name of the squad
	ntstd::String m_Name;

	// List of all the entities in the squad. 
	ntstd::List< AI* > m_Entities;

	// The currenty assigned formation of the squad. 
	AIFormation* m_pAssignedFormation;

	// Attacks are disabled for the squad
	bool m_AttackDisabled;

	// Message to send when a entity in the squad dies
	ntstd::String m_onDeath;

	// Message to send when a entity in the attack get's KO'd
	ntstd::String m_onKO;
};
typedef ntstd::List<FormationSquad*, Mem::MC_AI>	FormationSquadList;


//------------------------------------------------------------------------------------------
//!
//!	FormationAttack
//!	
//! 
//!
//------------------------------------------------------------------------------------------
struct FormationAttack
{
	// Name of the attack
	ntstd::String m_Name;

	// The formation attack data
	AIFormationAttack* m_pAttackData;

	// array of the formation squads that can be involved in the attack
	CKeywords	m_Squads;

	// Message to send when a entity in the attack dies
	ntstd::String m_onDeath;

	// Message to send when a entity in the attack get's KO'd
	ntstd::String m_onKO;
};
typedef ntstd::List<FormationAttack*, Mem::MC_AI>	FormationAttackList;

//------------------------------------------------------------------------------------------
//!
//!	FormationComponent
//!	
//! This component can be seen a mini-formation manager and should be attached to
//! entities that want to manage group combat formations.
//! 
//! 
//! 
//!
//------------------------------------------------------------------------------------------

class FormationComponent : public CAnonymousEntComponent
{
public:
	FormationComponent(CEntity* pEnt);
	virtual ~FormationComponent();

	HAS_LUA_INTERFACE()

	// Update
	//////////////////////////////////
	virtual void Update(float fTimeDelta);


	// Return the parent entity
	//////////////////////////////////
	CEntity* GetParent() const { return m_pParent; }


	// Add a formation to the component
	////////////////////////////////////
	bool AddFormation( const char* pName, NinjaLua::LuaObject rFormationMakeup );

	// Add a formation attack 
	////////////////////////////////////
	AIFormationAttack* AddAttackPattern( const CGromboAttackPattern* pAttackPattern, const ntstd::String& rName);


	// Clear the attack patterns
	////////////////////////////////////
	void ClearAttackPatterns();

	// Delte all formation on the component
	////////////////////////////////////
	void DeleteAllFormations( void );


	// Add an entity to the formation, there are rules to adding entities to formations. 
	////////////////////////////////////a
	bool AddEntity( AI* pEnt, bool bTestOnly = false );
	bool LuaAddEntity( AI* pEnt ) { return AddEntity( pEnt ); }

	// Remove an entity from the formation
	////////////////////////////////////a
	bool RemoveEntity( AI* pEnt );

	// Combat state changed
	/////////////////////////////////////////////////////////////////
	void CombatStateChanged( AI* pEnt, COMBAT_STATE eCombatState );

	// Mechanism for activating formations
	////////////////////////////////////
	bool ActivateFormations( NinjaLua::LuaObject rFormationActivation );	

	// Function for entry rules into the squads
	////////////////////////////////////
	void SetEntryRules( NinjaLua::LuaObject EntryRules ) { m_EntryRules = EntryRules; }

	// Function for entry rules into the squads
	////////////////////////////////////
	void SetMetaTags( const char* pcKeywords ) { m_obMetadata = CKeywords( pcKeywords ); }

	// Assigns formations to squads
	////////////////////////////////////
	void SquadAssignment( NinjaLua::LuaObject SquadList );

	// Assigns attacks to squads
	////////////////////////////////////
	void AttackAssignment( const char* pcName );

	// Assigns formations to squads
	////////////////////////////////////
	void SquadAttackAllowed( const char* pcName, bool bValue );

	// Are there any formations
	////////////////////////////////////
	bool ActiveFormationAvailable(void) const { return m_FormationList.size(); }

	// Restart the current formation
	////////////////////////////////////
	bool ReStartCurrentFormation(void) { /*AttackAssignment(m_AttackDef); */return ActivateFormations( m_Def ); }

	// Get largest attack count for a formation
	int AttackCount(void) const { return m_AttackList.size(); }

	// Find a squad
	FormationSquad* FindSquad( const ntstd::String& );
	FormationSquad* FindSquad( u_int uiHash );

	// Find an attack
	FormationAttack* FindAttack( const ntstd::String& );

	// Is the space of the given point in the context of the formation valid?
	void UpdateClearOfOtherSlots(AIFormation* pFormation, float fRange = 2.0f);

	// List of all the entities in the formations.
	const ntstd::List<CEntity*>& GetEntityList() const { return m_Entities; }

	// Set the formation with a region lock
	void SetNavRegionLock(const char* rRegionName ) { m_obNavRegionLock = rRegionName; }
	void ClearNavRegionLock( void ) { m_obNavRegionLock.resize(0); }
	const char* GetNavRegionLock(void) const { return ntStr::GetString(m_obNavRegionLock); }

	void ResetOverlapCheckedStatus();

	// Get the metadata strings. 
	const CKeywords& GetMetadta(void) const { return m_obMetadata; }

private:

	// Create a new squad, if a squad with the same name already exists then it returns that.
	////////////////////////////////////
	FormationSquad* NewSquad( const char* pcName );

	// Create a new attack, if an attack with the same name already exists then it returns that.
	////////////////////////////////////
	FormationAttack* NewAttack( const ntstd::String& );

	// Sor the squads based on their assigned formation priority
	////////////////////////////////////
	void SortSquads( void );

	// Obtain a formation by a given name
	////////////////////////////////////
	AIFormation* GetFormation(const char* pcName );


	// Free up the formation component of all it's entities
	////////////////////////////////////
	void Free( bool bInDestructor );

private:

	// Pointer to the owner parent entity
	CEntity*	m_pParent;

	// Current formation definition 
	NinjaLua::LuaObject m_Def;

	// Rules for entering the squads
	NinjaLua::LuaObject m_EntryRules;

	// Is the Formation locked to a region in the world?
	ntstd::String	m_obNavRegionLock;

	// List of all the entities in the formations.
	ntstd::List<CEntity*> m_Entities;


	// List of formations held by the formation component
	AIFormationList m_FormationList;

	// List of squads
	FormationSquadList m_SquadList;

	// List of attacks
	FormationAttackList m_AttackList;

	// List of active attacks on the formation
	AIFormationAttackList	m_ActiveAttacks;

	// Metadata for picking formation attacks
	CKeywords	m_obMetadata;
};

LV_DECLARE_USERDATA(FormationComponent);


//------------------------------------------------------------------------------------------
//!
//!	AIFormationComponent
//!	Not a component that is placed in the root of the entry, but instead is placed in the 
//! the AI component of the entity. 
//!
//! The formation component was created to contain entity specific formation info,
//! a common interface to formation features for an entity and to help the entity manage
//! formations systems.
//!
//------------------------------------------------------------------------------------------

class AIFormationComponent
{
public:
	AIFormationComponent(AI* pEnt);
	~AIFormationComponent();

	HAS_LUA_INTERFACE()

	// Update
	//////////////////////////////////
	void Update(float fTimeDelta);

	// Return the parent entity
	//////////////////////////////////
	AI* GetParent() const { return m_pParent; }

	// Remove all the active formations
	/////////////////////////////////////////////////////////////////
	//void DeleteAllFormations( Lua

	// Combat state changed
	/////////////////////////////////////////////////////////////////
	void CombatStateChanged( AI* pEnt, COMBAT_STATE eCombatState );

	// Return the formation assigned to the entity
	AIFormation* GetFormation(void) const { return m_pFormation; }

	// Allow the commander to be set
	void SetCommander(FormationComponent* pObject) { m_pCommander = pObject; }
	FormationComponent* GetCommander(void) const { return m_pCommander; }


	// Access the squad information
	void SetSquadName(const ntstd::String& Name) { m_SquadName = Name; }
	const ntstd::String& GetSquadName(void) const { return m_SquadName; }
	void NullSquadName(void) { m_SquadName = ntstd::String(); }

	// Set the formation for the entity.
	void SetFormation(AIFormation* pFormation) { m_pFormation = pFormation; }

	// Obtain the formation attack the entity is currently in
	AIFormationAttack*	GetFormationAttack() const {return m_pAttack;}

	// Set the formation attack
	void SetFormationAttack(AIFormationAttack* pAtt) {m_pAttack = pAtt;}

	// Is the entities movement currently paused with regards to the formation?
	bool IsMovementPaused() const {return m_fFormationMovementPaused > 0.0f; }

	// Allow setting of a movement pause.
	void SetMovementPause(float fValue)	{m_fFormationMovementPaused = fValue; }
	float GetMovementPause(void) const { return m_fFormationMovementPaused; }

	// Access to to the NoFormationUpdate boolean
	bool GetNoFormationUpdate(void) const { return m_bNoFormationUpdate || !m_Active; }
	void SetNoFormationUpdate(bool bState);

	// Is this entity active in the formation?
	bool IsActive(void) const {return m_Active;}
	void Activate(bool b)     {m_Active = b;}

	// Remove this entity from the formation
	void Remove(void);

	// Can the entity perform one on one style attacks?
	bool CanAttack(void);
	bool CanBlock(void);

	// There are cases when the entities can attack
	void AllowFreeAttack(int iAttackCount = 1) { m_FreeAttackCount = iAttackCount; }

	// Return the time that we've not been in a grombo
	float GetTimeNotInGrombo() const { return m_TimeNotInGrombo; }

	// Set the formation with a region lock
	void SetNavRegionLock(const char* rRegionName ) { m_obNavRegionLock = rRegionName; }
	void ClearNavRegionLock( void ) { m_obNavRegionLock.resize(0); }
	const char* GetNavRegionLock(void) const { return ntStr::GetString(m_obNavRegionLock); }

	// Behaviour ref count access
	u_int	BehaviourRefCount(void) const { return m_uiBehaviourRefCount; }
	void	BehaviourRefCountInc(void) { ++m_uiBehaviourRefCount; }
	bool	BehaviourRefCountDec(void) { --m_uiBehaviourRefCount;  return !m_uiBehaviourRefCount; }

private:

	// Pointer to the owner parent entity
	AI* m_pParent;

	// Formation behaviour ref counter
	u_int	m_uiBehaviourRefCount;

	// Pointer to the commander entity
	FormationComponent* m_pCommander;

	// Recorded name of the squad the entity is in
	ntstd::String m_SquadName;

	// Pointer to the formation the entity is current in
	AIFormation* m_pFormation;

	// Pointer to the formation attack
	AIFormationAttack* m_pAttack;

	// Is the Formation locked to a region in the world?
	ntstd::String	m_obNavRegionLock;

	// This allows the entity to pause its movement in the formation for a time. 
	float m_fFormationMovementPaused;

	// Can the entity perform some free attacks
	int m_FreeAttackCount;

	// Ask the formation system not to update the formation position for this entity
	bool  m_bNoFormationUpdate;

	// Record the time the entity hasn't been in the grombo
	float m_TimeNotInGrombo;

	// Is the formation active?
	bool m_Active;
};

LV_DECLARE_USERDATA(AIFormationComponent);


#endif // _AIFORMATION_COMPONENT_H
