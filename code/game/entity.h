/***************************************************************************************************
*
*	$Header:: /game/entity.h 8     18/08/03 13:51 Dean                                             $
*
*	Core Entity System Header
*
*	CHANGES
*
*	17/7/2003	Dean	Created
*
***************************************************************************************************/

#ifndef	_ENTITY_H
#define	_ENTITY_H

// necessary includes
#include "physics/config.h"
#include "anim/CharacterBoneID.h"
#include "anim/AnimationHeader.h"
#include "game/keywords.h"

#ifndef AREA_SYSTEM_H
#include "area/areasystem.h"
#endif

#ifndef ENTITY_ANIM_SET_H
#include "game/entityanimset.h"
#endif

// Forward references
class	Transform;
class	CHierarchy;
class	CAnimator;
class	CClumpHeader;
class	CMovement;
class	CMessageHandler;
class	CDynamics;
class	CRenderableComponent;
class	CAIComponent;
class	CInputComponent;
class	CAttackComponent;
class	EntityAudioChannel;
class	CEntityBrowser;
class	CLODComponent;
class	CInteractionComponent;
class	AwarenessComponent;
class	LuaAttributeTable;
class   SceneElementComponent;
class	SectorSystemImpl;
class	GameEventList;
class	FormationComponent;
class	BlendShapesComponent;
class	CLODComponentDef;
class	LookAtComponent;
struct	LookAtInfo;
class	CollisionSphereSet;
class	ArtificialWind;
class	CollisionSword;
class	CollisionFloor;
class	AimingComponent;
class	OneChain;
class	Character;
class	Player;
class	AI;
class	Interactable;
class	Object_Projectile;
class	CAttackComponent;

class	CAnonymousEntComponent;
class	CAnonymousEntComponentMap;


namespace FSM {class StateMachine;}

namespace NinjaLua
{
	class LuaObject;
	class LuaState;
};

namespace Physics
{
	class System;
};

#define COMPONENT_ACCESS( TYPE, FUNCTION, VAR ) \
		virtual TYPE*		FUNCTION( void ) { return VAR; } \
		virtual const TYPE*	FUNCTION( void ) const { return VAR; } \
		virtual TYPE*		FUNCTION##_nonconst( void ) { return VAR; } \
		virtual const TYPE*	FUNCTION##_const( void ) const { return VAR; }

// Used to define access to a component that does not exist in base class
#define COMPONENT_ACCESS_STUB(TYPE, FUNCTION)							\
		virtual TYPE*		FUNCTION()					{return 0;}		\
		virtual const TYPE*	FUNCTION() const			{return 0;}		\
		virtual TYPE*		FUNCTION##_nonconst()		{return 0;}		\
		virtual const TYPE*	FUNCTION##_const() const	{return 0;}


