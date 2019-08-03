//------------------------------------------------------------------------------------------
//!
//!	\file combatstyle.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_COMBATSTYLE_H
#define	_COMBATSTYLE_H

// Necessary includes
#include "game/hitcounter.h"
#include "game/archerhitcounter.h"

class StyleManager;
class StyleLabelRenderer;
class Object_Checkpoint;

/***************************************************************************************************
*
*	FUNCTION		LifeClockDef
*
*	DESCRIPTION		Defines properties for a lifeclock
*
***************************************************************************************************/
class LifeClockDef
{
	HAS_INTERFACE( LifeClockDef );
public:
	LifeClockDef() : m_iDays( 2 ),
					m_iHours( 2 ),
					m_iMinutes( 2 ),
					m_fSeconds( 2.0f ),
					m_bKillPlayer( true ),
					m_fTimeToTakeUpdating( 2.0f )
	{
		if (m_iDays > 99) m_iDays = 99;
		if (m_iHours > 23) m_iHours = 23;
		if (m_iMinutes > 59) m_iMinutes = 59;
		if (m_fSeconds > 59) m_fSeconds = 59;
	};

	~LifeClockDef() {};
	
	int m_iDays, m_iHours, m_iMinutes;
	float m_fSeconds;
	bool m_bKillPlayer;

	int m_iLowDays, m_iLowHours, m_iLowMinutes;
	float m_fLowSeconds;
	
	float m_fTimeToTakeUpdating;
};

/***************************************************************************************************
*
*	CLASS			LifeClock
*
*	DESCRIPTION		An ellaborate clock for the hero to use to count down to death.
*
***************************************************************************************************/
class LifeClock
{
public:
	LifeClock(LifeClockDef* pobLCD);
	~LifeClock();
    
	void IncrementLifeClock(float fSeconds);
	void DecrementLifeClock(float fSeconds);
	void Update(float fTimeDelta);
	double GetTotalInSeconds();
	double GetInitialTime() { return m_dInitialTime; };
	void SetTotalInSeconds( double dSeconds );

	void SetScalar ( float fLifeClockScalar ) { m_fLifeClockScalar = fLifeClockScalar; };
	float GetScalar ( void ) { return m_fLifeClockScalar; };

	bool IsActive () { return m_bActive; };
	void SetActive (bool bActive) { m_bActive = bActive; };

private:
	bool m_bOwnsDef;
	LifeClockDef* m_pobDef;

	bool m_bKilledPlayer;

	double m_dTime;
	double m_dInitialTime;
	double m_dLowTime;
	bool m_bLowWarning;
	float m_fLifeClockScalar;

	bool m_bActive;
};

class StylePoints
{
public:
	// This interface is exposed
	HAS_INTERFACE( StylePoints );

	StylePoints(): 	m_iCausedKO( 5 ),
					m_iCausedKill( 10 ),
					m_iSuccessfulGrab( 3 ),
					m_iCausedRecoil( 1 ),
					m_iCausedImpactStagger( 2 ),
					m_iCausedBlockStagger( 2 ),
					m_iCounterAttack( 5 ),
					m_iEvadedIncomingAttack( 5 ),
					m_iStartedAerial( 5 ),
					m_fComboMultiple( 1.25f )
	{};
	~StylePoints() {};
	
	int m_iCausedKO, m_iCausedKill, m_iSuccessfulGrab, m_iCausedRecoil, m_iCausedImpactStagger, m_iCausedBlockStagger, m_iCounterAttack, m_iEvadedIncomingAttack, m_iStartedAerial;
	float m_fComboMultiple;
};

//------------------------------------------------------------------------------------------
//!
//!	StyleEventDef
//!
//------------------------------------------------------------------------------------------
class StyleEventDef
{
public:
	HAS_INTERFACE(StyleEventDef)

	// Construction destruction
	StyleEventDef( void );
	~StyleEventDef( void );

protected:
	int m_iDefaultStyleValue;
	ntstd::String m_obEventString;
	ntstd::String m_obEventImage;
	int m_iPriority;

	friend class StyleManager;
	friend class StyleLabelRenderer;
};

//------------------------------------------------------------------------------------------
//!
//!	StyleEvent
//!
//------------------------------------------------------------------------------------------
class StyleEvent
{
public:
	// Construction destruction
	StyleEvent( StyleEventDef* pobDef, int iStyle, STYLE_TYPE eStyleType )
		:	m_pobDef ( pobDef )
		,	m_iStyleValue ( iStyle )
		,	m_eStyleType ( eStyleType )
	{;};

	StyleEvent( )
		:	m_pobDef ( 0 )
		,	m_iStyleValue ( 0 )
		,	m_eStyleType ( STYLE_TYPE_MISC )
	{;};

	StyleEvent( StyleEvent& obStyleEvent )
		:	m_pobDef ( obStyleEvent.m_pobDef )
		,	m_iStyleValue ( obStyleEvent.m_iStyleValue )
	{;};

	~StyleEvent( void );
			
	int GetStyleValue() { return m_iStyleValue; };
	STYLE_TYPE  GetStyleType() { return m_eStyleType; }

protected:
	StyleEventDef* m_pobDef;
	int m_iStyleValue;
	STYLE_TYPE m_eStyleType;

	friend class StyleManager;
	friend class StyleLabelRenderer;
};

//------------------------------------------------------------------------------------------
//!
//!	StyleManagerDef
//!
//------------------------------------------------------------------------------------------
class StyleManagerDef
{
public:
	HAS_INTERFACE(StyleManagerDef)

