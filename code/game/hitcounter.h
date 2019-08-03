//------------------------------------------------------------------------------------------
//!
//!	\file hitcounter.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_HITCOUNTER_H
#define	_HITCOUNTER_H

#include "effect/effect_shims.h"
#include "effect/effect_manager.h"
#include "core/boostarray.h"
#include "lua/ninjalua.h"

// Necessary includes
#include "editable/enumlist.h"

#include "game/eventlog.h"
#include "game/attacks.h"


// Foward declaration
class CEntity;
class HitCounter;
class CAttackComponent;
class Boss;
class StylePoints;

// What base are we assuming for the counter - handles to 16
static const int s_iMaxCounterBase = 16;
static const int s_iCounterBase = 10;

//------------------------------------------------------------------------------------------
//!
//!	HitCounterDef
//!	Define the hit counter behaviour for a particular character
//!
//------------------------------------------------------------------------------------------
class HitCounterDef
{
public:

	// This interface is exposed
	HAS_INTERFACE( HitCounterDef );

	// Construction - just for defaults
	HitCounterDef( void ) :	m_fTimeout( 5.0f ),
							m_iLevelOneSuperStyleThreshold( 3 ),
							m_iLevelTwoSuperStyleThreshold( 6 ),
							m_iLevelThreeSuperStyleThreshold( 9 ),
							m_iLevelFourSuperStyleThreshold( 12 ),
							m_iLevelSpecialSuperStyleThreshold( 50 ),

							m_iLevelOneStyleProgessionThreshold( 30 ),
							m_iLevelTwoStyleProgessionThreshold( 60 ),
							m_iLevelThreeStyleProgessionThreshold( 90 ),
							m_iLevelFourStyleProgessionThreshold( 120 ),
							
							m_iLevelSpecialStyleProgessionThreshold( 270 ),
							m_pobStylePoints( 0 ),
							m_eStartSSLevel ( HL_THREE ),
							m_bIveStartedSoIllFinish( true ),
							m_iBossSpecialThreshold( 25 )
	{
	}

	// How long are we allowed between hits for them to count as consecutive?
	float m_fTimeout;

	// Style thresholds for super execution
	int	m_iLevelOneSuperStyleThreshold, m_iLevelTwoSuperStyleThreshold, m_iLevelThreeSuperStyleThreshold, m_iLevelFourSuperStyleThreshold, m_iLevelSpecialSuperStyleThreshold;
	// Style thresholds for bar progression
	int	m_iLevelOneStyleProgessionThreshold, m_iLevelTwoStyleProgessionThreshold, m_iLevelThreeStyleProgessionThreshold, m_iLevelFourStyleProgessionThreshold, m_iLevelSpecialStyleProgessionThreshold;
	// Style points for combat events
    StylePoints* m_pobStylePoints;

	HIT_LEVEL m_eStartSSLevel;

	// Do we wait till the end of the current strike before reseting the clock?
	bool m_bIveStartedSoIllFinish;

	// At what level of health do we decide we're in HL_SPECIAL to select our superstyle
	int m_iBossSpecialThreshold;

};

//------------------------------------------------------------------------------------------
//!
//!	HitCounterCharacterMulDef
//!	
//!
//------------------------------------------------------------------------------------------

class HitCounterCharacterMulDef
{
	// This interface is exposed
	HAS_INTERFACE( HitCounterCharacterMulDef );

public:

	HitCounterCharacterMulDef(void)
	{
		m_fScalePointsForKO = 1.0f;
		m_fScalePointsForKill = 1.0f;
		m_fScalePointsForGrab = 1.0f;
		m_fScalePointsForRecoil = 1.0f;
		m_fScalePointsForImpactStagger = 1.0f;
		m_fScalePointsForBlockStagger = 1.0f;
		m_fScalePointsForCounterAttack = 1.0f;
		m_fScalePointsForEvadeIncomingAttack = 1.0f;
		m_fScalePointsForAerial = 1.0f;
	}

	float m_fScalePointsForKO;
	float m_fScalePointsForKill;
	float m_fScalePointsForGrab;
	float m_fScalePointsForRecoil;
	float m_fScalePointsForImpactStagger;
	float m_fScalePointsForBlockStagger;
	float m_fScalePointsForCounterAttack;
	float m_fScalePointsForEvadeIncomingAttack;
	float m_fScalePointsForAerial;
};


//------------------------------------------------------------------------------------------
//!
//!	HitCounter
//!	This deals with monitoring the number of successful strikes that a character has made
//!	and hence how 
//!
//------------------------------------------------------------------------------------------
class HitCounter
{
	// History for the hit counter
	struct History
	{
		// The type of hit processed
		enum STRIKETYPE
		{
			ENTITY_STRIKE,
			ENTITY_STRUCK,
		} m_Type;

