/***************************************************************************************************
*
*	DESCRIPTION		Processes controller input and converts it into direct attack requests
*
*	NOTES
*
***************************************************************************************************/

#ifndef _ATTACKTRACKING_H
#define _ATTACKTRACKING_H

#include "editable/enumlist.h"
//#include "hitcounter.h"

// Forward declarations
class CAttackLink;
class CAttackData;
class CClusterStructure;
class CEntity;
class CAttackTargeting;
class CInputComponent;
class HitCounter;
struct GameGUID;
class SpecificCounterIndex;

/***************************************************************************************************
*
*	CLASS			CAttackTracker
*
*	DESCRIPTION		Knows how to move through the attack data structure - including links, headers
*					and data objects.
*
*					//GILES i don't want the attack component to be passed in to any of the methods
*					on this class - it means the responsiblities are screwy.
*
***************************************************************************************************/
class CAttackTracker
{
private:
	
	// A structure to hold all we need to know about attacks
	class CAttackTrackingData
	{
	public:

		// Construction - make sure all is clear
		CAttackTrackingData();

		// Assignment
		CAttackTrackingData& operator=( const CAttackTrackingData& obOther );

		// Clear out the existing data
		void Reset();

		// Make sure an item contains the necessary data
		bool IsComplete( void ) const;

		// The members
		const CAttackLink*		pobAttackLink;
		const CAttackData*		pobAttackData;
		ATTACK_MOVE_TYPE		eRequestType;
	};

public:

	// Construction destruction
	CAttackTracker();
	~CAttackTracker();

	// Set the attack data structure that this tracker should track
	void InitialiseAttackStructure( const CClusterStructure* pobClusterStructure );

	// This component can be overridden with direct attacks
	void SetToDirectMode( void ) { m_bDirectMode = true; Reset(); }

	// Move from the current to the requested attack
	void MoveToRequested( void );

	// We may want to clear out a request that we don't want
	bool ClearRequested( void );

	// Access to the main data that this component looks after
	const CAttackData*		GetCurrentAttackDataP()		const { return m_obCurrentAttack.pobAttackData; }
	const CAttackLink*		GetCurrentAttackLinkP()		const { return m_obCurrentAttack.pobAttackLink; }
	ATTACK_MOVE_TYPE		GetCurrentAttackType()		const { return m_obCurrentAttack.eRequestType; }

	// Access to the waiting attack data
	const CAttackData*		GetRequestedAttackDataP()	const { return m_obRequestedAttack.pobAttackData; }
	const CAttackLink*		GetRequestedAttackLinkP()		const { return m_obRequestedAttack.pobAttackLink; }
	ATTACK_MOVE_TYPE		GetRequestedAttackType()		const { return m_obRequestedAttack.eRequestType; }

	// Button held stuff
	bool SwitchToRequestedButtonHeldAttack( void );
	void SetIsWaitingForHeld(bool bWait) { m_bWaiting = bWait; };
	bool GetIsWaitingForHeld() { return m_bWaiting; };



	// Attack selection - the start points for attacks
	bool SelectStartAttack( CEntity* pobEntity, STANCE_TYPE eStance, HIT_LEVEL eHitLevel, bool bOnTheSpotEquivalent,
		ATTACK_EVADE_TYPE ePossibleEvadeType = AET_STANDARD, ATTACK_EVADE_SECTOR ePossibleEvadeSector = AES_FRONT);
	
	bool SelectSpecificCounterAttack(CEntity* pobEntity, STANCE_TYPE eStance, SpecificCounterIndex* pobSpecificCounterCollection, bool bAutoCounter = false );

	bool SelectInstantKORecoverAttack( CEntity* pobEntity, STANCE_TYPE eStance, HIT_LEVEL eHitLevel );

	// For selecting the next attack in the chain
	bool SelectNextAttack( CEntity* pobEntity, STANCE_TYPE eStance, bool bAutomatic, HIT_LEVEL eHitLevel, bool bOnTheSpotEquivalent );

	// If we change target we must return to the start of the attack string
	bool SelectInitialAttackEquivalent( void );

	// For move to tweaking
	bool SelectMediumRangeEquivalent( void );
	bool SelectLongRangeEquivalent( void );

