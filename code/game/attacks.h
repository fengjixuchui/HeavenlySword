/***************************************************************************************************
*
*	Core attack structures 
*
*	CHANGES
*	Note that implementation is in attackcomponent.cpp and attackdataandhelpers.cpp - DGF 20/03/06
*
*	31/10/2003	Mike	Created
*
***************************************************************************************************/

#ifndef _ATTACKS_H
#define _ATTACKS_H

// Necesary includes
#include "editable/enumlist.h"
#include "editable/flipflop.h"

#include "game/combat_lua.h"

#include "attacktracking.h"
#include "attackmovement.h"

#include "game/strike.h"

#include "game/entitymanager.h"
#include "game/entityinfo.h"

#include "game/attackcameras.h"
#include "game/eventlog.h"

#include "objectdatabase/gameguid.h"

// Forward references
struct lua_State;
class CInputComponent;
//class CAttackComponent;
//class CAttackTracker;
class CAttackRecoil;
class CAttackStagger;
class CAttackBlock;
class CEntity;
class CStrike;
class CMessage;
class CAttackDefinition;
class CAttackKO;
class TimeScalarCurve;
class CAttackData;
class CAttackLink;
class CAttackTargetingData;
class AttackDebugger;
class CombatEffectsDefinition;
class SpecialDef;
class AttackSpecial;
class HitCounterDef;
class HitCounterCharacterMulDef;
class HitCounter;
class Transform;
class SuperStyleSafetyVolume;

/***************************************************************************************************
*
*	FUNCTION		SpecificAttackVulnerabilityZone
*
*	DESCRIPTION		Area of vulnerability to a specific attack
*
***************************************************************************************************/
class SpecificAttackVulnerabilityZone
{
	HAS_INTERFACE(SpecificAttackVulnerabilityZone)
public:
	SpecificAttackVulnerabilityZone() :
		m_fZoneAngle( 0.0f ),
		m_fZoneSweep( 0.0f ),
		m_fInnerDistance( 0.0f ),
		m_fOuterDistance( 0.0f ),
		m_pobSpecificAttack( 0 ),
		m_iNumUses( 0 ),
		m_bRemoveIfUsedSuccessfully( false ),
		m_bIsDisabled( false )
		{}
	CAttackLink* IsInZone(CEntity* pobZoneOwner, CEntity* pobAttacker);
	bool GetRemoveIfUsedSuccessfully() { return m_bRemoveIfUsedSuccessfully; };
	bool IsDisabled() { return m_bIsDisabled; }
	void SetRemoveIfUsedSuccessfully(bool bRemoveIfSuccessful) { m_bRemoveIfUsedSuccessfully = bRemoveIfSuccessful; }
	void SetDisabled(bool bDisabled) { m_bIsDisabled = bDisabled; }
	void IncrementNumUses() { m_iNumUses++; }
private:
	float m_fZoneAngle; // Orientation of this zone relative to zone owners Z
	float m_fZoneSweep; // Amount of angle the zone covers
	float m_fInnerDistance; // How far until we start the zone
	float m_fOuterDistance; // How far until we stop the zone
	CAttackLink* m_pobSpecificAttack; // Attack link to check if we're in this zone and generating a strike
	int m_iNumUses;
	bool m_bRemoveIfUsedSuccessfully;
	bool m_bIsDisabled;

	friend class AttackDebugger;
};

/***************************************************************************************************
*
*	FUNCTION		CombatPhysicsPushVolumes
*
*	DESCRIPTION		Lists of physics volumes that push stuff
*
***************************************************************************************************/
class CombatPhysicsPushVolumeDescriptor;
class CombatPhysicsPushVolumeData;
// Typedef of chunked lists for easy reference
typedef ntstd::List<CombatPhysicsPushVolumeDescriptor*, Mem::MC_ENTITY> PushVolumeDescriptorList;
typedef ntstd::List<CombatPhysicsPushVolumeData*, Mem::MC_ENTITY> 		PushVolumeDataList;


class CombatPhysicsPushVolumes
{
	HAS_INTERFACE( CombatPhysicsPushVolumes );
public:
	PushVolumeDescriptorList m_obSpeedPhysicsVolumes;
	PushVolumeDescriptorList m_obPowerPhysicsVolumes;
	PushVolumeDescriptorList m_obRangePhysicsVolumes;
};
class CombatPhysicsPushVolumesData
{	
public:
	CombatPhysicsPushVolumesData(const CombatPhysicsPushVolumes& obCombatPhysicsPushVolumes);
	~CombatPhysicsPushVolumesData();

	PushVolumeDataList m_obSpeedPhysicsVolumes;
	PushVolumeDataList m_obPowerPhysicsVolumes;
	PushVolumeDataList m_obRangePhysicsVolumes;
};


/***************************************************************************************************
*
*	CLASS			UninterruptibleWindowDetails
*
*	DESCRIPTION		Stores details about which attack types can interrupt and uninterruptible.
*
***************************************************************************************************/
class UninterruptibleWindowDetails
{
	HAS_INTERFACE(UninterruptibleWindowDetails)
public:
	// Bools describing which attacks stances interupt out uninterruptible attack
	bool m_bUninterruptibleForSpeedAttackMedium, m_bUninterruptibleForSpeedAttackFast;
	bool m_bUninterruptibleForRangeAttackMedium, m_bUninterruptibleForRangeAttackFast;
	bool m_bUninterruptibleForPowerAttackMedium, m_bUninterruptibleForPowerAttackFast;
	bool m_bUninterruptibleForGrab;

	UninterruptibleWindowDetails() {	m_bUninterruptibleForSpeedAttackMedium = 
										m_bUninterruptibleForSpeedAttackFast = 
										m_bUninterruptibleForRangeAttackMedium = 
										m_bUninterruptibleForRangeAttackFast = 
										m_bUninterruptibleForPowerAttackMedium = 
										m_bUninterruptibleForPowerAttackFast = 
										m_bUninterruptibleForGrab = true; };
	~UninterruptibleWindowDetails() {};
};

//------------------------------------------------------------------------------------------
//!
//!	StanceSwitchingDef
//!	Defines how, and how quickly, a character can switch between stances.
//!
//------------------------------------------------------------------------------------------
class StanceSwitchingDef
{
public:

	// Construction
	StanceSwitchingDef( void );

	// Members
	CHashedString	m_obSpeedToPowerAnim;
	CHashedString	m_obSpeedToRangeAnim;
	CHashedString	m_obRangeToSpeedAnim;
	CHashedString	m_obRangeToPowerAnim;
	CHashedString	m_obPowerToSpeedAnim;
	CHashedString	m_obPowerToRangeAnim;
};


//------------------------------------------------------------------------------------------
//!
//!	AerialDetailsDef
//!	Contains the parameters required for an individuals aerial combat
//!
//------------------------------------------------------------------------------------------
class AerialDetailsDef
{
public:

	// Construction
	AerialDetailsDef( void );

	// Members
	CHashedString	m_obAerialWinDrop;
	CHashedString	m_obAerialWinLand;
	CDirection		m_obAerialGrappleTargetOffset;
};


//------------------------------------------------------------------------------------------
//!
//!	ReactionAnimList
//!	Serialised container to hold a set of animations that are used for reacting to 
//!	attacks
//!
//------------------------------------------------------------------------------------------
class ReactionAnimList
{
public:

	// Construction
	ReactionAnimList( void );

	// Our list of reactions - linked to the number of appearances
	CHashedString	m_obAnimation[RA_COUNT];
};

/***************************************************************************************************
*
*	CLASS			ReactionMatrixColumnEntry
*
*	DESCRIPTION		A class to act as an entry in the ReactionMatrixColumn, from this the new 
*					REACTION_TYPE is taken.
*
***************************************************************************************************/
class ReactionMatrixColumnEntry
{
	HAS_INTERFACE(ReactionMatrixColumnEntry)
public:
	ReactionMatrixColumnEntry();
	virtual ~ReactionMatrixColumnEntry();

	REACTION_TYPE GetReactionType(REACTION_TYPE eReactionType);

	REACTION_TYPE m_eDeflect;
	REACTION_TYPE m_eBlockStagger;
	REACTION_TYPE m_eImpactStagger;
	REACTION_TYPE m_eRecoil;
	REACTION_TYPE m_eHeld;
	REACTION_TYPE m_eKO;
	REACTION_TYPE m_eKill;
};

