//------------------------------------------------------------------------------------------
//!
//!	\file awareness.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_AWARENESS_H
#define	_AWARENESS_H

// Necessary includes
#include "editable/enumlist.h"
#include "lua/ninjalua.h"

#include "game/awareness_lua.h"
#include "game/query.h"

// Forward declarations
class CEntity;
class CAttackTargeting;
class AttackDebugger;

//------------------------------------------------------------------------------------------
//!
//!	CAttackTargetingData
//!	Editable data for deciding how the awareness component behaves.  Probably this should go
//! and the awareness component should become more static - maybe not even a component 
//! at all.
//!
//------------------------------------------------------------------------------------------
class CAttackTargetingData
{
public:
	// Construction destruction
	CAttackTargetingData( void );
	~CAttackTargetingData( void );

	// For super close automatic lockon
	float m_fAutoLockOnAngle;
	float m_fAutoLockOnDistance;

	// For general awareness
	float m_fAwareAngle;
	float m_fAwareDistance;

	// Standard attack lockon
	float m_fToLockOnAngle;
	float m_fToLockOnDistance;

	// Medium distance attack lockon
	float m_fMediumLockOnAngle;
	float m_fMediumLockOnDistance;

	// Long distance attack lockon
	float m_fLongLockOnAngle;
	float m_fLongLockOnDistance;

	// Inner lockon angle and range - for short range attacks
	float m_fInnerLockonAngle;
	float m_fInnerLockonDistance;

	// The lockon angle for launches
	float m_fLaunchLockOnAngle;

	// The maximum distance that one can launch at a target
	float m_fLaunchLockOnDistance;

	// The minimum distance that you can lockon for a launch
	float m_fLaunchInnerLockOnDistance;

	// The lockon angle for throwing
	float m_fThrowLockOnAngle;

	// The lock distance for throwing
	float m_fThrowLockOnDistance;

	float m_fPickUpLockOnAngle; // The lockon angle for picking things up
	float m_fPickUpLockOnDistance; // The lock distance for picking things up

	float m_fRunningPickupLockOnAngle; // The lockon angle for doing a running pickup
	float m_fRunningPickupLockOnDistance; // The maximum distance you need to be from a running pickup target

	float m_fCatchLockOnAngle; // The lockon angle for a catch target
	float m_fCatchLockOnDistance; // The maximum distance you need to be from a catch target

	// The lockon distance for evades
	float m_fEvadeLockOnDistance;

	// The lockon angle for evades
	float m_fEvadeLockOnAngle;

	// Stuff for aerial targeting
	float m_fAerialComboDistance;
	float m_fAerialComboAngle;
	float m_fAerialComboHeight;
	float m_fAerialComboHeightBottom;

	// Details specifically for the extra speed attack
	float m_fSpeedExtraDistance;
	float m_fSpeedExtraAngle;

	// Pad direction magnitude to allow lockon change ( 0 - 1 )
	float m_fDirectionLockonMagnitude;

	// For moving directly away from the character
	float m_fMinimumBreakAwayAngle;
	float m_fBreakAwayMagnitude;

	// For choosing the best target within a result set
	float m_fAngleWeighting;
	float m_fDistanceWeighting;

	// Maximum auto lockon speed
	float m_fMaximumLockonSpeed;

	// Height bounds for all the standard targeting
	float m_fUpperHeight;
	float m_fLowerHeight;

	// Height bounds for juggle targeting
	float m_fUpperJuggleHeight;
	float m_fLowerJuggleHeight;

	// Ground attacks
	float m_fGroundAttackInnerDistance;
	float m_fGroundAttackOuterDistance;
	float m_fGroundAttackAngle;
	float m_fGroundAttackHeightTop;
	float m_fGroundAttackHeightBottom;

	// Next nearest - atm just for monty python super... NI!
	float m_fNextNearestInnerDistance;
	float m_fNextNearestOuterDistance;
	float m_fNextNearestAngle;
};


//------------------------------------------------------------------------------------------
//!
//!	AwarenessComponent
//!	For the moment this is an stand alone component that deals with some aspects of how a
//! character can interact with the world around it.  This will probably undergo a lot of
//! changes as we see how the object interaction develops and as the requirements in this
//! area become more clear.
//!
//------------------------------------------------------------------------------------------
class AwarenessComponent : public Awareness_Lua
{
	friend class Awareness_Lua;
public:

