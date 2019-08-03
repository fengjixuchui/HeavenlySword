#ifndef _TUTORIAL_H
#define _TUTORIAL_H

#include "game/attacks.h"
#include "game/eventlog.h"
#include "editable/enumlist.h"

// For screen message
#include "gui/guiunit.h"

// Base class for all watch objects
class CTutorialBaseWatch 
{
public:
	//! Construction Destruction
	CTutorialBaseWatch( const CEntity* pobEnt, const CHashedString obMsg );
	virtual ~CTutorialBaseWatch( void );

protected:
	const CHashedString	m_obReplyMsg;
	const CEntity*	m_pobReplyEnt;
};

// Base class for all combat watch objects
class CCombatTutorialBaseWatch : protected CTutorialBaseWatch
{
public:
	//! Construction Destruction
	CCombatTutorialBaseWatch( const CEntity* pobEnt, const CHashedString obMsg );
	//virtual ~CCombatTutorialBaseWatch();

	virtual bool	Complete( CombatEventLog* pobCombatEventLog );

protected:
};

// Simple strike - any hit made including those blocked
class CTutorialSimpleStrikeWatch : protected CCombatTutorialBaseWatch
{
public:
	CTutorialSimpleStrikeWatch( const CEntity* pobEnt, const CHashedString obMsg, int iAttackClass, bool bSuccessful = true );
	//virtual ~CTutorialSimpleStrikeWatch( void );

	virtual bool	Complete( CombatEventLog* pobCombatEventLog );

private:
	int m_iAttackClass;
	bool m_bSuccessful;
};

// Combat event watch, ie KO, Kills and Grabs
class CTutorialSimpleEventWatch : protected CCombatTutorialBaseWatch
{
public:
	CTutorialSimpleEventWatch( const CEntity* pobEnt, const CHashedString obMsg, int iEvent );
	//virtual ~CTutorialSimpleEventWatch( void );

	virtual bool	Complete( CombatEventLog* pobCombatEventLog );

private:
	int m_iEvent;
};

// Sucessful blocks ( deflections - i.e. blocking axeman is power stance) 
class CTutorialDeflectWatch : protected CCombatTutorialBaseWatch
{
public:
	CTutorialDeflectWatch( const CEntity* pobEnt, const CHashedString obMsg, int iStance );
	//virtual ~CTutorialSimpleStrikeWatch( void );

	virtual bool	Complete( CombatEventLog* pobCombatEventLog );

private:
	int m_iStance;
};

// Evade attacks
class CTutorialEvadeAttackWatch : protected CCombatTutorialBaseWatch
{
public:
	CTutorialEvadeAttackWatch( const CEntity* pobEnt, const CHashedString obMsg/*, int iAttackClass*/, bool bSuccessful = true );
	//virtual ~CTutorialSimpleStrikeWatch( void );

	virtual bool	Complete( CombatEventLog* pobCombatEventLog );

private:
	enum { EAW_EVADE,
		EAW_ATTACK, } m_eState;
	/*int m_iAttackClass;*/
	bool m_bSuccessful;
};

// Specific attack targets
class CTutorialAttackTargetWatch : protected CCombatTutorialBaseWatch
{
public:
	CTutorialAttackTargetWatch( const CEntity* pobEnt, const CHashedString obMsg, int iAttackTarget, bool bSuccessful = true );
	//virtual ~CTutorialAttackTargetWatch( void );

	virtual bool	Complete( CombatEventLog* pobCombatEventLog );

private:
	int m_iAttackTarget;
	bool m_bSuccessful;
};

// Superstyle watch
class CTutorialSuperstyleWatch : protected CCombatTutorialBaseWatch
{
public:
	CTutorialSuperstyleWatch( const CEntity* pobEnt, const CHashedString obMsg, int iLevel );
	//virtual ~CTutorialSuperstyleWatch( void );

	virtual bool	Complete( CombatEventLog* pobCombatEventLog );

private:
	int m_iLevel;
};