/***************************************************************************************************
*
*	CLASS			ReactionMatrixColumn
*
*	DESCRIPTION		A class to act as a column in the ReactionMatrix.
*
***************************************************************************************************/
class ReactionMatrixColumn
{
	HAS_INTERFACE(ReactionMatrixColumn)
public:
	ReactionMatrixColumn();
	virtual ~ReactionMatrixColumn();

	virtual void PostConstruct();
	virtual bool EditorChangeValue(CallBackParameter, CallBackParameter);

	ReactionMatrixColumnEntry* GetReactionMatrixColumnEntry(ATTACK_CLASS eAttackClass);

	ReactionMatrixColumnEntry* m_pobSpeedAttacks;
	ReactionMatrixColumnEntry* m_pobPowerAttacks;
	ReactionMatrixColumnEntry* m_pobRangeAttacks;
	ReactionMatrixColumnEntry* m_pobOtherAttacks;
};

/***************************************************************************************************
*
*	CLASS			ReactionMatrix
*
*	DESCRIPTION		A class to act as a lookup matrix for specific reactions.
*
***************************************************************************************************/
class ReactionMatrix
{
	HAS_INTERFACE(ReactionMatrix)
public:
	ReactionMatrix();
	virtual ~ReactionMatrix();

	virtual void PostConstruct();
	virtual bool EditorChangeValue(CallBackParameter, CallBackParameter);

	ReactionMatrixColumn* GetReactionMatrixColumn(REACTION_MATRIX_LOOKUP eReactionMatrixLookup);

	ReactionMatrixColumn* m_pobUnBlocked;
	ReactionMatrixColumn* m_pobSpeedBlocked;
	ReactionMatrixColumn* m_pobPowerBlocked;
	ReactionMatrixColumn* m_pobRangeBlocked;
	ReactionMatrixColumn* m_pobSyncdSecondary;
};

class SpecificCounterIndexEntry
{
	HAS_INTERFACE(SpecificCounterIndexEntry);
public:
	CHashedString GetSpecificCounterAttackName() { return m_obSpecificCounterAttackName; };
private:
	CHashedString m_obSpecificCounterAttackName;
};

class SpecificCounterIndex
{
	HAS_INTERFACE(SpecificCounterIndex);
public:
	SpecificCounterIndex() { m_iLastUsed = -1; };
	int GetNumberOfEntries() { return (int)m_obEntries.size(); };
	SpecificCounterIndexEntry* GetEntryAt(int i) const;
	void SetLastUsedIndex(int iLast) { m_iLastUsed = iLast; };
	int GetLastUsedIndex() { return m_iLastUsed; };
private:
	int m_iLastUsed;
	ntstd::List<SpecificCounterIndexEntry*, Mem::MC_ENTITY> m_obEntries;
};

/***************************************************************************************************
*
*	CLASS			SpecificCounter
*
*	DESCRIPTION		A wrapper class for a specific counter name and an associated CAttackLink.
*
***************************************************************************************************/
class SpecificCounter
{
	HAS_INTERFACE(SpecificCounter);
public:
	virtual ~SpecificCounter() {};

	virtual void PostConstruct();
	virtual bool EditorChangeValue(CallBackParameter, CallBackParameter);

	CHashedString GetSpecificCounterAttackName() { return m_obSpecificCounterAttackName; };
	CAttackLink* GetSpecificCounterAttackLink() { return m_pobSpecificCounterAttackLink; };
private:
	CHashedString m_obSpecificCounterAttackName;
	CAttackLink* m_pobSpecificCounterAttackLink;
};

/***************************************************************************************************
*
*	CLASS			SpecificCounterCollection
*
*	DESCRIPTION		A collection class for SpecificCounter objects, with search functionality.
*
***************************************************************************************************/
class SpecificCounterCollection
{
	HAS_INTERFACE(SpecificCounterCollection);
public:
	CAttackLink* GetSpecificCounter(CHashedString obSpecificCounterName) const;
private:
	ntstd::List<SpecificCounter*, Mem::MC_ENTITY> m_obSpecificCounters;	
};


/***************************************************************************************************
*
*	CLASS			SkillEvadeCollection
*
*	DESCRIPTION		A collection class for CAttackLink objects for Kai skill-evades.
*
***************************************************************************************************/
class SkillEvadeCollection
{
	HAS_INTERFACE(SkillEvadeCollection);
public:
	SkillEvadeCollection() { for(int i = 0 ; i < AES_NUM_EVADESECTORS ; i++) { m_iLastSkillEvadeNum[i] = -1; } }
	const CAttackLink* GetRandomSkillEvade(ATTACK_EVADE_SECTOR eEvadeSector);
private:
	int m_iLastSkillEvadeNum[AES_NUM_EVADESECTORS];	//So that we don't get the same one twice in a row.
	//Exposed.
	ntstd::List<CAttackLink*, Mem::MC_ENTITY> m_obSkillEvades[AES_NUM_EVADESECTORS];
};

/***************************************************************************************************
*
*	CLASS			CClusterStructure
*
*	DESCRIPTION		The class manages the different clusters of attacks available to a character.
*
***************************************************************************************************/
class CClusterStructure
{
public: 

	HAS_INTERFACE(CClusterStructure)

	// Construction destruction
	CClusterStructure();
	~CClusterStructure();

	// The main attack cluster
	CAttackLink* m_pobLeadCluster;

	// Cluster for ground attacks (which require different targetting to the normal attacks)
	CAttackLink* m_pobGroundClusterFront;
	CAttackLink* m_pobGroundClusterBack;

	// The cluster for the instant KO recovery
	CAttackLink* m_pobInstantKORecoverAttackCluster;

	// The clusters for 'interception' attacking - moves just before the strike.
	CAttackLink* m_pobInterceptCluster;

	// A cluster that one can use when getting up from a KO
	CAttackLink* m_pobRisingCluster;

	// Clusters to manage style levels
	
	typedef ntstd::List<CAttackLink*, Mem::MC_ENTITY> SuperStyleLevelLinkList; 
		
	SuperStyleLevelLinkList		m_aobSuperStyleLevelLinks[3]; // 3 because only HL_ONE, HL_TWO and HL_THREE need them - HL_FOUR is special, and HL_SPECIAL is... special
	SuperStyleLevelLinkList		m_aobOnTheSpotStyleLevelLinks[3]; // 3 because only HL_ONE, HL_TWO and HL_THREE need them - HL_FOUR is special, and HL_SPECIAL is... special

	CAttackLink* m_pobStyleLevelSpecialCluster;

	// 'To-lockon' attack details
	CAttackLink* m_pobShortRangeCluster;		// Added specifically for AI formation attacks
	CAttackLink* m_pobMediumRangeCluster;
	CAttackLink* m_pobLongRangeCluster;

	// Specific counter collection
	//SpecificCounterCollection* m_pobSpecificCounters;

	// We should do this is we're grabbing someone and they've blocked us
	CAttackLink* m_pobBlockedGrab;

	// Clusters for different types of evade
	CAttackLink *m_pobOpenFreeEvades, *m_pobComboFreeEvades;
	// Clusters for different types of skill-evade (for Kai).
	SkillEvadeCollection *m_pobOpenSkillEvades, *m_pobCloseSkillEvades, *m_pobOpenSuperSkillEvades, *m_pobCloseSuperSkillEvades;

	// Big list of all the counters we can try to do
	SpecificCounterCollection* m_pobSpecificCounterCollection;

	CAttackLink* m_pobLedgeRecoverAttack;
};

/***************************************************************************************************
*
*	CLASS			CAttackLink
*
*	DESCRIPTION		The linking attack structure.
*
***************************************************************************************************/
class CAttackLink
{
public:

	// Construction 
	CAttackLink( void );

	// Access to the details of our attack
	const CAttackData* GetAttackDataP() const { return m_pobAttackData; }

	// Query to see if there is an available attack type
	bool HasAttackType( ATTACK_MOVE_TYPE eAttackType ) const;

	// Get the next attack node
	const CAttackLink* GetNextAttackNode( ATTACK_MOVE_TYPE eAttackType ) const;

	// The basic definitions for this attack - the attack data is all animation relative
	const CAttackData*	m_pobAttackData;

	// Which attacks can be strung on to the end of this
	const CAttackLink* 	m_pobLinks[AM_NONE];

	const CAttackLink* m_pobButtonHeldAttack;

	void SetForceSwapWithLink(CAttackLink* pobForceSwapWith) { m_pobForceSwapWith = pobForceSwapWith; };
	const CAttackLink* GetForceSwapWithLink() const { return m_pobForceSwapWith; };
private:
	// If this is set, whenever this attack link is found in the attack tracker, it will be swapped out with this one
	const CAttackLink* m_pobForceSwapWith;
};