// Used to define access to a component that does not exist in base class
#define COMPONENT_ACCESS_NON_IMPLEMENTED(x)	\
	virtual x* Get##x(){					\
		ntAssert_p(0, (#x " components not available in base class"));	\
		return 0;	}						\
	virtual void Set##x(x *){				\
		ntAssert_p(0, (#x " components not available in base class")); }

// Used to define access to a component that exists in derived class
#define COMPONENT_ACCESS_IMPLEMENTED(x)		\
	virtual x* Get##x() {return m_pob##x;}	\
	virtual void Set##x(x *pob##x)			\
	{										\
		ntError(m_pob##x == 0);				\
		m_pob##x = pob##x;					\
	}


/***************************************************************************************************
*
*	CLASS			CEntity
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

class	CEntity	: public CNonCopyable, public EntityAnimSet
{
	friend	class CEntityBrowser;
	friend	class AreaManager;

protected:
	// Largest members first (ooh err...)
	CPoint			m_obNewPos;
	CPoint			m_obLastPos;
	CDirection		m_obCalculatedVel;
	CPoint			m_InitialPosition;
	CQuat			m_InitialRotation;

	// A collection of keywords so we can be selected in relevant groups
	CKeywords m_obKeywords;

	// List of child entities
	ntstd::List<CEntity*>				m_obChildEntities;


	HAS_INTERFACE(CEntity)
    

public:
	CEntity();
	virtual ~CEntity();	

	// may be a family of these, such as:
	// OnAreaActivation / Deactivation
	// OnAreaMakeVisible / Invisible
	// Help with putting in clean access points when entities can rely on resources
	// being properly loaded.

	// main purpose is to defer the access of unloaded animations / unpatched anim
	// containers, till after level XML parsing is complete (i.e. move 'PlayAnim'
	// type calls from OnPostConstruct to OnLevelStart
	virtual void OnLevelStart() {};

	HAS_LUA_INTERFACE()

	// -------------------------------------------------------------------------
	// A list of the major types
	// -------------------------------------------------------------------------
	// PLEASE UPDATE THIS NUMBER OF ENTITY TYPES IF THE ENTTYPE ENUM IS UPDATED
	// ALSO UPDATE CEntityManager::m_aobEntityBucketInfos
	//
	// NOTE: The order of these entity types defines their update order
	enum	EntIndex
	{
			EntIndex_Interactable,
			EntIndex_Static,
			EntIndex_Unknown,
			EntIndex_Player,
			EntIndex_AI,
			EntIndex_Boss,
			EntIndex_Collision,
			EntIndex_Projectile,
			EntIndex_Object,
			m_iNumEntTypes
	};
	enum	EntType
	{
			// Bucketed types
			EntType_Interactable	= 0x0001,
			EntType_Static			= 0x0002,
			EntType_Unknown			= 0x0004,
			EntType_Player			= 0x0008,
			EntType_AI				= 0x0010,
			EntType_Boss			= 0x0020,
			EntType_Collision		= 0x0040,
			EntType_Projectile		= 0x0080,
			EntType_Object         = 0x0100,

			// Helper types
			EntType_All				= EntType_Player | EntType_AI | EntType_Interactable | EntType_Static | EntType_Unknown | EntType_Boss | EntType_Collision | EntType_Projectile | EntType_Object,
			EntType_Character		= EntType_Player | EntType_AI | EntType_Boss,
			EntType_AllButStatic	= EntType_All & (~EntType_Static),
			EntType_Weapon			= EntType_Interactable | EntType_Unknown | EntType_Projectile,	// This should be updated when weapons are updated

	};

	enum	EntFlags
	{
		// Game state flags
		EntFlags_Paused				= 1 << 0,
		EntFlags_InNinjaSequence	= 1 << 1,
		EntFlags_DeadMessageSent	= 1 << 2,

		// System state flags
		EntFlags_PostConstructed	= 0x00010000,
		EntFlags_AreaResFixedUp		= 0x00020000,
		EntFlags_ToDestroy			= 0x00040000,
	};

 	

	// --------------------------------------------------------------------------

	// ---- Accessors ----
	inline ntstd::String		GetName( void ) const;
	uint32_t					GetHashKey() const;
	inline ntstd::String		GetType( void ) const;
	uint32_t					GetMappedAreaInfo() const { return m_iMappedAreaInfo; }

	// ---- Queries ----
	inline bool	IsPlayer(void) const;
	inline bool	IsBoss(void) const;
	inline bool	IsEnemy(void) const;
	inline bool	IsStatic() const;
	inline bool IsCharacter() const;
	inline bool IsAI() const;
	inline bool IsFriendly() const;
	inline bool IsInteractable() const;
	inline bool IsProjectile() const;

	// ---- Downcasts ----
	inline Character* ToCharacter();
	inline const Character* ToCharacter() const; 
	inline Player* ToPlayer();
	inline const Player* ToPlayer() const; 
	inline AI* ToAI();
	inline const AI* ToAI() const; 
	inline Interactable* ToInteractable();
	inline const Interactable* ToInteractable() const; 
	inline Object_Projectile* ToProjectile();
	inline const Object_Projectile* ToProjectile() const;

	// ---- Component access ----
	COMPONENT_ACCESS( CHierarchy,			GetHierarchy,				m_pobHierarchy );
	COMPONENT_ACCESS( CAnimator,			GetAnimator,				m_pobAnimator );
	COMPONENT_ACCESS( CMovement,			GetMovement,				m_pobMovement );
	COMPONENT_ACCESS( CMessageHandler,		GetMessageHandler,			m_pobMessageHandler );
	COMPONENT_ACCESS( Physics::System,		GetPhysicsSystem,			m_pobPhysicsSystem );
	COMPONENT_ACCESS( AwarenessComponent,	GetAwarenessComponent,		m_pobAwarenessComponent );
	COMPONENT_ACCESS( CAttackComponent,		GetAttackComponent,			m_pobAttackComponent );
	COMPONENT_ACCESS( EntityAudioChannel,	GetEntityAudioChannel,		m_pobEntityAudioChannel );
	COMPONENT_ACCESS( CRenderableComponent,	GetRenderableComponent,		m_pobRenderableComponent );
	COMPONENT_ACCESS( CLODComponent,		GetLODComponent,			m_pobLODComponent );
	COMPONENT_ACCESS( CInteractionComponent,GetInteractionComponent,	m_pobInteractionComponent );
	COMPONENT_ACCESS( SceneElementComponent,GetSceneElement,			m_pSceneElementComponent );
	COMPONENT_ACCESS( BlendShapesComponent,	GetBlendShapesComponent,	m_pobBlendShapesComponent );
	COMPONENT_ACCESS( LookAtComponent,		GetLookAtComponent,			m_pobLookAtComponent );

	COMPONENT_ACCESS_IMPLEMENTED(AimingComponent)
	COMPONENT_ACCESS_STUB(CInputComponent, GetInputComponent)
	COMPONENT_ACCESS_NON_IMPLEMENTED(CollisionSphereSet)
	COMPONENT_ACCESS_NON_IMPLEMENTED(ArtificialWind)
	COMPONENT_ACCESS_NON_IMPLEMENTED(CollisionSword)
	COMPONENT_ACCESS_NON_IMPLEMENTED(CollisionFloor)
	COMPONENT_ACCESS_IMPLEMENTED(OneChain)

#define DEFINE_FLAG_ACCESSORS(n) \
	bool	Is##n() const { return m_uiEntityFlags & (EntFlags_##n); }	\
	void	Set##n(bool bValue) { m_uiEntityFlags = bValue ? (m_uiEntityFlags | EntFlags_##n) : (m_uiEntityFlags & ~EntFlags_##n); }

	DEFINE_FLAG_ACCESSORS(Paused)
	DEFINE_FLAG_ACCESSORS(InNinjaSequence)
	DEFINE_FLAG_ACCESSORS(DeadMessageSent)
	DEFINE_FLAG_ACCESSORS(PostConstructed)
	DEFINE_FLAG_ACCESSORS(AreaResFixedUp)
	DEFINE_FLAG_ACCESSORS(ToDestroy)

	// Query the type of an entity
	bool	IsType(const char*) const;

	// Debug render the entity
	void	DebugRender( void );

	// get the root world position of entity
	inline CPoint	GetPosition( void ) const;

	// get a 'soft' position for the entity
	CPoint	GetLocation( void ) const;

	// Set World Position of entity
	void SetPosition(const CPoint &obPoint);

	// get world orientation of entity
	CQuat	GetRotation( void ) const;
	// Set world Rotation. 
	void	SetRotation(const CQuat& obRot);
	// get the world matrix
	inline const CMatrix&	GetMatrix( void ) const;
	// Get a pointer to the root transform for this entity
	inline const Transform* GetRootTransformP( void ) const;
	// Get a particular transform in the hierarchy for the entity
	inline const Transform* GetTransformP( CHashedString obTransformName ) const;
	// Get a particular transform in the hierarchy for the entity
	inline Transform* GetCharacterTransformP( CHARACTER_BONE_ID eBone );
	inline const Transform* GetCharacterTransformP( CHARACTER_BONE_ID eBone ) const;
	// Checks if a particular transform exists
	inline bool DoesTransformExist( const char* pcTransformName ) const;

	// Get the camera correct position of the entity.  (e.g. Look at the heroines arse not her feet...)
	inline CPoint GetCamPosition() const;

	//! true to put the entity into pause mode
	virtual void Pause(bool bPause, bool bFullPhysicsPause = false);

	//! return our paused status
	void	InstallHierarchy();
	void	InstallHierarchyFromLua(LuaAttributeTable* attrTable);
	inline void	InstallAudioChannel();
	inline void	InstallMessageHandler();
	void	InstallAnimator(const CHashedString& AnimContainerName);
	void	InstallRenderableFromLua(LuaAttributeTable* attrTable);
	void	InstallDynamics();
	void	InstallCamElementComponent();
	void	InstallBlendShapesComponent( const char* bsclumpFileName = 0 );
	void	UninstallBlendShapesComponent( void );
	void	InstallLookAtComponent( CHashedString obCompName );

	void	SetMovement( CMovement* pobMovement )	{ m_pobMovement = pobMovement; }
	
	void	SetAwarenessComponent( AwarenessComponent* pobAwarenessComponent ) { m_pobAwarenessComponent = pobAwarenessComponent; }
	void	SetAttackComponent( CAttackComponent* pobAttackComponent ) { m_pobAttackComponent = pobAttackComponent; }
	void	SetSceneElementComponent(SceneElementComponent* pSceneElementComponent) {m_pSceneElementComponent = pSceneElementComponent;}
	void	SetFormationComponent(FormationComponent* pobFormationComponent) {m_pobFormationComponent = pobFormationComponent;}

	// Access to the time multiplier functionaliity
	float	GetTimeMultiplier( void ) const {return m_fTimeMultiplier;}
	void	SetTimeMultiplier( float fTimeMultiplier ) {m_fTimeMultiplier = fTimeMultiplier;} 

	// Access to the time change functionality.
	float	GetLastTimeChange( void ) const { return m_fLastTimeChange; }
	void	SetLastTimeChange( float fTimeChange ) { m_fLastTimeChange = clamp( fTimeChange, 0.0f, 5.0f/30.0f ); }

	// Setting up the two way entity hierarchy
	void SetParentEntity( CEntity* pobParent );

	// Required for the lua binding, 
	FormationComponent* GetFormationComponent() const { return m_pobFormationComponent; }

	LookAtInfo*			GetLookAtInfo( void )		{ return m_pobLookAtInfo; }
	const LookAtInfo*	GetLookAtInfo( void ) const { return m_pobLookAtInfo; }

	virtual void	UpdateDerivedComponents(float) {};

	bool	IsBlendShapesCapable( void ) const;
// Bind functions
public:
	#include "game/entity_lua.h"

private: 
	
	// For internal access only - the SetParent should be used for constructing the heirarchy
	void AddChildEntity( CEntity* pobChild );
	void RemoveChildEntity( CEntity* pobChild );

public:

	// Const access to the two way entity hierarchy
	const CEntity* GetParentEntity( void ) const { return m_pobParentEntity; }
	const ntstd::List<CEntity*>& GetChildEntities( void ) const { return m_obChildEntities; }

	void SetPhysicsSystem( Physics::System* p_system );

	// ----- Scripting -----

	void	DoThinkBehaviour (); // LUA think & nextthink update
	bool	HasThink(void) const { return m_bHasThink; }
	void	SetHasThink(bool bValue) { m_bHasThink = bValue; }

	bool	HasAttributeTable() const { return (m_obAttributeTable != 0); }
	inline bool	HasLuaTable() const;


	// ----- Animation -----
	void		RebuildAnimEventLists ();
	
	// ----- Rendering -----
	virtual void Hide();
	virtual void Show();
	bool IsHidden();

	// Dangerous ability to change the attribute table of an object
	inline void SetAttributeTable( LuaAttributeTable* );
	inline LuaAttributeTable* GetAttributeTable() const { return m_obAttributeTable; }

	// Return the attrib table for the entity
	NinjaLua::LuaObject GetAttrib(void) const;

	// ----- Sectoring Stuff -----
	void		FixUpAreaResources();
	void		ReleaseAreaResources();

	// ---- Remove from world - if possible ------ 
	virtual bool CanRemoveFromWorld(void);
	virtual bool RemoveFromWorld(bool bSafeRemove = true);

	virtual void Reset() {;}

	// Returns true if an entity is within an area
	bool InArea(int iAreaNumber) const { return m_areaInfo.Within( iAreaNumber ); }

	// Get clump path
	ntstd::String	GetClumpString() const
	{
		return m_Clump;
	}

	// Get hash value for the clump file name (minus the extension)
	unsigned int GetClumpFileHash () const
	{
		return m_uiClumpFileHash;
	}

	// Visibility query
	bool IsRenderDisabled() const
	{
		return m_bDisableRender;
	}

protected:
	// General flags to describe the state of the entity. Access to individual flags is through the DEFINE_FLAG_ACCESSORS macro
	uint32_t	m_uiEntityFlags;

	// from MrEd, bitmask flags what areas we're within
	uint32_t	m_iMappedAreaInfo;	

	// entity to switch off when we are made visible by the area system
	bool			m_replacesEnt;
	CHashedString	m_entToReplace;

	// Per entity storage for area information
	AreaInfo	m_areaInfo;			

	void		LoadClumpHeader( const char* pClumpName );
	void		ReleaseClumpHeader();

	void		InstallGfxComponents( const char* pClumpName );
	void		DeleteGfxComponents();

protected:

	// Two way linking through the entity heirarchy
	CEntity*							m_pobParentEntity;

	// Here are the components
	CHierarchy*							m_pobHierarchy;	
	CAnimator*							m_pobAnimator;
	CMovement*							m_pobMovement;
	CRenderableComponent*				m_pobRenderableComponent;
	CMessageHandler*					m_pobMessageHandler;
	Physics::System*					m_pobPhysicsSystem;
	AwarenessComponent*					m_pobAwarenessComponent;
	CAttackComponent*					m_pobAttackComponent;
	EntityAudioChannel*					m_pobEntityAudioChannel;
	CLODComponent*						m_pobLODComponent;
	CInteractionComponent*				m_pobInteractionComponent;
 	SceneElementComponent*				m_pSceneElementComponent;
	BlendShapesComponent*				m_pobBlendShapesComponent;
	LookAtComponent*					m_pobLookAtComponent;
	FormationComponent*					m_pobFormationComponent; // This should be removed when there is an appropriate Entity type to cover GroupCombatEntity
	AimingComponent*					m_pobAimingComponent;
	OneChain*							m_pobOneChain;

	// Pointer to the state machine if entity has one.
	FSM::StateMachine*	m_pFSM;

	// A time multiplier which will be used for special behaviour
	float				m_fTimeMultiplier;
	float				m_fLastTimeChange;

	//This is where all the dynamic state of the character is stored
	//it is altered by states and messages
	LuaAttributeTable*	m_obAttributeTable;
	CClumpHeader*		m_pobClumpHeader;			// Pointer to R/O clump header

	ntstd::String		m_Clump;
	unsigned int		m_uiClumpFileHash; // Hash value for the clump file name (minus the rest of the path and the .clump extension)
	CHashedString		m_ConstructionScript;
	CHashedString		m_DestructionScript;
	CHashedString		m_ParentTransform;
	

	CHashedString		m_DefaultDynamics;
	// Mirror of the entity name in the attribute table, just readable from the debugger
	CHashedString		m_Name;						
	bool				m_bCastShadows;
	bool				m_bRecieveShadows;
	bool				m_bDisableRender;
	bool				m_bHasThink;

	void				OnPostConstruct();
	void				OnPostPostConstruct();

	void				SetClumpString( const ntstd::String& clump );

	// So that event processing may be attached to an entity
	GameEventList*		m_pobGameEventsList;

	// The primary type of the object
	EntType				m_eType;

	// entity look-at attribs (if any)
	LookAtInfo*			m_pobLookAtInfo;
	bool				m_bLevelActiveConstruction;

public:

	bool				HasFSM() { return m_pFSM; }
	FSM::StateMachine*	GetFSM() { return m_pFSM; }

	// Entity Type accessor
	EntType GetEntType() const
	{
		return m_eType;
	}

// New stuff from the CentityInfo 

	// Change Description of a single item
	void RemoveKeyword( const char* pcRemove );
	void AddKeyword( const char* pcAdd );

	// Description can potentially be modified on an entity (e.g. in script)
	CKeywords& GetKeywords() { return m_obKeywords; } 
	const CKeywords& GetKeywords() const { return m_obKeywords; } 

	// Gameplay type stuff
	bool IsLockonable( void ) const;

	void UpdateVelocity( float fTimeStep );
	CDirection GetCalcVelocity( void ) const;
	void GetLookDirection( CDirection& obFacing ) const;

	virtual bool IsRangedWeapon() const { return false; }
};

#include "lua/ninjalua.h"
	
LV_DECLARE_USERDATA(CEntity);

#endif	//_ENTITY_H