	// For reseting back to where we were if we try out alternative attacks
	bool SelectNextAttackEquivalent( void );

	// For intercept tweaking
	bool SelectInterceptEquivalent( HIT_LEVEL eHitLevel );

	// For attacks from on the floor
	bool SelectRisingAttack( CEntity* pobEntity, STANCE_TYPE eStance, bool& bRequestMade, HIT_LEVEL eHitLevel );

	// For when our grab is blocked
	bool SelectBlockedGrab( void );

	// For hitting people on the floor, if we've found a target
	bool SelectGroundAttack( CEntity* pobEntity, STANCE_TYPE eStance, HIT_LEVEL eHitLevel, REACTION_ZONE eZone );

	// For when we're in zones of specific attack vulnerability and need to swap out old attack with a new one
	bool SelectSpecificAttackFromVulnerabilityZone(const CAttackLink* pobNewLink);

	// This is for Welder/anim event editor - and it's a horrible hack
	bool ForceRequestedAttackToBe(const CAttackLink* pobNewLink);

	bool AISelectStartAttack( ATTACK_MOVE_TYPE eMoveType );
	bool AISelectShortLockAttack( ATTACK_MOVE_TYPE eMoveType );
	bool AISelectLockAttack( ATTACK_MOVE_TYPE eMoveType, float fDist );
	bool AISelectNextAttack( ATTACK_MOVE_TYPE eMoveType );
	bool AISelectAttack( const CAttackLink* );
	bool AISelectNextAttack( const CAttackLink* );

	// AI Query attack link
	const CAttackLink* AIQueryAttackLink( ATTACK_MOVE_TYPE eMoveType, float fLockOnRange, const GameGUID* pPrevGUID ) const;

	// Return to the beginning of the attack structure
	void Reset();

	// For checking for pauses, camera shots etc
	bool  CurrentAttackStillRequested( CInputComponent& obInput, float fAttackTime ) const;
	bool  ButtonHeldAttackStillRequested( CInputComponent& obInput ) const;
	float GetAttackRequestTime(CInputComponent& input) const;

	// We need to distinguish between the requested attack types in some circumstances
	ATTACK_CLASS GetRequestedAttackClass( void );

	// Are we ready to move on th a new attack?
	bool RequestIsReady( void ) const { return m_obRequestedAttack.IsComplete() && !m_bWaiting; }

	// Can another request for an attack be made - depends on whether we are set 
	// to recieve multiple attack requests
	bool WillTakeAttackRequests( void ) const;

	// Get the attack type requested based on the current pad state
	ATTACK_MOVE_TYPE GetRequestedAttackType( const CInputComponent* pobInput, STANCE_TYPE eStance ) const;

	void SelectLedgeRecoverAttack();
	bool CanLedgeRecover();
protected:

	ntstd::List<CAttackLink*> m_obDisallowedAttacks;

	// A helper to automatically choose an attack from a given link
	ATTACK_MOVE_TYPE AutoSelect( const CAttackLink* pobFromLink, STANCE_TYPE eStanceType );

	// A helper to get the next attack
	bool SelectNewAttack( const CAttackLink* pobFromLink, ATTACK_MOVE_TYPE eMoveType, HIT_LEVEL eHitLevel, bool bOnTheSpotEquivalent );

	// Helper for selecting superstyle moves
	const CAttackLink* GetStyleLevelLink( ATTACK_MOVE_TYPE eAttackType, HIT_LEVEL eHitLevel, bool bOnTheSpotEquivalent );
	
	// Decide what type of evade we want to do
	const CAttackLink* GetEvade( ATTACK_MOVE_TYPE eAttackType, ATTACK_EVADE_TYPE eEvadeType = AET_STANDARD, ATTACK_EVADE_SECTOR eEvadeSector = AES_FRONT);

	// A reference to the attack structure we are to track
	const CClusterStructure* m_pobClusterStructure;

	// The current attack data
	CAttackTrackingData m_obCurrentAttack;

	// The requested attack data
	CAttackTrackingData m_obRequestedAttack;
	CAttackTrackingData m_obRequestedButtonHeldAttack;

	// Cabin crew doors to manual
	bool m_bDirectMode;

	bool m_bWaiting;

	int m_iLastRandomStyleIndex;
};

#endif // _ATTACKTRACKING_H