/***************************************************************************************************
*
*	CLASS			CAttackData
*
*	DESCRIPTION		The core attack structure.
*
***************************************************************************************************/
class CombatPhysicsStrikeVolumeDescriptor;
class CombatPhysicsStrikeVolume;
class CAttackData
{
public:

	HAS_INTERFACE( CAttackData )

	// Construction destruction
	CAttackData( void );
	virtual ~CAttackData( void );

	// Make sure that the data is safe - and visible
#ifndef _RELEASE
	virtual void PostConstruct( void );
	virtual bool EditorChangeValue(CallBackParameter, CallBackParameter);
	void CheckFlipFlopRobustness(CFlipFlop& obFlipFlop, const char* pcName, float fFPS, float fRobustnessCheck);
	virtual void DebugRender( void );
#endif

	// Get a new attack time passing in the attack scalar
	float GetAttackTime( float fScalar ) const { return m_fAttackTime * fScalar; }
	float GetAttackRecoverTime( float fScalar ) const { return m_fAttackRecoverTime * fScalar; }

	// Helper to querry the CAttacks style type
	STYLE_TYPE GetStyleType( void ) const;

	// The windows within Attack Time that the strike phases are active
	CFlipFlop	m_obStrike;

	// The windows within Attack Time that seconday characters can be hit by a strike
	CFlipFlop	m_obStrike2;

	// The windows within Attack Time that the an attack can be executed
	CFlipFlop	m_obAttackPopOut;

	// The windows within Attack Time that a movement can be executed
	CFlipFlop	m_obMovementPopOut;

	// The windows within Attack Time that a block or evade can be executed
	CFlipFlop	m_obBlockPopOut;

	// The windows within Attack Time that the next move button can be registered
	CFlipFlop m_obNextMove;
	VIRTUAL_BUTTON_TYPE m_eHintInNextMove;

	// The windows within Attack Time that a character is invulnerable to strikes
	CFlipFlop m_obInvulnerabilityWindow;

	// The windows within Attack Time that a the attack won't lock on to an evading opponent
	CFlipFlop m_obNoLockWindows;

	// The windows within Attack Time the character will not register contact
	CFlipFlop m_obNoCollideWindows;

	// The windows within an attack in which we can be intercepted by a range move
	CFlipFlop m_obRangeInterceptWindow;

	// The windows within an attack in which we can be intercepted by a special evade move
	CFlipFlop m_obInterceptWindow;

	// The window in which an attack is uninterruptable
	CFlipFlop m_obUninterruptibleWindow;
	UninterruptibleWindowDetails* m_pobUninterruptibleWindowDetails;

	CFlipFlop m_obEvadeAndGrabDenyWindow;

	CFlipFlop m_obVictimRagdollableWindow;

	// The attack class of this attack
	ATTACK_CLASS m_eAttackClass;

	// The animation for the attack
	CHashedString m_obAttackAnimName;

	// If the attack is scaled to meet the opponent - this describes how far 
	// from the target the animation will aim to move
	float m_fAttackScaledOffset;

	// How much damage will be incurred if this attack hits the opponent - and it's equivalent value for secondary strikes
	int m_iDamage, m_iDamage2;

	// Reactions to this attack
	REACTION_TYPE m_eUnBlockedReaction;
	REACTION_TYPE m_eSpeedBlockedReaction;
	REACTION_TYPE m_eRangeBlockedReaction;
	REACTION_TYPE m_ePowerBlockedReaction;
	REACTION_TYPE m_eSyncdSecondaryReaction;
	REACTION_TYPE m_eUnBlockedReactionFinalStrikeOverride;

	// Links to very specific reactions for these attacks
	CHashedString m_obReceiverAnim;				//This one will be the UnblockedReaction anim too.
	CHashedString m_obReceiverAnimSpeedBlocked;
	CHashedString m_obReceiverAnimRangeBlocked;
	CHashedString m_obReceiverAnimPowerBlocked;
	CHashedString m_obReceiverAnimSyncdSecondaryReaction;
	CHashedString m_obSpecificKOAnimation;
	bool m_bUseProceduralKO;

	// Not an anim, name of attack link to lookup
	SpecificCounterIndex *m_pobSpecificCounterIndex, *m_pobSpecificKillCounterIndex, *m_pobSpecificSmallCounterIndex, *m_pobSpecificSmallKillCounterIndex;

	// The time for which a character should be deflected - implemented as a window
	float m_fDeflectionTime;

	// At what angle relative to the character root should the strike
	// proximity check be performed
	float m_fStrike2ProximityCheckAngle;
	// Amount of angle to rotate the proximity check angle from start of strike2 window to the end
	float m_fStrike2ProximityCheckAngleCoverage; 

	// Describes the area that the strike covers
	float m_fStrike2ProximityCheckSweep;
	float m_fStrikeProximityCheckDistance;

	// Reference of the curve for the cool cam for this move to use
	TimeScalarCurve* m_pobTSCurve;

	// The animation for the recovery
	CHashedString m_obRecoveryAnimName;
	CHashedString m_obStrikeFailedRecoveryAnimName;

	// How long should people react to this attack for
	float		m_fRecoilTime;

	// How should this attack be reacted to?
	REACTION_APPEARANCE m_eReactionAppearance;

	// Should we definately use the previous target
	bool m_bHoldPreviousTarget;

	// Should we definately not use the previous target
	bool m_bExemptPreviousTarget;

	// Group combat lock on move
	bool m_bGroupCombatLockOnMove;

	// What is the movement type of this attack?
	ATTACK_MOVEMENT_TYPE m_eAttackMovementType;

	// How far can this attack go?
	float m_fMaxDistance;
	
	// What type of targeting is needed for this attack
	ATTACK_TARGET_TYPE m_eTargetType;

	// Stuff for syncronised attack movement
	bool					m_bAutoLink;
	COMBAT_REL_MOVE_TYPE	m_eSyncTransform;
	CHashedString			m_obSyncToLocatorName;
	CHashedString			m_obSyncReceiverAnim;
	bool					m_bInteractWithSyncTransform;
	CDirection				m_obInteractiveSyncTranslateAbsoluteSpeed;
	CDirection				m_obMaxInteractiveSyncTranslateSpeed;
	float					m_obInteractiveSyncRotationAbsoluteSpeed;
	float					m_obMaxInteractiveSyncRotationSpeed;
	bool					m_bReverseInteractiveStickInput;
	float					m_fInteractiveCollisionCheckDistance;
	float					m_fInteractiveCollisionCheckStartHeight;
	int						m_iInteractiveCollisionCheckHeightCount;
	float					m_fInteractiveCollisionCheckHeightInterval;

	// What effect scripts to run if we dont want defaults.
	ntstd::String m_obEffectsScript;

	// Are we able to direct a KO from this attack?
	bool m_bAftertouchableKO;
	bool m_bAerialComboStartKO;
	bool m_bJuggleKO;
	bool m_bTurnOffGravity;

	// Cone to ignore targets in
	float m_fStrikeProximityCheckExclusionDistance;

	CAttackCamera* m_pobAttackCamera;

	CHashedString m_obSuperSafeRotationAnimAttacker, m_obSuperSafeRotationAnimReceiver;
	CHashedString m_obSuperSafeTranslationAnimAttacker, m_obSuperSafeTranslationAnimReceiver;
	CHashedString m_obSuperSafeLinkupAnimAttacker, m_obSuperSafeLinkupAnimReceiver;
	bool m_bNeedsSuperStyleSafety;
	bool m_bNeedsToBeInContinueVolume;

	ntstd::List<CombatPhysicsStrikeVolumeDescriptor*, Mem::MC_ENTITY> m_obStrikeVolumeDescriptors;

	bool m_bOverrideInvulnerabilityWindow;

	bool m_bSkipFlooredAndRise;

	// Flag to see if we need an falling and impact anim in our KOs
	bool m_bSkipFallingMovement;

	float m_fStrikeProximityCheckAngle;
	float m_fStrikeProximityCheckSweep;

	bool m_bNoAnimationBlend;

	bool m_bTrackTargetThroughout;
	bool m_bCorrectDirectionWhenScaling;
	bool m_bUse3DScaling;
	bool m_bNoRotateIfTargetBehind;
private:

	// Total time for the attack
	float m_fAttackTime;
	float m_fAttackRecoverTime;
};