	// Construction destruction
	StyleManagerDef( void );
	~StyleManagerDef( void );

	void PostConstruct( void );

private:
	LifeClockDef* m_pobLifeClockDef;
	HitCounterDef* m_pobHitCounterDef;
	ArcherHitCounterDef* m_pobArcherHitCounterDef;

	float m_fEventCooldownTime;

	friend class StyleManager;
};

//------------------------------
//!
//! StanceStylePoints
//!	Class for the four style point values
//!
//--------------------------------
class StanceStylePoints
{
public:
	StanceStylePoints()
	:	m_iStylePointsSpeed ( 0 )
	,	m_iStylePointsPower ( 0 )
	,	m_iStylePointsRange ( 0 )
	,	m_iStylePointsAerial ( 0 )
	,	m_iStylePointsMisc ( 0 )
	,	m_iStylePointsOverall ( 0 )
	{};

	void Reset()
	{
		m_iStylePointsSpeed = 0;
		m_iStylePointsPower = 0;
		m_iStylePointsRange = 0;
		m_iStylePointsAerial = 0;
		m_iStylePointsMisc = 0;
		m_iStylePointsOverall = 0;
	};

	// Operators
	StanceStylePoints  operator - ( const StanceStylePoints& obOther );
	StanceStylePoints  operator + ( const StanceStylePoints& obOther );
	StanceStylePoints& operator -= ( const StanceStylePoints& obOther );
	StanceStylePoints& operator += ( const StanceStylePoints& obOther );

	void Negate( void );
	void SetNegativeValuesToZero();

	void CompareAndStoreHighest( const StanceStylePoints& obPoints1, const StanceStylePoints& obPoints2 );

	int	m_iStylePointsSpeed;
	int	m_iStylePointsPower;
	int	m_iStylePointsRange;
	int	m_iStylePointsAerial;
	int	m_iStylePointsMisc;

	int m_iStylePointsOverall;
};

//------------------------------
//!
//! StyleStats
//!	Game or style stats to display for checkpoints and/or have as a players's total actions.
//!
//--------------------------------
class StyleStats
{
public:
	StyleStats()
	:	m_iTotalKills ( 0 )
	,	m_iKills ( 0 )
	,	m_iSuccessfulBlocks( 0 )
	,	m_iUnsuccessfulBlocks( 0 )
	,	m_iCausedKOs( 0 )
	,	m_iGotKOs( 0 )
	,	m_bSuperStyleActive(false)
	,	m_fTimePlayed ( 0.0f )
	{};

	void DoSectionTotals( void );

public:
	int m_iTotalKills;
	int m_iKills;

	// (chipb) These have been added for the interactive music system, and may not be in the best place
	int m_iSuccessfulBlocks;
	int m_iUnsuccessfulBlocks;
	int m_iCausedKOs;
	int m_iGotKOs;
	bool m_bSuperStyleActive;

	double m_dTotalTimePlayed;
	float m_fTimePlayed;

	// Style points for each stance
	StanceStylePoints m_obStylePointsForSection;			// Style Points earned in currently played section
	StanceStylePoints m_obStylePointsDeficitForSection;		// Style Points deficit in currently played section for combo unlocking calculations

	// Cached total of the stance style points for quicker calculations during gameplay
	StanceStylePoints m_obCachedStylePointTotals;
};

//------------------------------------------------------------------------------------------
//!
//!	StyleManager
//!
//------------------------------------------------------------------------------------------
class StyleManager : public Singleton<StyleManager>
{
public:
	// Construction / destruction
	StyleManager( StyleManagerDef* pDef );
	~StyleManager();

	void Update( float fTimeStep );

	void RegisterStyleEvent(CHashedString obEvent, STYLE_TYPE eStyleType = STYLE_TYPE_MISC );
	void RegisterStyleEvent(CHashedString obEvent, int iStyle, STYLE_TYPE eStyleType = STYLE_TYPE_MISC );

	LifeClock* GetLifeClock() { return m_pobLifeClock; };
	HitCounter* GetHitCounter() { return m_pobHitCounter; };
	ArcherHitCounter* GetArcherHitCounter() { return m_pobArcherHitCounter; };

	StyleStats& GetStats() { return m_obStyleStats; };

	bool GetRenderableEvent(StyleEvent& obRenderableEvent);
	bool HasRenderableEvent( void );

	bool IsActive( void ) { return m_bActive; };
	void SetActive( bool bActive ) { m_bActive = bActive; };

	bool IsPrologMode( void ) { return m_bPrologMode; };
	void SetPrologMode( bool bProlog ) { m_bPrologMode = bProlog; };

	void GetCurrentStylePoints( StanceStylePoints& rNewPoints );
private:

	StyleEventDef* GetEventDef(CHashedString obEvent);

	StyleManagerDef* m_pobDefinition;

	LifeClock* m_pobLifeClock;
	HitCounter* m_pobHitCounter;
	ArcherHitCounter* m_pobArcherHitCounter;

	StyleStats m_obStyleStats;

	float m_fCurrEventTime;

	ntstd::List<StyleEvent*> m_aobStyleEventQue;
	ntstd::List<StyleEvent*> m_aobStyleEventRenderList;

	bool m_bActive;

	bool m_bPrologMode;				// Flag to modify the style mechanics for the opening section of the game
};

typedef ntstd::List<StyleEvent*>::iterator StyleEventIter;

#endif // _COMBATSTYLE_H
