//----------------------------------------------------------------------------------------------------
//!
//!	\file eventlog.h
//!	Event Log functions
//!
//----------------------------------------------------------------------------------------------------

#ifndef _EVENTLOG_H
#define _EVENTLOG_H

//#include "attacktracking.h"
//#include "editable/enumlist.h"
//#include "game/entitymanager.h"
//#include "game/entity.h"

#include "core/timer.h"

class CEntity;

#define EVENTLOG_MAX_EVENTS (32)
#define EVENTLOG_MAX_LIST (128)

// All the types of combat event that can be fired - bit shifted for flag combination
enum CombatEventType {  CE_GOT_KOED					= (1 << 0),
						CE_GOT_KILLED				= (1 << 1), 
						CE_GOT_GRABBED				= (1 << 2), 
						CE_GOT_RECOILED				= (1 << 3), 
						CE_GOT_IMPACT_STAGGERED		= (1 << 4), 
						CE_GOT_BLOCK_STAGGERED		= (1 << 5),
						CE_GOT_DEFLECT				= (1 << 6),
						CE_CAUSED_KO				= (1 << 7), 
						CE_CAUSED_KILL				= (1 << 8), 
						CE_SUCCESSFUL_GRAB			= (1 << 9), 
						CE_CAUSED_RECOIL			= (1 << 10), 
						CE_CAUSED_IMPACT_STAGGER	= (1 << 11), 
						CE_CAUSED_BLOCK_STAGGER		= (1 << 12),
						CE_CAUSED_DEFLECT			= (1 << 13),
						CE_COUNTER_ATTACK			= (1 << 14), 
						CE_BAD_COUNTER_ATTACK		= (1 << 15),
						CE_STARTED_ATTACK			= (1 << 16), 
						CE_STARTED_SPECIAL			= (1 << 17),
						CE_STARTED_EVADE_ATTACK		= (1 << 18),
						CE_STARTED_RISE_ATTACK		= (1 << 19),
						CE_STARTED_AERIAL			= (1 << 20),
						CE_EVADED_INCOMING_ATTACK	= (1 << 21), 
						CE_COMBO					= (1 << 22),
						CE_CHANGE_STANCE			= (1 << 23),
						CE_STARTED_SUPERSTYLE		= (1 << 24),
						CE_FINISHED_SUPERSTYLE		= (1 << 25),
						CE_SELECTED_ATTACK			= (1 << 26),
						CE_MISSED_COUNTER			= (1 << 27),
						CE_MISSED_KILL_COUNTER		= (1 << 28),
						
						CE_COUNT					=		29};

#define CombatEvent	Event<CombatEventType>
#define CombatEventLog EventLog<CombatEventType>
#define CombatEventLogManager EventLogManager<CombatEventType>

// All the types of archer event that can be fired - bit shifted for flag combination
enum ArcherEventType {  AE_VAULT					= (1 << 0),
						AE_CROUCH					= (1 << 1), 
						AE_RELOAD					= (1 << 2), 
						AE_FIRSTPERSON_FIRE			= (1 << 3), 
						AE_THIRDPERSON_FIRE			= (1 << 4), 
						AE_FIRSTPERSON				= (1 << 5), 
						AE_THIRDPERSON				= (1 << 6),
						AE_HEADSHOT					= (1 << 7),
						AE_AFTERTOUCH				= (1 << 8),
						AE_MOUNT_TURRET				= (1 << 9),		// NB - remove this if a general objects event log is implemented
						AE_DISMOUNT_TURRET			= (1 << 10),	// NB - remove this if a general objects event log is implemented

						AE_COUNT					=		11};

#define ArcherEvent	Event<ArcherEventType>
#define ArcherEventLog EventLog<ArcherEventType>
#define ArcherEventLogManager EventLogManager<ArcherEventType>

/***************************************************************************************************
*
*	CLASS			Event
*
*	DESCRIPTION		Event information
*
***************************************************************************************************/
template <typename TEvent>
class Event
{
public:
	Event<TEvent>( TEvent eEventType, double fTimeCreated, void* pData, CEntity* pobEnt, const CEntity* pobTargetEnt ) 
		:	m_eEventType( eEventType )
		,	m_pData( pData )
		,	m_pobEntity(pobEnt)
		,	m_pobTargetEntity(pobTargetEnt)
		,	m_fTimeCreated( fTimeCreated )
	{};
	