	// Construction Destruction
	AwarenessComponent( CEntity* pobParent, const CAttackTargetingData* pobTargetData );
	~AwarenessComponent( void );

	// Get a pointer to any automatically locked on entity
	CEntity* GetCurrentAutoLockonP( void ) const { return m_pobAutoLockonEntity; }

	// Is this entity aware of enemies around it? - returns null if none in range
	const CEntity* IsAwareOfEnemies( void ) const { return m_pobAwarenessEntity; }

	// Find and set a target
	CEntity* GetTarget(	ATTACK_TARGET_TYPE	eTargetType,
						const CEntity*		pobMe,
						bool				bUseFacingDirection = false,
						bool				bUseSpecificDirection = false,
						const CDirection&	obSpecificDirection = CDirection( 0.0f, 0.0f, 1.0f ),
						const CEntity*		pobExemptThisEntity = 0 ) const;

	bool IsEntityInZone(const CEntity* pobZoneOwner, 
						const CEntity* pobZoneAttacker, 
						float fZoneAngle,
						float fZoneSweep,
						float fInnerDistance,
						float fOuterDistance ) const;

	bool IsGoingToLockonTargeting() { return m_fMovementMagnitude > m_pobAttackTargetingData->m_fDirectionLockonMagnitude; }

	// To be expanded so that external systems have more control over the data that they
	// are using for targeting - at the moment this is not the best bit of code - GH
	CEntity* GetDetailedTarget( ATTACK_TARGET_TYPE	eTargetType,
								const CEntity*		pobMe,
								float				fOuterRadius,
								float				fInnerRadius, 
								float				fTargetAngle ) const;

	// Update the lockon
	void Update( float fTimeStep );

	// For visualing how the targeting picker works
	void SetDebugRender( void ) const { m_bDebugRender = true; }
	void ClearDebugRender( void ) const { m_bDebugRender = false; }

	float GetTargetedEvadeEscapeAngle();

	const CAttackTargetingData* GetAttackTargetingData() { return m_pobAttackTargetingData; };

	CEntity* FindRangedWeaponInteractionTarget(RANGED_WEAPON_TYPE eWeaponType, int iCharacterType = 0);


private: 

	// The attack debug stuff can poke around in here as it wishes
	friend class AttackDebugger;

	// Helper to find a new lockon character
	CEntity* FindTarget( ATTACK_TARGET_TYPE eTargetType, const CMatrix& obThisCharacter, const CEntity* pobThis, float fInnerDistance, float fOuterDistance, float fAngle, const CEntity* pobExemptThisEntity ) const;
	
	// Helper function to distinguish some target types
	bool TargetRequiresPadDirection( ATTACK_TARGET_TYPE eTargetType, const CEntity* pobEntity ) const;

	// Helper to find the best lockon from a given set
	CEntity* FindBestResult( const QueryResultsContainerType& obEntities, const CMatrix& obCharacterMatrix, float fInnerDistance, float fOuterDistance, float fAngle ) const;

	// ...and another
	CEntity* GetTarget( const CEntity* pobEntity, const CDirection& obDirection, float fMagnitude, float fInnerDistance, float fOuterDistance, float fAngle );

	// Find all the entities that we are currently 'aware' of
	const CEntity* FindAwareOfEntities( CEntityQuery& obQuery );

	// A pointer to any entity we have automatically locked on to
	CEntity* m_pobAutoLockonEntity;

	// Pointer to our targeting data
	const CAttackTargetingData* m_pobAttackTargetingData;

	// Our parent entity
	CEntity* m_pobParent;

	// We store and update our 'inputs' each frame, catering for player/AI
	float		m_fMovementMagnitude;
	CDirection	m_obMovementDirection;
	CEntity*	m_pobDirectTarget;

	// Are we aware of an enemy
	const CEntity* m_pobAwarenessEntity;

	// Do we want to show how we work?
	mutable bool m_bDebugRender;
};

LV_DECLARE_USERDATA(AwarenessComponent);


#endif // _AWARENESS_H