// Stance change watch
class CTutorialStanceWatch : protected CCombatTutorialBaseWatch
{
public:
	CTutorialStanceWatch( const CEntity* pobEnt, const CHashedString obMsg, int iStance );
	//virtual ~CTutorialStanceWatch( void );

	virtual bool	Complete( CombatEventLog* pobCombatEventLog );

private:
	int m_iStance;
};

// Specific strike - a particular attack started
class CTutorialSpecificStrikeWatch : protected CCombatTutorialBaseWatch
{
public:
	CTutorialSpecificStrikeWatch( const CEntity* pobEnt, const CHashedString obMsg, const CHashedString* pobSpecificStrike, bool bSuccessful = true );
	//virtual ~CTutorialSpecificStrikeWatch( void );

	virtual bool	Complete( CombatEventLog* pobCombatEventLog );

private:
	CAttackData* m_pobSpecificStrike;
	bool	m_bSuccessful;
};

// Base class for all archer watch objects
class CArcherTutorialBaseWatch : protected CTutorialBaseWatch
{
public:
	//! Construction Destruction
	CArcherTutorialBaseWatch( const CEntity* pobEnt, const CHashedString obMsg );
	//virtual ~CArcherTutorialBaseWatch();

	virtual bool	Complete( ArcherEventLog* pobArcherEventLog );

protected:
};


// Combat event watch, ie KO, Kills and Grabs
class CArcherSimpleEventWatch : protected CArcherTutorialBaseWatch
{
public:
	CArcherSimpleEventWatch( const CEntity* pobEnt, const CHashedString obMsg, int iEvent );
	//virtual ~CArcherSimpleEventWatch( void );

	virtual bool	Complete( ArcherEventLog* pobArcherEventLog );

private:
	int m_iEvent;
};

// Base class for all entity watch objects
class CEntityTutorialBaseWatch : protected CTutorialBaseWatch
{
public:
	//! Construction Destruction
	CEntityTutorialBaseWatch( const CEntity* pobEntity, const CEntity* pobReplyEnt, const CHashedString obMsg );
	//virtual ~CEntityTutorialBaseWatch();

	virtual bool	Complete( );

protected:
	const CEntity* m_pobEntity;
};

// Base class for all entity watch objects
class CEntityAttackWindowWatch : protected CEntityTutorialBaseWatch
{
public:
	enum ATK_WINDOW
	{
		AW_INVUNERABLE,
		AW_STRIKE,
		AW_STRIKE2,
		AW_ATTACK_POP_OUT,
		AW_BLOCK_POP_OUT,
		AW_NO_LOCK,
		AW_MOVEMENT_POP_OUT,
		AW_UNINTERRUPTABLE,
		AW_NO_COLLIDE,
		AW_NEXTMOVE,
		AW_RANGE_INTERCEPT,
		AW_INTERCEPT,
		AW_EVADE_AND_GRAB_DENY,
		AW_VICTIM_RAGDOLLABLE
	};

	enum ATK_WINDOW_STATE
	{
		AWS_INIT,
		AWS_IN,
		AWS_OUT
	};

	//! Construction Destruction
	CEntityAttackWindowWatch( const CEntity* pobEntity, const CEntity* pobReplyEnt, const CHashedString obEnterMsg, const CHashedString obExitMsg, ATK_WINDOW eAtkWindow );
	//virtual ~CEntityAttackWindowWatch();

	virtual bool	Complete( );

protected:
	ATK_WINDOW			m_eAtkWindow;
	ATK_WINDOW_STATE	m_eAtkWindowState;

	CHashedString		m_obEnterMsg;
};

class CTutorialManager : public Singleton<CTutorialManager>
{
public:
	//! Construction Destruction
	CTutorialManager( void );
	~CTutorialManager( void );

	//! Go
	void	Update( void );