	Event() {};
	~Event() {}; // Not responsible for clearing up data

	TEvent			m_eEventType;
	void*			m_pData;
	CEntity*		m_pobEntity;
	const CEntity*	m_pobTargetEntity;
	double			m_fTimeCreated;
};

/***************************************************************************************************
*
*	CLASS			EventLog
*
*	DESCRIPTION		Takes notifications from combat code (when registered with an attack components
*					combat event manager) and stores them until instructed to dispose of them.
*					Other code, primarily HUD, should use this as a reference for whats going on with
*					the players combat. This should replace any direct bindings into attack component
*					fields eventually.
*
***************************************************************************************************/
template <typename TEvent>
class EventLog
{
public:
	EventLog();
	EventLog(int iFlags);

	const Event<TEvent>* GetEvents() { return m_aobEvents; };
	int GetEventCount() { return m_iNumEvents; };
	void AddEvent(TEvent eEvent, void* pData, CEntity* pobEntity, const CEntity* pobTargetEntity);
	void SetFlags(int iFlags);
	void Reset();
	void ClearEvents();

	~EventLog();
private:
	Event<TEvent> m_aobEvents[EVENTLOG_MAX_LIST];

	int m_aTotals[EVENTLOG_MAX_EVENTS];

	int m_iFlags;

	int m_iNumEvents;
};

/***************************************************************************************************
*
*	CLASS			EventLogManager
*
*	DESCRIPTION		Takes notifications from combat code and passes them onto the registered combat
*					event log objects.
*
***************************************************************************************************/
template <typename TEvent>
class EventLogManager
{
public:
	EventLogManager(CEntity* pobParentEntity = 0);
	~EventLogManager();

	void RegisterEventLog(EventLog<TEvent>* pobLog);
	void UnRegisterEventLog(EventLog<TEvent>* pobLog);

	void AddEvent(TEvent eEvent, const CEntity* pEventTarget, void* pData = 0);

	// Set parent required due to warning given by MSVC compiler and using this in base member initialisers
	void SetParent(CEntity* pParent) { ntAssert( m_pobParentEntity == NULL ); m_pobParentEntity = pParent; }

private:
	ntstd::Vector<EventLog<TEvent>*> m_obLogsToUpdate;

	CEntity* m_pobParentEntity;
};

/***************************************************************************************************
*
*	FUNCTION		EventLog::EventLog
*
*	DESCRIPTION		Construct me
*
***************************************************************************************************/
template <typename TEvent> EventLog<TEvent>::EventLog() 
:	m_aobEvents(),
	m_aTotals(),
	m_iFlags( -1 ),
	m_iNumEvents( 0 )
{
	// Init all totals to 0
	for (int i = 0; i < EVENTLOG_MAX_EVENTS; i++)
	{
		this->m_aTotals[i] = 0;
	}

	// Set flags to -1, so we accept every event
	m_iFlags = -1;
}

/***************************************************************************************************
*
*	FUNCTION		EventLog::EventLog
*
*	DESCRIPTION		Construct me with some flags to filter events that are added
*
***************************************************************************************************/
template <typename TEvent>  EventLog<TEvent>::EventLog(int iFlags) 
:	m_aobEvents(),
	m_aTotals(),
	m_iFlags( iFlags ),
	m_iNumEvents( 0 )
{
	// Init all totals to 0
	for (int i = 0; i < EVENTLOG_MAX_EVENTS; i++)
	{
		this->m_aTotals[i] = 0;
	}
}

/***************************************************************************************************
*
*	FUNCTION		EventLog::SetFlags
*
*	DESCRIPTION		Set my flags - use -1 to receive all events
*
***************************************************************************************************/
template <typename TEvent> void EventLog<TEvent>::SetFlags(int iFlags)
{
	m_iFlags = iFlags;
}

/***************************************************************************************************
*
*	FUNCTION		EventLog::~EventLog
*
*	DESCRIPTION		Destruct me
*
***************************************************************************************************/
template <typename TEvent> EventLog<TEvent>::~EventLog()
{
}