/***************************************************************************************************
*
*	CLASS			CAttackDefinition
*
*	DESCRIPTION		This is the root of all the attack data used by a single entity.
*
***************************************************************************************************/
class CAttackDefinition
{
public:

	// Construction destruction
	CAttackDefinition();
	~CAttackDefinition();

	// A pointer to a a clustered structure of attacking data
	CClusterStructure*	m_pobClusterStructure;

	// A factor to translate between block and strike attack areas
	float				m_fStrikeToBlockFactor;

	// What factor of the incapacity time does a single wiggle remove
    float				m_fWiggleReductionFactor;

	// Block definitions
	CAttackBlock*		m_pobSpeedBlock;
	CAttackBlock*		m_pobPowerBlock;
	CAttackBlock*		m_pobRangeBlock;

	// Deflection definitions
	ReactionAnimList*	m_pobPowerDeflections;
	ReactionAnimList*	m_pobSpeedDeflections;
	ReactionAnimList*	m_pobRangeDeflections;
	CHashedString			m_obRiseDeflectionAnim;

	// Stagger Definitions
	CHashedString			m_obSpeedImpactStaggerRecoverAnim,m_obRangeImpactStaggerRecoverAnim,m_obPowerImpactStaggerRecoverAnim;
	ReactionAnimList*	m_pobPowerImpactStaggers;
	ReactionAnimList*	m_pobSpeedImpactStaggers;
	ReactionAnimList*	m_pobRangeImpactStaggers;
    CHashedString			m_obSpeedBlockStaggerRecoverAnim, m_obRangeBlockStaggerRecoverAnim, m_obPowerBlockStaggerRecoverAnim;
	ReactionAnimList*	m_pobPowerBlockStaggers;
	ReactionAnimList*	m_pobSpeedBlockStaggers;
	ReactionAnimList*	m_pobRangeBlockStaggers;
	float				m_fBlockStaggerTime;
	float				m_fImpactStaggerTime;
	float				m_fMinBlockStaggerTime;
	float				m_fMinImpactStaggerTime;

	// Recoil definitions
	ReactionAnimList*	m_pobPowerRecoilsFront;
	ReactionAnimList*	m_pobPowerRecoilsBack;
	ReactionAnimList*	m_pobSpeedRecoilsFront;
	ReactionAnimList*	m_pobSpeedRecoilsBack;
	ReactionAnimList*	m_pobRangeRecoilsFront;
	ReactionAnimList*	m_pobRangeRecoilsBack;

	// Dying Definitions
	ReactionAnimList*	m_pobPowerDeathsFront;
	ReactionAnimList*	m_pobPowerDeathsBack;
	ReactionAnimList*	m_pobSpeedDeathsFront;
	ReactionAnimList*	m_pobSpeedDeathsBack;
	ReactionAnimList*	m_pobRangeDeathsFront;
	ReactionAnimList*	m_pobRangeDeathsBack;

	// KO Definitions
	CHashedString			m_obFrontAirKOAnim;
	CHashedString			m_obBackAirKOAnim;
	CAttackKO*			m_pobFrontKODefinition;
	CAttackKO*			m_pobBackKODefinition;
	ReactionAnimList*	m_pobPowerKOsFront;
	ReactionAnimList*	m_pobPowerKOsBack;
	ReactionAnimList*	m_pobSpeedKOsFront;
	ReactionAnimList*	m_pobSpeedKOsBack;
	ReactionAnimList*	m_pobRangeKOsFront;
	ReactionAnimList*	m_pobRangeKOsBack;
	float				m_fKOTime;
	float				m_fKOInstantRecoverTime;

	// Details about switching stances
	StanceSwitchingDef* m_pobStanceSwitchingDef;

	// Stuff for aerial movement
	AerialDetailsDef*	m_pobAerialDetails;

	// Does this character have a hit counter 
	HitCounterDef* m_pobHitCounterDef;

	// JML Added for Attack Cameras
	class CAttackCameraList* m_pobCameras;	

	// Details for the strike proximity checking
	float	m_fStrikeUpperHeight;
	float	m_fStrikeLowerHeight;
	float	m_fAerialStrikeUpperHeight;
	float	m_fAerialStrikeLowerHeight;

	// Effect stuff for the whole character
	CombatEffectsDefinition* m_pCombatEffectsDef;

	// Any definition stuff that is required to start a special
	SpecialDef*	m_pobSpecialDefinition;

	// Amount of time after counter window for player to make a punishably bad counter attempt
	float m_fBadCounterDetectTime;
	// Amount of time the player will not be able to attack after making a bad counter
	float m_fBadCounterPunishTime;

	// Counter timings, moved from being per attack in CAttackData into being per character here
	float m_fCounterTime;
	float m_fQuickCounterTime;

	// For grabs specifically
	float m_fGrabProximityCheckSweep;

	// Reaction matrix for this character
	ReactionMatrix* m_pobReactionMatrix;

	// Deflection time overrides for incoming attacks of different stances
	float m_fDeflectionTimeOverrideSpeed;
	float m_fDeflectionTimeOverrideRange;
	float m_fDeflectionTimeOverridePower;

	bool m_bCanAutoBlockGrabs;
	bool m_bCanAutoBlockSpeed, m_bCanAutoBlockPower, m_bCanAutoBlockRange;

	float m_fHeldAttackThresholdSpeed, m_fHeldAttackThresholdRange, m_fHeldAttackThresholdPower;

	REACTION_TYPE m_eStrikeProximityCheckExclusionDistanceReaction;
	CHashedString m_obStrikeProximityCheckExclusionDistanceReactionAnim;
	
	bool m_bIsGroundAttackable;
	bool m_bIsAerialable;
	bool m_bExemptFromRagdollKOs;

	// Specific case for the gladiatior axeman for the moment - specifies whether the 
	// entity is susceptible to head shots only with ranged weapons
	bool m_bOnlyHeadShots;

	// How the hit counter variables defined in the hero effect this entity
	HitCounterCharacterMulDef*	m_pobHCMultiplierDef;

	// Push volumes for when she's attacking and not attacking
	CombatPhysicsPushVolumes* m_pobCombatPhysicsPushVolumesAttacking;
	CombatPhysicsPushVolumes* m_pobCombatPhysicsPushVolumesNonAttacking;

	// Zones of vulnerability to specific attacks - not fully tested yet
	ntstd::List<SpecificAttackVulnerabilityZone*, Mem::MC_ENTITY> m_obSpecificVulnerabilities;

	// How long we remember (and prioritise) the entity we last started juggling
	float m_fJuggleTargetMemoryTime;

	// For checking if it's ok to start a proper counter
	CPoint m_obCounterAttackEnvironmentCheckHalfExtents;

	CAttackMovementBlendInTimes* m_pobAttackMovementBlendInTimes;

	// Ledge recover values
	bool m_bCanLedgeRecover;
	float m_fLedgeRecoverRequiredHeight;

	//Skill-evades require a time-margin where you can evade (before the first strike window), and a box-check
	//to see whether we want to do close (on-the-spot) synchronised skill-evade attacks, or wider-area ones.
	float m_fSkillEvadeRadius;
	float m_fSkillEvadeMargin;
	float m_fSuperSkillEvadeMargin;
	CPoint m_obSkillEvadeAttackEnvironmentCheckHalfExtents;
};


/***************************************************************************************************
*
*	CLASS			CAttackBlock
*
*	DESCRIPTION		A basic block is a three stage movement.  This movement simply represents the 
*					the duration of a blocking state.  A player will react differently to strikes
*					when in the block state
*
***************************************************************************************************/
class CAttackBlock
{
public:

	// Construction destruction
	CAttackBlock();
	~CAttackBlock();

	// The three animiser stages
	CHashedString m_obLoopAnimName;
	CHashedString m_obEnterAnimName;
	CHashedString m_obExitAnimName;
};


/***************************************************************************************************
*
*	CLASS			CAttackKO
*
*	DESCRIPTION		A set of movements for the a KO reaction
*
***************************************************************************************************/
class CAttackKO
{
public:

	// Construction destruction
	CAttackKO();
	~CAttackKO();

	// The animations
	CHashedString m_obFallAnimName;
	CHashedString m_obFlooredAnimName;
	CHashedString m_obWaitAnimName;
	CHashedString m_obRiseAnimName;
	CHashedString m_obIRecoverAnimName;
	CHashedString m_obGroundAttackRiseAnimName;
};