	void	AddWatch( const CHashedString obEvent, const CEntity* pobReplyEnt, const CHashedString obMsg, void* pParam = 0, CEntity* pobWatchEnt = 0);
	void	AddAttackWindowWatch( CEntity* pobWatchEnt, const CHashedString obWindow, const CEntity* pobReplyEnt, const CHashedString obEnterMsg, const CHashedString obExitMsg );


	void	ClearWatches( void );

	bool	AddScreenMessage(const char* pcTextID, const char* pcFont, float fDuration);

	void	ScreenMessagePosition(float fX, float fY);

	void	RemoveScreenMessage(void);

	typedef ntstd::List< CCombatTutorialBaseWatch* > CombatWatchList;
	
	typedef ntstd::List< CArcherTutorialBaseWatch* > ArcherWatchList;

	typedef ntstd::List< CEntityTutorialBaseWatch* > EntityWatchList;

	void	AddTimeScalar ( float fTimeScalar, float fBlendTime );		
	void	ClearTimeScalar ( float fBlendTime );
	void	TimeScalarCallback ( CEntity* pobReplyEnt, CHashedString obMsg );

	
	// Struct to manage Combat logs/watches on many characters
	struct CombatTutorialContainer
	{
	private:
		CombatTutorialContainer::CombatTutorialContainer()
		:	m_pobCombatEventLog ( 0 )
		,	m_pobEnt ( 0 )
		{
			m_obCombatWatchList.clear();
		};

	public:
		CombatTutorialContainer::CombatTutorialContainer( CEntity* pobEnt )
		{
			m_obCombatWatchList.clear();

			m_pobCombatEventLog = NT_NEW CombatEventLog;

			if ( pobEnt && pobEnt->GetAttackComponent() )
			{
				m_pobEnt = pobEnt;

				// Which events are we interested in?
				m_pobCombatEventLog->SetFlags( -1 );

				m_pobEnt->GetAttackComponent()->RegisterCombatEventLog( m_pobCombatEventLog );
			}
		};

		CombatTutorialContainer::~CombatTutorialContainer(  )
		{
			if ( m_pobEnt )
			{
				m_pobEnt->GetAttackComponent()->UnRegisterCombatEventLog( m_pobCombatEventLog );
				m_pobEnt = 0;
			}

			for( CombatWatchList::iterator obIt = m_obCombatWatchList.begin(); obIt != m_obCombatWatchList.end(); ++obIt)
			{
				NT_DELETE ( (*obIt) );
			}

			m_obCombatWatchList.clear();
			
			if ( m_pobCombatEventLog )
			{
				NT_DELETE (m_pobCombatEventLog);
				m_pobCombatEventLog = 0;
			}
		};

		CombatWatchList		m_obCombatWatchList;
		CombatEventLog*		m_pobCombatEventLog;

		CEntity*			m_pobEnt;
	};

protected:

	EntityWatchList m_obEntityWatchList;
	//CombatWatchList m_obCombatWatchList;
	ArcherWatchList m_obArcherWatchList;

	//CombatEventLog m_obCombatEventLog;
	ArcherEventLog m_obArcherEventLog;

	
	
	bool m_bRegistered;

	CPoint m_obTextPosition;

	CGuiUnit* m_pobScreenMsg;

	float m_fLastTimeScalar;
	float m_fAimTimeScalar;
	float m_fCurrTimeScalar;

	float m_fBlendTime;
	float m_fCurrBlendTime;

	ntstd::List< ntstd::pair < float, float > > m_aobTimeScalarQue;

	typedef ntstd::List< ntstd::pair < float, float > >::iterator TTSIt;

	CEntity* m_pobScalarCallbackEnt;
	CHashedString m_obScalarCallbackMsg;

	ntstd::Map< CEntity*, CombatTutorialContainer* > m_aobCombatTutorialList;
	typedef ntstd::Map< CEntity*, CombatTutorialContainer* >::iterator CTLIt;
};

#endif // _TUTORIAL_H