/***************************************************************************************************
*
*	FUNCTION		EventLog::AddEvent
*
*	DESCRIPTION		Add an event, if it matches my flags, the associated void data pointer has 
*					meaning depending on the event being added. E.g. CE_STARTED_ATTACK takes a 
*					CAttackData* for the attack.
*
***************************************************************************************************/
template <typename TEvent> void EventLog<TEvent>::AddEvent(TEvent eEvent, void* pData, CEntity* pobEnt, const CEntity* pobTargetEnt)
{
	// Have we run out of space in our array?
	ntError(!(m_iNumEvents > EVENTLOG_MAX_LIST));

	// Which total do we use?
	int iTotalsIndex = 0;
	for (int i = 1; i < EVENTLOG_MAX_EVENTS; i++)
	{
		if (eEvent == (1<<i) )
			iTotalsIndex = i;
	}

	if (m_iFlags == -1)
	{
		this->m_aTotals[iTotalsIndex]++;
		m_aobEvents[m_iNumEvents] = Event<TEvent>(eEvent,CTimer::Get().GetGameTime(),pData,pobEnt,pobTargetEnt);
		m_iNumEvents++;
	}
	else
	{
		if ((m_iFlags & eEvent) == eEvent)
		{
			this->m_aTotals[iTotalsIndex]++;
			m_aobEvents[m_iNumEvents] = Event<TEvent>(eEvent,CTimer::Get().GetGameTime(),pData,pobEnt,pobTargetEnt);
			m_iNumEvents++;
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		EventLog::Reset
*
*	DESCRIPTION		Reset everything.
*
***************************************************************************************************/
template <typename TEvent> void EventLog<TEvent>::Reset()
{
	for (int i = 0; i < EVENTLOG_MAX_EVENTS; i++)
	{
		m_aTotals[i] = 0;
	}

	ClearEvents();
}

/***************************************************************************************************
*
*	FUNCTION		EventLog::Events
*
*	DESCRIPTION		Clear all events.
*
***************************************************************************************************/
template <typename TEvent> void EventLog<TEvent>::ClearEvents()
{
	m_iNumEvents = 0;
}

/***************************************************************************************************
*
*	FUNCTION		EventLogManager::EventLogManager
*
*	DESCRIPTION		Manager class that takes care of all combat event logs on an attack component
*
***************************************************************************************************/
template <typename TEvent> EventLogManager<TEvent>::EventLogManager(CEntity* pobParentEntity) 
: m_obLogsToUpdate()
, m_pobParentEntity(pobParentEntity)
{
}

/***************************************************************************************************
*
*	FUNCTION		EventLogManager::~EventLogManager
*
*	DESCRIPTION		DESTROY.
*
***************************************************************************************************/
template <typename TEvent> EventLogManager<TEvent>::~EventLogManager() 
{
	// Nothing to dispose of, individual combat event logs are responsibility of registrars
}

/***************************************************************************************************
*
*	FUNCTION		EventLogManager::RegisterEventLog
*
*	DESCRIPTION		Adds pobLog to our list so we can update it when we receive events, unregister
*					pobLog from this manager before deleting it.
*
***************************************************************************************************/
template <typename TEvent> void EventLogManager<TEvent>::RegisterEventLog(EventLog<TEvent>* pobLog) 
{ 
	m_obLogsToUpdate.push_back(pobLog); 
}
	
/***************************************************************************************************
*
*	FUNCTION		EventLogManager::UnRegisterEventLog
*
*	DESCRIPTION		Gets rid of this from our list so it's no longer updated, and can be deleted safely.
*
***************************************************************************************************/
template <typename TEvent> void EventLogManager<TEvent>::UnRegisterEventLog(EventLog<TEvent>* pobLog) 
{ 
	for (unsigned int i = 0; i < m_obLogsToUpdate.size(); i++)
	{
		if (m_obLogsToUpdate[i] == pobLog)
		{
			m_obLogsToUpdate.erase(m_obLogsToUpdate.begin() + i);
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		EventLogManager::AddEvent
*
*	DESCRIPTION		CAttackComponent uses this to notify upon events
*
***************************************************************************************************/
template <typename TEvent> void EventLogManager<TEvent>::AddEvent(TEvent eEvent, const CEntity* pEventTarget, void* pData) 
{ 
	for (unsigned int i = 0; i < m_obLogsToUpdate.size(); i++) 
		m_obLogsToUpdate[i]->AddEvent(eEvent,pData,m_pobParentEntity, pEventTarget); 
}

#endif //_EVENTLOG_H