/***************************************************************************************************
*
*	CLASS			CAttackComponent
*
*	DESCRIPTION		This class is getting fairly unmanagable.  I think it could be simplified and
*					split apart - i have tried to show what the real external interface is so there
*					are lots of critiquing notes about at the moment, just noting my thought process
*
*					Er...since i wrote that note the interface has grown a lot.  Things are getting
*					very messy indeed.
*
***************************************************************************************************/
class CAttackComponent : public Combat_Lua
{
public:

	// Construction destruction
	CAttackComponent( Character& obParentEntity, CAttackDefinition* pobAttackDefinition );
	~CAttackComponent();

	HAS_LUA_INTERFACE();

	// Update the attack
	void Update( float fTimeDelta );

	// This is called by opponents - the strike stack is mutable because they are only allowed a const 
	// pointer to us.  Makes perfect sense to me.
	void ReceiveStrike( CStrike* pobStrike ) const;

	// Also required by networking...
	const CStrike* GetCurrentStrike() const { return m_pobMyCurrentStrike; }
	const CEntity* GetCurrentTargetP( void ) const { return m_pobMyCurrentStrike ? m_pobMyCurrentStrike->GetTargetP() : 0; }

	// Switch the component on and off
	void SetDisabled( bool bDisabled );
	bool GetDisabled( void ) const { return m_bExternallyDisabled; }

	// Targeting disabled?
	void SetTargetingDisabled( bool bState ) { m_bTargetingDisabled = bState; }
	bool GetTargetingDisabled( void ) const { return m_bTargetingDisabled; }

	// To be used for targeting
	bool CanTakeStandardHit( void ) const;
	bool CanTakeSpeedExtraHit( void ) const;

	// For superstyle action
	//bool CharacterSuitableForSuperStyle( void ) const;
	bool CharacterUninterruptibleVulnerableTo(ATTACK_CLASS eAttackClass) const;

	// Stuff used for audio - nasty test code - should be removed ASAP
	bool Audio_Access_HitByMainCharacter( void ) const;
	bool Audio_Access_HitByPowerAttack( void ) const;
	bool Audio_Access_HitByRangeAttack( void ) const;
	bool Audio_Access_HitBySpeedAttack( void ) const;
	bool Audio_Access_GetHitByKick( void ) const;

	// For counting successful hits - should these be messages?  ...along with strike passing?
	// I reckon - since we have all kinds of const nastyness issues here
	void PlayerCausedKO( const CStrike* pobStrike ) const;
	void PlayerCausedRecoil( const CStrike* pobStrike ) const;
	void PlayerCausedImpactStagger( const CStrike* pobStrike ) const;
	void PlayerCausedDeath( const CStrike* pobStrike ) const;

	// Exposed to the character state system - these calls are not to be called directly by other components
	// The interface to the combat needs some work in order to sort out the AI and interactive objects.  These
	// functions should be taken as private to all but the bindings and the message handler system - GH
	bool StartNewAttack( void );
	void SelectNextAttack( void );
	void CompleteRecovery( void );
	void StartFlooredState();
	void AffirmStanceNextUpdate(void) { m_bAffirmStanceNextUpdate = true; }
	bool MoveInPowerStance( void ) const { return ( ( m_eCurrentStance == ST_POWER ) && ( m_pobAttackDefinition->m_pobStanceSwitchingDef ) ); }
	bool MoveInRangeStance( void ) const { return ( ( m_eCurrentStance == ST_RANGE ) && ( m_pobAttackDefinition->m_pobStanceSwitchingDef ) ); }
	bool MoveInSpeedStance( void ) const { return m_eCurrentStance == ST_SPEED; }
	void EndSyncdReaction( void );
	void AllowSwitchStance( void ) { m_bCanExecuteStanceChangeMovement = true; }
	void DisallowSwitchStance( void ) { m_bCanExecuteStanceChangeMovement = false; }
	STANCE_TYPE CurrentStance( void) { return m_eCurrentStance; }
	void MakeDead( void );
	void MakeUndead()     {SetState(CS_STANDARD);}

	// These calls need to be made by AI based characters on startup - ideally we won't need
	// different internal settings in the future - its a sign that things are structured wrongly
	void	AI_Setup_SetToDirectRequestMode( void ) { m_obAttackTracker.SetToDirectMode(); }
	void	AI_Setup_RemoveAutoBlock( void ) { /*m_bAutoBlock = false;*/ }

	// This is the full set of commands that the AI should ever use on this component
	bool	AI_Command_RequestBlock( BLOCK_TYPE eBlockType, float fBlockTime );
	bool	AI_Command_LeaveBlock( void );
	bool	AI_Command_RequestAttack( ATTACK_MOVE_TYPE eattackMoveType, bool bForceToLockon = false, const CAttackComponent* pTarget = NULL, const CDirection& obEvadeDirection = CDirection( 0.0f, 0.0f, 1.0f ) ); 	
	bool	AI_Command_RequestDirectAttack( CHashedString pcName ); 	
	bool	Boss_Command_RequestDirectAttack( const CAttackLink* pobLink, bool bForce = false ); 
	bool	Boss_Command_RequestDirectNextAttack( const CAttackLink* pobLink );
	bool	AI_Command_DoInstantKORecovery( void );
	bool	AI_Command_SetStaggerTime( float fTime );
	bool	AI_Command_Breakout( void );

	// Accessors added specifically for the AI component
	float				AI_Access_GetAttackScalar( void ) const { return m_fAttackScalar; }
	ATTACK_CLASS		AI_Access_GetStruckStrikeAttackType( void ) const;
	int					AI_Access_GetStringDepth( void ) const { return m_iStringDepth; }
	bool				AI_Access_HasStrikeLanded( void ) const { return m_bStrikeLanded; }
	bool				AI_Access_IsPerformingCounter( void ) const { return m_pobMyCurrentStrike ? m_pobMyCurrentStrike->IsCounter() : false; }
	float				AI_Access_GetStaggerTime( void ) const;
	COMBAT_STATE		AI_Access_GetLandedStrikeResponse( void ) const;
	COMBAT_STATE		AI_Access_GetState( void ) const { return m_eCombatState; }
	bool				AI_Access_IsInCSStandard( void ) const { return (AI_Access_GetState() == CS_STANDARD); }
	float				AI_Access_GetTimeBeforeStrike( void ) const;
	float				AI_Access_GetTimeUntilCurrentAttackComplete(void) const;
	const CAttackData*	AI_Access_GetCurrentAttackDataP( void ) const;
	const CAttackLink*	AI_Access_GetLeadAttackLink( void ) const { return m_pobAttackDefinition->m_pobClusterStructure->m_pobLeadCluster; }
	//const CAttackLink*	AI_Access_GetCounterAttackLink( void ) const { return m_pobAttackDefinition->m_pobClusterStructure->m_pobSlowCounterCluster; }
	const CAttackLink*	AI_Access_GetInterceptAttackLink( void ) const { return m_pobAttackDefinition->m_pobClusterStructure->m_pobInterceptCluster; }
	const CAttackLink*	AI_Access_GetCurrentAttackLink( void ) const { return !m_pobMyCurrentStrike ? 0 : m_obAttackTracker.GetCurrentAttackLinkP(); }
	bool				AI_Access_CanRequestAttack( void ) const;
	bool				AI_Access_CanBreakAway( void ) const;
	bool				AI_Access_CanBlock(BLOCK_TYPE eCheckBlockType) const { return CanBlock(eCheckBlockType); }
	float				AI_Access_GetPostStrikeRecovery(void) const;
	const CAttackLink*  AI_Access_QueryAttackLink( ATTACK_MOVE_TYPE eMoveType, float fLockOnRange, const GameGUID* pPrevGUID ) const 
	{
		return m_obAttackTracker.AIQueryAttackLink( eMoveType, fLockOnRange, pPrevGUID );
	}
	bool				AI_Access_AttackPending() const { return m_obAttackTracker.GetRequestedAttackDataP() != NULL; }
	int					AI_Access_GetAttackerCount() const { return m_obAIAttackList.size(); }
	float				AI_Access_TimeSinceLastDeflect() const { return m_fTimeSinceLastDeflect; }
	const CStrike*		AI_Access_GetStruckStrike() const { return m_pobStruckStrike; }
	const CStrike*		AI_Access_GetCurrentStrike() const { return m_pobMyCurrentStrike; }
	int					AI_Access_GetStrikeStackSize() const { return m_obStrikeStack.size();  }
	
	// Accessor to the data definition of this combat component
	const CAttackDefinition* GetAttackDefinition( void ) const { return m_pobAttackDefinition; }

