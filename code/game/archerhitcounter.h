//------------------------------------------------------------------------------------------
//!
//!	\file archerhitcounter.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_ARCHERHITCOUNTER_H
#define	_ARCHERHITCOUNTER_H

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
class ArcherHitCounter;
class CAttackComponent;
class StylePoints;

//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounterDef
//!	Define the hit counter behaviour for Archer
//!
//------------------------------------------------------------------------------------------
class ArcherHitCounterDef
{
public:

	// This interface is exposed
	HAS_INTERFACE( ArcherHitCounterDef );

	// Construction - just for defaults
	ArcherHitCounterDef( void )
	:	m_pobStylePoints ( 0 )
	,	m_fTimeout( 5.0f )
	,	m_bIveStartedSoIllFinish( true )
	{
	}

	// Style points for combat events
    StylePoints* m_pobStylePoints;

	// How long are we allowed between hits for them to count as consecutive?
	float m_fTimeout;

	// Do we wait till the end of the current strike before reseting the clock?
	bool m_bIveStartedSoIllFinish;
};

//------------------------------------------------------------------------------------------
//!
//!	ArcherHitCounter
//!	This deals with monitoring the number of successful strikes that a character has made
//!	and hence how 
//!
//------------------------------------------------------------------------------------------
class ArcherHitCounter
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
	ArcherHitCounter( ArcherHitCounterDef* obDef );
	~ArcherHitCounter( void );

	HAS_LUA_INTERFACE()

	// This updates things as you'd expect
	void Update( float fTimeStep );

	// Events that we monitor
	void SuccessfulHit( void );
	void UnSuccessfulHit( void );
	void PlayerStruck( void );
	void StrikeStarted( void );
	void StrikeFinished( void );

	int GetHits() { return m_iHitCount; };

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

private:

	// This can look right inside us
	friend class AttackDebugger;

	// Helpers
	void ResetHitCount( void );
	int TestHistory( double, double, History::STRIKETYPE );

	// Our definition
	ArcherHitCounterDef* m_pobDefinition;

	// How long has it been since our last successful hit?
	float m_fTimeSinceLastHit;

	// How many hits have we strung together
	int m_iHitCount, m_iLastHitCount;
	float m_fTimeSinceLastHitCountDisplayed;
	bool m_bDisplayLastHitCount;

	CombatEventLog* m_pobCombatEventLog; // So we can be notified of style events

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
};

LV_DECLARE_USERDATA(ArcherHitCounter);

#endif // _ARCHERHITCOUNTER_H