		// Time when the hit was issued
		double		m_Time;
	};

public:
	// Construction destruction
	HitCounter( HitCounterDef* obDef );
	~HitCounter( void );

	HAS_LUA_INTERFACE()

	// This updates things as you'd expect
	void Update( float fTimeStep );

	// Events that we monitor
	void SuccessfulHit( void );
	void UnSuccessfulHit( void );
	void PlayerStruck( void );
	void StrikeStarted( void );
	void StrikeFinished( void );

	// Get current 'hit level'
	HIT_LEVEL GetCurrentHitLevel( void ) const;
	HIT_LEVEL GetHitLevel( int iStylePoints ) const;
	HIT_LEVEL GetCurrentStyleProgressionLevel( void ) const;
	int GetHits() { return m_iHitCount; };
	int GetStylePoints() { return m_iStylePoints; };
	int AddStylePoints(int iPoints);
	int GetStyleProgression() { return m_iStyleProgression; };
	int GetNextStyleProgressionThreshold();
	int GetNextSuperStyleThreshold();
	int GetSuperStyleThreshold( HIT_LEVEL obHitLevel );
	int GetCurrentSuperStyleThreshold();

	void SetStylePoints( int iPoints ) { m_iStylePoints = iPoints; };
	void SetStyleProgression( int iProgression ) { m_iStyleProgression = iProgression; };

	void ActivateSpecialStyleLevel( void );
	void DeactivateSpecialStyleLevel( void );

	void SetSpecialStyleThreshold ( int iThreshold );
	void ResetSpecialStyleThreshold ( void );


	void SetBossMode( Boss* pobBoss ) { m_pobBoss = pobBoss; };
	Boss* GetBossMode() { return m_pobBoss; };

	// To be called when a player has used the count for reward
	void Used( void );

	// Reset the history
	void ResetHistory(void) 
	{
		m_HistoryIndex = 0;
		m_HistorySize= 0;
	}

	// Public methods to 
	int TestStrikeHistory( double WindowStart, double WindowEnd ) { return TestHistory( WindowStart, WindowEnd, History::ENTITY_STRIKE ); }
	int TestStruckHistory( double WindowStart, double WindowEnd ) { return TestHistory( WindowStart, WindowEnd, History::ENTITY_STRUCK ); }

	CombatEventLog* GetCombatEventLog() { return m_pobCombatEventLog; };
	int GetHitCount() { return m_iHitCount; }

	void RegisterHintEntity ( CEntity* pobEnt );
	void ClearHintEntities ( void );

	bool IsForceActive () { return m_bForceActive; };
	void ForceActive( bool bForce ) { m_bForceActive = bForce; };

private:

	// This can look right inside us
	friend class AttackDebugger;
	friend class SpecialHitCounterRenderer;
	friend class BossHitCounterRenderer;
	friend class CombatHUD;

	// Helpers
	void ResetHitCount( void );
	int TestHistory( double, double, History::STRIKETYPE );

	bool ActivateStyleProgression( HIT_LEVEL StyleLevel);

	// Our definition
	HitCounterDef* m_pobDefinition;

	// How long has it been since our last successful hit?
	float m_fTimeSinceLastHit;

	// How many hits have we strung together
	int m_iHitCount, m_iLastHitCount;
	float m_fTimeSinceLastHitCountDisplayed;
	bool m_bDisplayLastHitCount;

	int m_iStylePoints, m_iStyleProgression; // Keep track of style points
	int m_iOldStylePoints, m_iOldStyleProgression; // Keep track of style points - when switched to special style level
	CombatEventLog* m_pobCombatEventLog; // So we can be notified of style events
	HIT_LEVEL m_eStyleProgressionLevel;
	HIT_LEVEL m_eOldStyleProgressionLevel;

	int m_iCurrentSpecialStyleThreshold;

	// Is the character currently attacking?
	int m_iCharacterAttacking;

	// The number of items to store in the history
	static const int HISTORY_ARRAY_SIZE = 32;

	// Array of History elements.
	Array<History, HISTORY_ARRAY_SIZE>	m_History;
	
	// Index for the next new element in the array
	int	m_HistoryIndex;

	// Number of valid items in the history
	int	m_HistorySize;

	// A flag to help correct display of the manual LC recharge
	bool m_bForceActive;

	// If this is a valid pointer, we should be using only the health of a boss as progression
	Boss* m_pobBoss;

	// Any entities that a special bar wants to do a button hint on
	ntstd::Vector<CEntity*> m_aobEntList;
};

LV_DECLARE_USERDATA(HitCounter);

#endif // _HITCOUNTER_H