	// Specials
	bool StartSpecial( void );
	bool IsDoingSpecial( void ) const;

	// For attack debugger and rotating strike proximity angles
	float GetAttackStrikeProximityCheckAngle(int iStrike) const;
	float GetAttackStrikeProximityCheckDistance(void) const;
	float GetAttackStrikeProximityCheckExclusionDistance(void) const;
	float GetAttackStrikeProximityCheckSweep(int iStrike) const;
	void SetAutoCounter( bool bAutoCounter ) { m_bAutoCounter = bAutoCounter && this->m_pobParentEntity->IsPlayer(); }

	// How long have I been in whatever state I'm in?
	float GetStateTime() const { return m_fStateTime; }

	// KO Aftertouch
	void SetKOAftertouch (bool bEnable) { m_bKOAftertouch=bEnable; }
	bool InKOAftertouch () const { return m_bKOAftertouch; }

	// Aerial
	void AerialTargetingNotify();
	void AerialTargetingCancel();
	const CStrike* GetStruckStrike() const { return m_pobStruckStrike; }

	// Register a CombatEventLog of this attack component
	void RegisterCombatEventLog(CombatEventLog* pobLog) { m_pobCombatEventLogManager->RegisterEventLog(pobLog); };
	void UnRegisterCombatEventLog(CombatEventLog* pobLog) { m_pobCombatEventLogManager->UnRegisterEventLog(pobLog); };

	// Safe access to the HitCounter
	HitCounter* GetHitCounter() const;
	
	// A global to enable and disable autoblock
	static bool m_bGlobablAutoBlockEnable;

	// Getters and setters for the new ragdoll transitions, allows them to set which anims we play according to their final resting position
	void NotifyWasFullyRagdolled() { m_bWasFullyRagdolledInKO = true; }
	void NotifyHeldVictimRagdolled();
	REACTION_ZONE GetStruckReactionZone() const { return m_eStruckReactionZone; }
	void SetStruckReactionZone(REACTION_ZONE eRZ) { m_eStruckReactionZone = eRZ; }

	// Called by SuperStyleSafetyTransition when it's entity is done moving - decides when to allow next autolink from grab goto
	void NotifyDoneSuperStyleSafetyTransition(const CEntity* pobEnt);
	void ForceDoneSuperStyleSafetyTransition();
	bool IsInSuperStyleSafetyTransition() const { return m_bIsDoingSuperStyleSafetyTransition; };
	CEntity* GetSuperStyleSafetyAttacker() { return m_pobSuperStyleSafetyAttacker; };

	// Because ragdolls can be buggy, exempt some entities from being put in ragdoll in KOs
	void SetExemptFromRagdollKOs(bool bExemption) { m_bEntityExemptFromRagdollKOs = bExemption; };
	bool GetExemptFromRagdollKOs() { return m_bEntityExemptFromRagdollKOs; };

	/*******************************************************************************
	*	HELPERS FOR QUERYING ATTACK DATA - now public cos they're useful
	*******************************************************************************/

	int RobustIsInWindow( const CFlipFlop& obWindow ) const;

	int IsInNextMoveWindow( void ) const		{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obNextMove ); }
	int IsInStrikeWindow( void ) const			{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike ); }
	int IsInStrike2Window( void ) const			{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obStrike2 ); }
	int IsInAttackWindow( void ) const			{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obAttackPopOut ); }
	int IsInMovementWindow( void ) const		{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obMovementPopOut ); }
	int IsInBlockWindow( void ) const			{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obBlockPopOut ); }
	int IsInNoCollideWindow( void ) const		{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obNoCollideWindows ); }
	int IsInRangeInterceptWindow( void ) const	{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obRangeInterceptWindow ); }
	int IsInInterceptWindow( void ) const		{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obInterceptWindow ); }
	int IsInNoLockWindow( void ) const			{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obNoLockWindows ); }
	int IsInInvulnerabilityWindow( void ) const	{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obInvulnerabilityWindow ); }
	int IsInUninterruptibleWindow( void ) const	{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obUninterruptibleWindow ); }
	int IsInEvadeAndGrabDenyWindow( void ) const	{ return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obEvadeAndGrabDenyWindow ); }
	int IsInVictimRagdollableWindow( void ) const { return ( !m_pobMyCurrentStrike ) ? 0 : RobustIsInWindow( m_pobMyCurrentStrike->GetAttackDataP()->m_obVictimRagdollableWindow ); }

	const CAttackLink* CAttackComponent::CheckSpecificAttackVulnerabilityZones(CEntity* pobAttacker, SpecificAttackVulnerabilityZone** pobVulnerabilityZone);

	bool ForceAttack(const CAttackLink* pobAttackLink);
	bool IsInForcedMode() const { return m_bForcedModeEnabled; }

	bool GetNeedsHintForButton(VIRTUAL_BUTTON_TYPE eButton) const;
	void SetNeedsHintForButton(VIRTUAL_BUTTON_TYPE eButton, bool bNeedsHint);

	void SetDefaultLeadClusterTo(CClusterStructure* pobCluster);
	void ChangeLeadClusterTo(CClusterStructure* pobCluster);
	void ResetLeadCluster();

	bool HasSelectedANextMove() const;

	bool GetCurrentAttackTurnOffGravity() const;

	bool GetLeadClusterChanged() const { return m_bLeadClusterChanged; };

	float GetDistanceToClosestStrikeVolume(CPoint& obFromPosition) const;

	void SetEnableStrikeVolumeCreation( bool bEnable ) { m_bEnableStrikeVolumeCreation = bEnable; };

	int GetDepthInRecoiling() const { return m_iDepthInRecoiling; };
	int GetDepthInDeflecting() const { return m_iDepthInDeflecting; };

	// Slightly hacktastic way for ragdoll blend in advancedcharactercontroller.cpp to get access to the floored blend time
	const CAttackMovementBlendInTimes* GetAttackMovementBlendInTimes() { return m_obAttackMovement.GetAttackMovementBlendInTimes(); };

	void SetMovementFromCombatPoseFlag(bool bMovementFromCombatPoseFlag) { m_bMovementFromCombatPoseFlag = bMovementFromCombatPoseFlag; };
	bool GetMovementFromCombatPoseFlag() const { return m_bMovementFromCombatPoseFlag; };

	bool GetCannotDieIntoRagdoll() { return m_bCannotUseRagdoll; };

	void NotifyNeedsLedgeRecover() { m_bNeedLedgeRecover = true; };
	bool CanLedgeRecover() { return m_pobAttackDefinition->m_bCanLedgeRecover && m_obAttackTracker.CanLedgeRecover(); };
	bool IsDoingLedgeRecover() const { return m_eCombatState == CS_ATTACKING && m_bNeedLedgeRecover; };
	CPoint GetLastKOPosition() { return m_obLastKOPosition; };
	float GetLedgeRecoverRequiredHeight() { return m_pobAttackDefinition->m_fLedgeRecoverRequiredHeight; };

	void NotifyDieOutOfCurrentMovement();

	void SetInvulnerableToNormalStrike( bool bInv ) { m_bInvulnerableToNormalStrike = bInv; };
	void SetInvulnerableToSyncStrike( bool bInv ) { m_bInvulnerableToSyncStrike = bInv; };
	void SetInvulnerableToProjectileStrike( bool bInv ) { m_bInvulnerableToProjectileStrike = bInv; };
	void SetInvulnerableToCounterStrike( bool bInv ) { m_bInvulnerableToCounterStrike = bInv; };

	bool GetCanHeadshotThisEntity() { return m_bCanBeHeadshot; };
	void SetCanHeadshotThisEntity( bool bHead ) { m_bCanBeHeadshot = bHead; };
private:
	bool m_bCanBeHeadshot;
	bool m_bInvulnerableToNormalStrike, m_bInvulnerableToSyncStrike, m_bInvulnerableToProjectileStrike, m_bInvulnerableToCounterStrike;

	bool m_bNotifiedToDieOutOfCurrentMovement;

	CPoint m_obLastKOPosition;
	bool m_bNeedLedgeRecover;

	// Flag to show we need to do a purely animated ko/death, otherwise PS3 will die horribly in a pool of it's own underperforming filth.
	bool m_bCannotUseRagdoll;
	bool m_bUseProceduralKODeath; //< use pure ragdoll ko or death... 

	bool m_bMovementFromCombatPoseFlag;

	bool m_bLeadClusterChanged;

	// Some general combat settings
	static const float m_fAutoBlockLeadTime;
	static const float m_fAutoRiseTime;

	// A Pointer to our parent character - can only be attached to character
	Character* m_pobParentEntity;

	// Our debugging friend - he won't do anything nasty
	friend class AttackDebugger;
	friend class Combat_Lua;

	// A pointer to the root of combat definition structure
	const CAttackDefinition* m_pobAttackDefinition;

	// A hit counter to monitor the progress of the player
	HitCounter*		m_pobHitCounter;

	// Our major sub components
	CAttackTracker		m_obAttackTracker;
	CAttackMovement		m_obAttackMovement;

	// Have we requested a cool cam because we've held the button down?
	int  m_iCombatCam;

	// Has this component been switched off?
	bool m_bExternallyDisabled;

	// Targeting diabled. 
	bool m_bTargetingDisabled;

	// for effects handling
	u_int	m_iCurrEffectsTrigger;

	/*******************************************************************************
	*	FOR MANAGING STRIKES
	*******************************************************************************/

	// For generating strikes
	const CStrike*	BuildStrikeFromData( bool bCounter, bool bAutoLink );
	const CStrike*	BuildStrikeForTarget( const CAttackData* pobAttackData, const CEntity* pobTarget, bool bCounter );
	const CStrike*	BuildStrikeFromInput( const CAttackData* pobAttackData );
	bool			SuitableForToLockon( const CAttackData* pobAttackData, bool bCounter );

	// Can we track our target
	bool CanTargetAttack( void ) const;

	// What are we currently trying to do
	const CStrike* m_pobMyCurrentStrike;

	// For updating strike data
	bool GenerateMyStrike( bool bCounter, bool bAutoLink );

	/*******************************************************************************
	*	Standard State
	*******************************************************************************/

	void UpdateStandard( float fTimeDelta );

	/*******************************************************************************
	*	SINGLE ATTACKS
	*******************************************************************************/

	bool StartAttack( void );
	void UpdateAttack( float fTimeDelta );
	void EndAttack( void );

	void ResetAttack(bool bResetCamera);
	bool CanExecuteNextAttack();

	float m_fAttackScalar;			// A scalar for the attack time
	CDirection m_obEvadeDirection;	// What direction was an evade requested in

	/*******************************************************************************
	*	GENERAL REACTIONS
	*******************************************************************************/

	CHashedString				GetSpecificResponse( const CHashedString& obResponse );
	static REACTION_ZONE	ChooseReactionZone( const CEntity* pobEntFrom, const CStrike* pobStrike );
	void					StartReaction( REACTION_TYPE eReactionType, const CStrike* pobStrike, REACTION_MATRIX_LOOKUP eRML );

	const CStrike*	m_pobStruckStrike;			// What were we struck with
	REACTION_ZONE	m_eStruckReactionZone;		// How did it strike us
	float			m_fIncapacityTime;			// How long are we incapacitated
	float			m_fCounteringTime;			// How long do we have to counter the attack
	float			m_fQuickCounteringTime;		// Can we counter really quickly with a more powerful attack

	/*******************************************************************************
	*	STRINGS
	*******************************************************************************/

	bool StartString( void );
	void EndString( void );
	bool IsInCombo( void ) const { return m_iStringDepth > 1; }

	int m_iStringDepth;			// The current depth into the string

	/*******************************************************************************
	*	RECOILS
	*******************************************************************************/

	void StartRecoil( const CStrike* pobStrike, CHashedString obChosenReceiverAnim );
	void UpdateRecoil( float fTimeDelta );
	void EndRecoil( void );

	/*******************************************************************************
	*	HOLDING
	*******************************************************************************/

	void StartHeld( const CStrike* pobStrike, CHashedString obChosenReceiverAnim );
	void UpdateHeld( float fTimeDelta );
	void EndHeld( void );

	/*******************************************************************************
	*	BLOCK-STAGGERS
	*******************************************************************************/

	void StartBlockStagger( const CStrike* pobStrike, CHashedString obChosenReceiverAnim );
	void UpdateBlockStagger( float fTimeDelta );
	void EndBlockStagger( void );

	/*******************************************************************************
	*	IMPACT-STAGGERS
	*******************************************************************************/

	void StartImpactStagger( const CStrike* pobStrike, CHashedString obChosenReceiverAnim );
	void UpdateImpactStagger( float fTimeDelta );
	void EndImpactStagger( void );

	/*******************************************************************************
	*	KOS
	*******************************************************************************/

	void		StartKO( const CStrike* pobStrike, CHashedString obChosenReceiverAnim, bool bFromHeld = false );
	void		UpdateKO( float fTimeDelta );
	CHashedString	FindKOHitAnim( REACTION_ZONE eReactionZone, const CStrike* pobStrike, COMBAT_STATE eCurrentState, bool bUseSpecific,
		CHashedString obChosenReceiverAnim);

	bool m_bQuickRecover;
	bool m_bKOAftertouch;

	/*******************************************************************************
	*	FLOORED
	*******************************************************************************/

	void		UpdateFloored( float fTimeDelta );
	void		EndFloored();

	/*******************************************************************************
	*	RISE WAIT
	*******************************************************************************/

	void		UpdateRiseWait( float fTimeDelta );

	/*******************************************************************************
	*	BLOCKING
	*******************************************************************************/

	void StartBlock( const CStrike* pobStrike, BLOCK_TYPE eBlockType, float fDefaultBlockTime = 0.0f );
	void UpdateBlock( float fTimeDelta );
	void EndBlock( void );
	bool MayAutoBlockAttack( ATTACK_CLASS eAttackClass ) const;
	bool CanBlock( BLOCK_TYPE eBlockType ) const;
	BLOCK_TYPE AttackClassToBlockType(ATTACK_CLASS eAttackClass) const;

	//bool m_bAutoBlock;
	BLOCK_TYPE m_eBlockType;

	/*******************************************************************************
	*	DEFLECTING
	*******************************************************************************/

	void StartDeflecting( const CStrike* pobStrike, CHashedString obChosenReceiverAnim );
	void UpdateDeflecting( float fTimeDelta );
	void EndDeflecting( void );

	/*******************************************************************************
	*	DYING
	*******************************************************************************/

	void UpdateDying( float fTimeDelta );

	/*******************************************************************************
	*	RECOVERY
	*******************************************************************************/

	// void StartRecovery( void );
	void UpdateRecovery( float fTimeDelta );
	void EndRecovery( bool bLeaveStrike = false );

	RECOVERY_TYPE	m_eRecoveryType;

	/*******************************************************************************
	*	COMBAT STATES
	*******************************************************************************/

	void SetState( COMBAT_STATE eCombatState, const CStrike * pobStrike = NULL );
	void SetRagdollState( COMBAT_STATE eTo, const CStrike * pobStrike = NULL );

	COMBAT_STATE m_eCombatState;		// The current combat state
	float m_fStateTime;					// The current state time

	/*******************************************************************************
	*	STRIKE MANAGEMENT
	*******************************************************************************/

	bool			CanAttackProceedTargetless(const CAttackData* pobAttackData);
	bool			GenerateDirectStrike( const CEntity* pobOtherEntity, bool bPreStrike, bool bSyncronise, bool bSuperSafety = false );
	void			GenerateIncidentalStrike( const CEntity* pobOtherEntity, bool bPreStrike );
	void			MakeStrike( const CEntity* pobOtherEntity, bool bPreStrike, bool bSyncronise, bool bIncidental, bool bWithinExclusionDistance );
	const CEntity*	ProximityTestStrikeTarget( const CEntity* pobOtherEntity, float fRangeMultiplier );
	void			UpdateStrikeStack();
	bool			ProcessStrike(CStrike* pobStrike);
	bool			ProcessPreStrike(CStrike* pobStrike);
	void			StrikeLanded() const;
	bool			AttackIsValidCounter( ATTACK_CLASS eCurrentAttack, ATTACK_CLASS ePossibleCounter );

	ntstd::List<const CEntity*> m_obStrikeList;		// A list of entities against which strikes have been sent
	ntstd::List<const CEntity*> m_obPreStrikeList;	// A list of entities against which pre strikes have been sent
	mutable ntstd::List<CStrike*> m_obStrikeStack;	// The list of strikes that have been given to this component
	bool m_bPreStrikeRequested;					// Flag whether a strike has already been requested... temporary
	int m_iStrikeWindow;						// Which of the strike windows have we used?
	mutable bool m_bStrikeLanded;				//GILES - needs to become deterministic? The  number of strikes that have been successfully recieved (not sent) from this attack
	bool m_bKOSuccessful;						// Was this attack a successful KO?


	/*******************************************************************************
	*	AI Targeting. 
	*******************************************************************************/
	bool AddAIAttacker( const CEntity*, float fAttackTime, bool bAdd = false ) const;

	// List of entities that are currently attacking the parent
	mutable ntstd::List< ntstd::pair<const CEntity*, float> > m_obAIAttackList;	

	// The time that the entity hasn't been under attack
	mutable float  m_fTimeSinceLastDeflect;

	/*******************************************************************************
	*	KILLING
	*******************************************************************************/

	void StartKill( const CStrike* pobStrike, CHashedString obChosenReceiverAnim, bool bIsReaction = false );

	/*******************************************************************************
	*	COMBAT EFFECTS -	maybe should just send messages so this is dealt with 
	*						elsewhere.
	*******************************************************************************/

	void DoHitEffect( const CEntity* pAttacker );
	void DoHitSound ( const CStrike* pobStrike );
		
	/*******************************************************************************
	*	STANCES
	*******************************************************************************/

	// This needs to be moved into Lua but does mean that we are only accessing the input component in one place
	// void UpdateRequestedStance( void );
	bool SwitchStance( STANCE_TYPE eNewStance );

	// Our current stance
	STANCE_TYPE m_eCurrentStance;

	// Make sure we're using the correct stance. 
	bool		m_bAffirmStanceNextUpdate;

	void UpdateCurrentStance( void );
	void UpdateStanceSwitch( float fTimeDelta );

	// Can this character execute a stance change
	bool m_bCanExecuteStanceChangeMovement;

	/*******************************************************************************
	*	STYLE METER - TO BE EXTENDED - A LOT
	*******************************************************************************/

	HIT_LEVEL m_eHitLevel;

	/*******************************************************************************
	*	OTHER BITS AND BOBS
	*******************************************************************************/
	
	// Do some wiggling to increase the state time
	void CheckWiggleAction( float fIncapacityTime, float& m_fWiggleGetBack );

	// How much time has the user got back by wiggling
	float m_fWiggleGetBack;

	/*******************************************************************************
	*	AERIAL COMBOS
	*******************************************************************************/

	bool	m_bAerialCombo;
	int     m_iAerialCoolCamID;
	// Flag to indicate if we should pause in mid air waiting for aerial
	bool	m_bGoingToBeAerialed;

	/*******************************************************************************
	*	SYNCRONISED MOVEMENT
	*******************************************************************************/

	int			m_iSyncdDepth;
	Transform*	m_pobSyncdTransform;

	/*******************************************************************************
	*	STUFF FOR 'EXTRA' ATTACKS
	*******************************************************************************/

	bool		m_bIsScalingTime;

	/******************************************************************************
	*	FOR MONITORING SPECIALS
	******************************************************************************/
	
	void ProcessAttackeeList( void );
	void ClearAttackeeList( void );

	AttackSpecial*		m_pobAttackSpecial;		// A component for special moves
	ntstd::List<CEntity*>	m_obAttackeeList;		// Who have we hit in this string?

	/******************************************************************************
	*	FOR MANAGING BAD COUNTER PUNISHMENT
	******************************************************************************/
	
	bool	m_bInBadCounterDetectWindow;	// Indicates when we should be looking out for bad counters
	bool	m_bBadCounterBeingPunished;		// Indicates when we have detected a bad counter and should punish
	// Both bools set to true indicates that a bad counter window has been and gone without any bad counters
	// Both bools set to false indicates that a bad counter window is ready to be started
	float	m_fTimeInBadCounter;	// Float to hold time so far in bad counter window or in punishment
	void	UpdateBadCounter(float fTimeDelta);
	// Track the last attack we were doing
	CHashedString m_pcStruckStrikeString;
	CHashedString m_pcOldStruckStrikeString;

	/******************************************************************************
	*	FOR ROTATING STRIKE PROXIMITY CHECKS
	******************************************************************************/
	// Holds the degrees per second amount that a rotating strike proximity check needs in GetAttackStrikeProximityCheckAngle
	float	m_fCurrentAttackStrikeProximityCheckAngleDelta;

	/******************************************************************************
	*	GENERAL BITS AND PIECES
	******************************************************************************/

	// Switch to remember if health after instant recover opportunity has been taken off
	//bool m_bUpdatedInstantRecoverHealth;

	// Get damage values, taking into account if it's incidental
	float GetDamageFromStrike(const CStrike* pobStrike) const;

	// KO targeting other entities like tables etc
	CDirection m_obKOTargetVector;
	float m_fKOSpecificAngleToTarget;

	// Automatically take opportunities to counter?
	bool m_bAutoCounter;

	// Keep a check on whether we've started a camera
	bool m_bCombatCamStartedThisAttack;

	// For gracefully handling being hit out of a sync attack
	void InterruptSyncAttack();
	bool m_bSyncAttackInterrupted;

	// Indicates if we're in the first attack out of an evade, it is cleared after the first attack is ended
	bool m_bEvadeCombo;

	// Testing for new HUD
	CombatEventLogManager* m_pobCombatEventLogManager;

	// Flag for special rising recovery
	bool m_bGroundAttackRecover;

	float m_fAttackButtonTime;
	void UpdateButtonHeldAttack(float fTimeDelta);
	bool m_bNewButtonHeldAttack;

	int m_iHighComboReported;

	const CEntity* m_pobInstantKORecoverTargetEntity;
	const CEntity* m_pobSkillEvadeTargetEntity;

	bool m_bWasFullyRagdolledInKO;
	
	bool m_bIsDoingSuperStyleSafetyTransition; // Am I moving to a safe place to continue a super?
	CEntity *m_pobSuperStyleSafetyAttacker, *m_pobSuperStyleSafetyReceiver; // Who's who in our super style transition
	bool m_bSuperStyleSafetyAttackerDone, m_bSuperStyleSafetyReceiverDone; // Administrative flags for the attacker, not used by the receiver

	CPoint m_obHitPoint;

	// Push volume stuff

	// Push volumes for when she's attacking and not attacking
	CombatPhysicsPushVolumesData * m_pobCombatPhysicsPushVolumesDataAttacking;
	CombatPhysicsPushVolumesData * m_pobCombatPhysicsPushVolumesDataNonAttacking;

	bool m_bSetupCombatPhysicsPushVolumesOnFirstUpdate;
	void SetCombatPhysicsPushVolumes(bool bAttacking);
	void CleanupCombatPhysicsPushVolumes();

	// Check volumes for camera starts next to walls
	CPoint m_obCounterCameraCheckVolumeHalfExtents, m_obCounterAttackCheckVolumeHalfExtents, m_obNonSafetySuperCheckVolumeHalfExtents;
	
	// Flag for rejecting all ragdoll activations in attacks
	bool m_bEntityExemptFromRagdollKOs;

	// Time to wait till we reset under attack notification (only for bosses atm)
	float m_fTimeTillNotifyNotUnderAttack;

	// Which buttons need hints rendered
	bool m_abHintForButton[AB_NUM];

	// The currently active strike volumes, if any
	ntstd::List<CombatPhysicsStrikeVolume*> m_obActiveStrikeVolumes;
	
	// Useful for forcing attacks through the network
	bool m_bForcedModeEnabled;

	// Vars for keeping track of juggling
	const CEntity* m_pobLastJuggledEntity;
	float m_fJuggleTargetMemoryTimeSoFar;

	// Helper to do combat physics strike volumes
	void UpdateCombatPhysicsStrikeVolumes(float fTimeDelta);

	// Helper to do juggling memory
	void UpdateJugglingMemory(float fTimeDelta);

	mutable bool	m_bAttackCollisionBreak;

	CClusterStructure* m_pobDefaultLeadCluster;

	bool m_bEnableStrikeVolumeCreation;

	bool m_bSpecialStarted;

	u_int m_iCurrentStrikeVolumeEffectsTrigger;

	SuperStyleSafetyVolume* m_pobSuperStyleStartVolume;

	bool m_bPreviousAttackWasSynchronised;

	int m_iDepthInDeflecting, m_iDepthInRecoiling;

	CDirection m_obTargetPointOffset;
public:
	// Access to the attack flags to control close combat stuff
	bool GetAttackCollisionBreak( void ) const { return m_bAttackCollisionBreak; }
	void SetAttackCollisionBreak( bool bAttackCollisionBreak ) const { m_bAttackCollisionBreak = bAttackCollisionBreak; }
};

LV_DECLARE_USERDATA(CAttackComponent)

#endif //_ATTACKS_H
