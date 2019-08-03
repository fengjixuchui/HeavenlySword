/***************************************************************************************************
*
*	Tutorial.cpp
*
*	15/5/06		Tom McKeown created
*
*	Maintains watches on player actions and informs script entity of their succesful completion.
*
***************************************************************************************************/

// Includes
#include "tutorial.h"
#include "hud/hudmanager.h"
#include "gui/guiutil.h"
#include "game/entityarcher.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "game/messagehandler.h"
#include "game/hitcounter.h"
#include "objectdatabase/dataobject.h"


//////////////////////////////////////////////
//
//	CTutorialBaseWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CTutorialBaseWatch::CTutorialBaseWatch( const CEntity* pobEnt, const CHashedString obMsg  )
:	m_obReplyMsg ( obMsg )
,	m_pobReplyEnt ( pobEnt )
{
}

CTutorialBaseWatch::~CTutorialBaseWatch( void )
{
}

//////////////////////////////////////////////
//
//	CCombatTutorialBaseWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CCombatTutorialBaseWatch::CCombatTutorialBaseWatch( const CEntity* pobEnt, const CHashedString obMsg  )
:	CTutorialBaseWatch( pobEnt, obMsg  )
{}

//////////////////////////////////////////////
//
//	CCombatTutorialBaseWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CCombatTutorialBaseWatch::Complete( CombatEventLog* pobCombatEventLog )
{
	UNUSED ( pobCombatEventLog );
	return false;
}

//////////////////////////////////////////////
//
//	CTutorialSimpleStrikeWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CTutorialSimpleStrikeWatch::CTutorialSimpleStrikeWatch( const CEntity* pobEnt, const CHashedString obMsg, int iAttackClass, bool bSuccessful )
: CCombatTutorialBaseWatch( pobEnt, obMsg )
, m_iAttackClass ( iAttackClass )
, m_bSuccessful ( bSuccessful )
{
}

//////////////////////////////////////////////
//
//	CTutorialSimpleStrikeWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CTutorialSimpleStrikeWatch::Complete( CombatEventLog* pobCombatEventLog )
{
	int iEvents = pobCombatEventLog->GetEventCount();
	const CombatEvent* pobCombatEvent = pobCombatEventLog->GetEvents();

	for (int i = 0; i < iEvents; ++i)
	{	
		if ( pobCombatEvent[i].m_eEventType  & 
			( CE_CAUSED_IMPACT_STAGGER | CE_CAUSED_BLOCK_STAGGER | ( m_bSuccessful ? 0 : CE_CAUSED_DEFLECT )
			| CE_CAUSED_RECOIL| CE_CAUSED_KO | CE_CAUSED_KILL ) )
		{
			CAttackData* pobAttack = (CAttackData*)pobCombatEvent[i].m_pData;

			if ( pobAttack->m_eAttackClass == m_iAttackClass )
			{
				if( m_pobReplyEnt->GetMessageHandler() )
					CMessageSender::SendEmptyMessage( m_obReplyMsg, m_pobReplyEnt->GetMessageHandler());

				return true;
			}
		}
	}

	return false;
}


//////////////////////////////////////////////
//
//	CTutorialSimpleEventWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CTutorialSimpleEventWatch::CTutorialSimpleEventWatch( const CEntity* pobEnt, const CHashedString obMsg, int iEvent )
: CCombatTutorialBaseWatch( pobEnt,  obMsg  )
, m_iEvent ( iEvent)
{
}

//////////////////////////////////////////////
//
//	CTutorialSimpleEventWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CTutorialSimpleEventWatch::Complete( CombatEventLog* pobCombatEventLog )
{
	int iEvents = pobCombatEventLog->GetEventCount();
	const CombatEvent* pobCombatEvent = pobCombatEventLog->GetEvents();

	for (int i = 0; i < iEvents; ++i)
	{	
		if ( pobCombatEvent[i].m_eEventType == m_iEvent )
		{
			if( m_pobReplyEnt->GetMessageHandler() )
				CMessageSender::SendEmptyMessage( m_obReplyMsg, m_pobReplyEnt->GetMessageHandler());

			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////
//
//	CTutorialDeflectWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CTutorialDeflectWatch::CTutorialDeflectWatch( const CEntity* pobEnt, const CHashedString obMsg, int iStance  )
: CCombatTutorialBaseWatch( pobEnt,  obMsg  )
, m_iStance ( iStance )
{
}

//////////////////////////////////////////////
//
//	CTutorialDeflectWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CTutorialDeflectWatch::Complete( CombatEventLog* pobCombatEventLog )
{
	int iEvents = pobCombatEventLog->GetEventCount();
	const CombatEvent* pobCombatEvent = pobCombatEventLog->GetEvents();

	for (int i = 0; i < iEvents; ++i)
	{	
		if ( pobCombatEvent[i].m_eEventType == CE_GOT_DEFLECT )
		{
			CAttackData* pobAttack = (CAttackData*)pobCombatEvent[i].m_pData;

			bool bRequiredStance = false;

			switch ( m_iStance )
			{
			case ST_SPEED:
				if ( ( pobAttack->m_eAttackClass == AC_SPEED_MEDIUM ) || ( pobAttack->m_eAttackClass == AC_SPEED_FAST ) )
					bRequiredStance = true;
				break;

			case ST_RANGE:
				if ( ( pobAttack->m_eAttackClass == AC_RANGE_MEDIUM ) || ( pobAttack->m_eAttackClass == AC_RANGE_FAST ) )
					bRequiredStance = true;
				break;

			case ST_POWER:
				if ( ( pobAttack->m_eAttackClass == AC_POWER_MEDIUM ) || ( pobAttack->m_eAttackClass == AC_POWER_FAST ) )
					bRequiredStance = true;
				break;

			default:
				ntError_p( 0, ("Unrecognised stance") );
				break;
			} // switch ( m_iStance )

			if ( bRequiredStance )
			{
				if( m_pobReplyEnt->GetMessageHandler() )
					CMessageSender::SendEmptyMessage( m_obReplyMsg, m_pobReplyEnt->GetMessageHandler() );

				return true;
			}
		}
	}

	return false;
}

//////////////////////////////////////////////
//
//	CTutorialEvadeAttackWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CTutorialEvadeAttackWatch::CTutorialEvadeAttackWatch( const CEntity* pobEnt, const CHashedString obMsg/*, int iAttackClass*/, bool bSuccessful  )
: CCombatTutorialBaseWatch( pobEnt,  obMsg  )
, m_eState ( EAW_EVADE )
//, m_iAttackClass ( iAttackClass )
, m_bSuccessful ( bSuccessful )
{
}

//////////////////////////////////////////////
//
//	CTutorialEvadeAttackWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CTutorialEvadeAttackWatch::Complete( CombatEventLog* pobCombatEventLog )
{

		int iEvents = pobCombatEventLog->GetEventCount();
		const CombatEvent* pobCombatEvent = pobCombatEventLog->GetEvents();

		for (int i = 0; i < iEvents; ++i)
		{	
			switch (m_eState)
			{
			case EAW_EVADE:
				// We've started an evade attack
				if ( pobCombatEvent[i].m_eEventType == CE_STARTED_EVADE_ATTACK )
				{
					m_eState = EAW_ATTACK;
				}
				break;

			case EAW_ATTACK:
				// Oh dear, our evade attack must have missed
				if ( pobCombatEvent[i].m_eEventType == CE_STARTED_ATTACK )
				{
					m_eState = EAW_EVADE;
				}
				else
				if ( pobCombatEvent[i].m_eEventType  & 
					( CE_CAUSED_IMPACT_STAGGER | CE_CAUSED_BLOCK_STAGGER | ( m_bSuccessful ? 0 : CE_CAUSED_DEFLECT )
					| CE_CAUSED_RECOIL| CE_CAUSED_KO | CE_CAUSED_KILL ) )
				{
					//CAttackData* pobAttack = (CAttackData*)pobCombatEvent[i].m_pData;
					//if ( pobAttack->m_eAttackClass == m_iAttackClass )
					{
						if( m_pobReplyEnt->GetMessageHandler() )
							CMessageSender::SendEmptyMessage( m_obReplyMsg, m_pobReplyEnt->GetMessageHandler());

						return true;
					}
				}
				break;

			default:
				// Definatly shouldn't be here
				ntError ( 0 );
				break;
			} // switch (m_eState)
		}

	return false;
}


//////////////////////////////////////////////
//
//	CTutorialAttackTargetWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CTutorialAttackTargetWatch::CTutorialAttackTargetWatch( const CEntity* pobEnt, const CHashedString obMsg, int iAttackTarget, bool bSuccessful )
: CCombatTutorialBaseWatch( pobEnt,  obMsg  )
, m_iAttackTarget ( iAttackTarget )
, m_bSuccessful ( bSuccessful )
{
}

//////////////////////////////////////////////
//
//	CTutorialAttackTargetWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CTutorialAttackTargetWatch::Complete( CombatEventLog* pobCombatEventLog )
{
	int iEvents = pobCombatEventLog->GetEventCount();
	const CombatEvent* pobCombatEvent = pobCombatEventLog->GetEvents();

	for (int i = 0; i < iEvents; ++i)
	{	
		if ( pobCombatEvent[i].m_eEventType  & 
			( CE_CAUSED_IMPACT_STAGGER | CE_CAUSED_BLOCK_STAGGER | ( m_bSuccessful ? 0 : CE_CAUSED_DEFLECT )
			| CE_CAUSED_RECOIL| CE_CAUSED_KO | CE_CAUSED_KILL ) )
		{
			CAttackData* pobAttack = (CAttackData*)pobCombatEvent[i].m_pData;

			if ( pobAttack->m_eTargetType == m_iAttackTarget )
			{
				if( m_pobReplyEnt->GetMessageHandler() )
					CMessageSender::SendEmptyMessage( m_obReplyMsg, m_pobReplyEnt->GetMessageHandler());

				return true;
			}
		}
	}

	return false;
}

//////////////////////////////////////////////
//
//	CTutorialSuperstyleWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CTutorialSuperstyleWatch::CTutorialSuperstyleWatch( const CEntity* pobEnt, const CHashedString obMsg, int iLevel )
: CCombatTutorialBaseWatch( pobEnt,  obMsg  )
, m_iLevel ( iLevel )
{
}

//////////////////////////////////////////////
//
//	CTutorialSuperstyleWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CTutorialSuperstyleWatch::Complete( CombatEventLog* pobCombatEventLog )
{
	int iEvents = pobCombatEventLog->GetEventCount();
	const CombatEvent* pobCombatEvent = pobCombatEventLog->GetEvents();

	for (int i = 0; i < iEvents; ++i)
	{	
		if ( pobCombatEvent[i].m_eEventType == CE_STARTED_SUPERSTYLE )
		{
			int iHitLevel = (int)pobCombatEvent[i].m_pData;

			if (iHitLevel == m_iLevel )
			{
				if( m_pobReplyEnt->GetMessageHandler() )
					CMessageSender::SendEmptyMessage( m_obReplyMsg, m_pobReplyEnt->GetMessageHandler());
				return true;
			}	
		}
	}

	return false;
}

//////////////////////////////////////////////
//
//	CTutorialStanceWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CTutorialStanceWatch::CTutorialStanceWatch( const CEntity* pobEnt, const CHashedString obMsg, int iStance)
: CCombatTutorialBaseWatch( pobEnt,  obMsg  )
, m_iStance ( iStance )
{
}

//////////////////////////////////////////////
//
//	CTutorialStanceWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CTutorialStanceWatch::Complete( CombatEventLog* pobCombatEventLog )
{
	int iEvents = pobCombatEventLog->GetEventCount();
	const CombatEvent* pobCombatEvent = pobCombatEventLog->GetEvents();

	for (int i = 0; i < iEvents; ++i)
	{	
		if ( pobCombatEvent[i].m_eEventType == CE_CHANGE_STANCE )
		{
			int iStance = (int)pobCombatEvent[i].m_pData;

			if ( iStance == m_iStance )
			{
				if( m_pobReplyEnt->GetMessageHandler() )
					CMessageSender::SendEmptyMessage( m_obReplyMsg, m_pobReplyEnt->GetMessageHandler());
				return true;
			}	
		}
	}

	return false;
}

//////////////////////////////////////////////
//
//	CTutorialSpecificStrikeWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CTutorialSpecificStrikeWatch::CTutorialSpecificStrikeWatch( const CEntity* pobEnt, const CHashedString obMsg, const CHashedString* pobSpecificStrike, bool bSuccessful )
: CCombatTutorialBaseWatch( pobEnt, obMsg )
,	m_bSuccessful ( bSuccessful )
{
	m_pobSpecificStrike = ObjectDatabase::Get().GetPointerFromName<CAttackData*>( *pobSpecificStrike );
}

//////////////////////////////////////////////
//
//	CTutorialSpecificStrikeWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CTutorialSpecificStrikeWatch::Complete( CombatEventLog* pobCombatEventLog )
{
	int iEvents = pobCombatEventLog->GetEventCount();
	const CombatEvent* pobCombatEvent = pobCombatEventLog->GetEvents();

	for (int i = 0; i < iEvents; ++i)
	{	
		if ( pobCombatEvent[i].m_eEventType & ( CE_CAUSED_IMPACT_STAGGER | CE_CAUSED_BLOCK_STAGGER | ( m_bSuccessful ? 0 : CE_CAUSED_DEFLECT )
			| CE_CAUSED_RECOIL| CE_CAUSED_KO | CE_CAUSED_KILL ) )
		{
			CAttackData* pobAttack = (CAttackData*)pobCombatEvent[i].m_pData;

			if ( pobAttack == m_pobSpecificStrike )
			{
				if( m_pobReplyEnt->GetMessageHandler() )
					CMessageSender::SendEmptyMessage( m_obReplyMsg, m_pobReplyEnt->GetMessageHandler());

				return true;
			}
		}
	}

	return false;
}

//////////////////////////////////////////////
//
//	CArcherTutorialBaseWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CArcherTutorialBaseWatch::CArcherTutorialBaseWatch( const CEntity* pobEnt, const CHashedString obMsg  )
:	CTutorialBaseWatch( pobEnt, obMsg  )
{}

//////////////////////////////////////////////
//
//	CArcherTutorialBaseWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CArcherTutorialBaseWatch::Complete( ArcherEventLog* pobArcherEventLog )
{
	UNUSED ( pobArcherEventLog );
	return false;
}

//////////////////////////////////////////////
//
//	CArcherSimpleEventWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CArcherSimpleEventWatch::CArcherSimpleEventWatch( const CEntity* pobEnt, const CHashedString obMsg, int iEvent )
: CArcherTutorialBaseWatch( pobEnt,  obMsg  )
, m_iEvent ( iEvent)
{
}

//////////////////////////////////////////////
//
//	CArcherSimpleEventWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CArcherSimpleEventWatch::Complete( ArcherEventLog* pobArcherEventLog )
{
	int iEvents = pobArcherEventLog->GetEventCount();
	const ArcherEvent* pobArcherEvent = pobArcherEventLog->GetEvents();

	for (int i = 0; i < iEvents; ++i)
	{	
		if ( pobArcherEvent[i].m_eEventType == m_iEvent )
		{
			if( m_pobReplyEnt->GetMessageHandler() )
				CMessageSender::SendEmptyMessage( m_obReplyMsg, m_pobReplyEnt->GetMessageHandler());

			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////
//
//	CEntityTutorialBaseWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CEntityTutorialBaseWatch::CEntityTutorialBaseWatch(const CEntity* pobEntity, const CEntity* pobReplyEnt, const CHashedString obMsg )
:	CTutorialBaseWatch( pobReplyEnt, obMsg  )
,	m_pobEntity ( pobEntity )
{}

//////////////////////////////////////////////
//
//	CCombatTutorialBaseWatch::Complete
//
//	Has the action been completed?  True when done.
//
//////////////////////////////////////////////
bool	CEntityTutorialBaseWatch::Complete(  )
{
	return false;
}

//////////////////////////////////////////////
//
//	CEntityAttackWindowWatch::Ctor & Dtor
//
//////////////////////////////////////////////
CEntityAttackWindowWatch::CEntityAttackWindowWatch( const CEntity* pobEntity, const CEntity* pobReplyEnt, 
												   const CHashedString obEnterMsg, const CHashedString obExitMsg, ATK_WINDOW eAtkWindow )	
:	CEntityTutorialBaseWatch( pobEntity, pobReplyEnt, obExitMsg  )
,	m_eAtkWindow ( eAtkWindow )
,	m_eAtkWindowState ( AWS_INIT )
,	m_obEnterMsg ( obEnterMsg )
{}


bool	CEntityAttackWindowWatch::Complete( )
{
	ntAssert ( m_pobEntity->GetAttackComponent() );

	// Get state of interesting window
	int iInWindow = 0;

	switch ( m_eAtkWindow )
	{
	case AW_INVUNERABLE:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInInvulnerabilityWindow( );
		break;
	case AW_STRIKE:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInStrikeWindow( );
		break;
	case AW_STRIKE2:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInStrike2Window( );
		break;
	case AW_ATTACK_POP_OUT:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInAttackWindow( );
		break;
	case AW_BLOCK_POP_OUT:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInBlockWindow( );
		break;
	case AW_NO_LOCK:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInNoLockWindow( );
		break;
	case AW_MOVEMENT_POP_OUT:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInMovementWindow( );
		break;
	case AW_UNINTERRUPTABLE:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInUninterruptibleWindow( );
		break;
	case AW_NO_COLLIDE:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInNoCollideWindow( );
		break;
	case AW_NEXTMOVE:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInNextMoveWindow( );
		break;
	case AW_RANGE_INTERCEPT:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInRangeInterceptWindow( );
		break;
	case AW_INTERCEPT:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInInterceptWindow( );
		break;
	case AW_EVADE_AND_GRAB_DENY:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInEvadeAndGrabDenyWindow( );
		break;
	case AW_VICTIM_RAGDOLLABLE:
		iInWindow = m_pobEntity->GetAttackComponent()->IsInVictimRagdollableWindow( );
		break;
	default:
		break;
	} // m_eAtkWindow


	// if moved to in send enter msg
	if ( m_eAtkWindowState == AWS_INIT && iInWindow > 0 )
	{
		if( m_pobReplyEnt->GetMessageHandler() )
			CMessageSender::SendEmptyMessage( m_obEnterMsg, m_pobReplyEnt->GetMessageHandler());

		m_eAtkWindowState = AWS_IN;

	}

	// if moved to out send exit meg and return true as all done
	else if ( m_eAtkWindowState == AWS_IN && iInWindow == 0 )
	{
		if( m_pobReplyEnt->GetMessageHandler() )
			CMessageSender::SendEmptyMessage( m_obReplyMsg, m_pobReplyEnt->GetMessageHandler());

		m_eAtkWindowState = AWS_OUT;

		// FIX ME: will need to expand this if flip flops with multiple windows becomes relavent
		return true;
	}

	return false;
}

//////////////////////////////////////////////
//
//	CTutorialManager::Ctor & Dtor
//
//////////////////////////////////////////////
CTutorialManager::CTutorialManager( void )
:	m_bRegistered ( false )
,	m_obTextPosition ( CPoint ( 0.5f, 0.75f, 0.0f) )
,	m_pobScreenMsg ( 0 )
,	m_fLastTimeScalar ( 1.0f )
,	m_fAimTimeScalar ( 1.0f )
,	m_fCurrTimeScalar ( 1.0f )
,	m_fBlendTime ( 0.0f )
,	m_fCurrBlendTime ( 0.0f )
,	m_pobScalarCallbackEnt ( 0 )
,	m_obScalarCallbackMsg ( "" )
{	
	m_aobTimeScalarQue.clear();
}

CTutorialManager::~CTutorialManager( void )
{
	ClearWatches();
}

//////////////////////////////////////////////
//
//	CTutorialManager::Update
//
//	Per fame update tasks
//
//////////////////////////////////////////////
void CTutorialManager::Update( void )
{
	// Register our interest if able
	if (!m_bRegistered  && CEntityManager::Get().GetPlayer() )
	{
		Player* pPlayer = CEntityManager::Get().GetPlayer();

		if ( pPlayer->IsArcher() )
		{
			// Which events are we interested in?
			m_obArcherEventLog.SetFlags( -1 );

			pPlayer->ToArcher()->RegisterArcherEventLog( &m_obArcherEventLog );
		}

		m_bRegistered = true;
	}

	// Look at the entities we have regstered
	for( CTLIt obListIt = m_aobCombatTutorialList.begin(); obListIt != m_aobCombatTutorialList.end(); )
	{
		CombatTutorialContainer* pobCTC = obListIt->second;

		// Look at combat events we care about
		for( CombatWatchList::iterator obIt = pobCTC->m_obCombatWatchList.begin(); obIt != pobCTC->m_obCombatWatchList.end(); )
		{
			if ( (*obIt)->Complete( pobCTC->m_pobCombatEventLog ) )
			{
				// Remove when done
				NT_DELETE ( (*obIt) );
				obIt = pobCTC->m_obCombatWatchList.erase( obIt );
			}
			else
				++obIt;
		}

		// Done with the log now, so clear out ready for the next update
		pobCTC->m_pobCombatEventLog->ClearEvents();

		if ( pobCTC->m_obCombatWatchList.empty() )
		{
			// Remove when all watches done
			NT_DELETE ( pobCTC );
			obListIt = m_aobCombatTutorialList.erase( obListIt );
		}
		else
		{
			++obListIt;
		}
	}
	
	// Look at archer events we care about
	for( ArcherWatchList::iterator obIt = m_obArcherWatchList.begin(); obIt != m_obArcherWatchList.end(); )
	{
		if ( (*obIt)->Complete( &m_obArcherEventLog ) )
		{
			NT_DELETE ( (*obIt) );
			obIt = m_obArcherWatchList.erase( obIt );
		}
		else
			++obIt;
	}
	
	// Done with them now, so clear out ready for the next update
	m_obArcherEventLog.ClearEvents();

	// Look at entity events we care about
	for( EntityWatchList::iterator obIt = m_obEntityWatchList.begin(); obIt != m_obEntityWatchList.end(); )
	{
		if ( (*obIt)->Complete( ) )
		{
			NT_DELETE ( (*obIt) );
			obIt = m_obEntityWatchList.erase( obIt );
		}
		else
			++obIt;
	}

	// Tutorial Time scalar
#if 0 //def _DEBUG
	float fTimeScalar = CTimer::Get().GetGameTimeScalar();
	g_VisualDebug->Printf2D( 5.0f, 40.0f, DC_GREEN, 0, "Tutorial Time Scalar %f\n", fTimeScalar );
#endif // _DEBUG


	// Currently doing a tutorial time scalar blend
	if ( m_fCurrBlendTime > 0.0f )
	{
		m_fCurrBlendTime -= CTimer::Get().GetSystemTimeChange();

		// Bend has ended
		if ( m_fCurrBlendTime <= 0.0f )
		{
			m_fCurrBlendTime = 0.0f;

			m_fLastTimeScalar = m_fCurrTimeScalar = m_fAimTimeScalar;

			CTimer::Get().SetTutorialTimeScalar( m_fLastTimeScalar );

			// Callback when que empty
			if ( m_aobTimeScalarQue.empty() && m_pobScalarCallbackEnt && m_pobScalarCallbackEnt->GetMessageHandler() )				
			{
				CMessageSender::SendEmptyMessage( m_obScalarCallbackMsg, m_pobScalarCallbackEnt->GetMessageHandler() );

				m_pobScalarCallbackEnt = 0;
			}
		}
		else
		// Update blend
		{			
			float fTimeScalarDelta = m_fAimTimeScalar - m_fLastTimeScalar;

			m_fCurrTimeScalar = m_fLastTimeScalar + fTimeScalarDelta *
				CMaths::SmoothStep(1.0f - m_fCurrBlendTime / m_fBlendTime );

			CTimer::Get().SetTutorialTimeScalar( m_fCurrTimeScalar );
		}
	}
	else
	{
		// need to start a new blend
		if ( !m_aobTimeScalarQue.empty() )
		{
			ntstd::pair < float, float > obPair = m_aobTimeScalarQue.front();

			m_fAimTimeScalar = obPair.first;
			m_fCurrBlendTime = m_fBlendTime = obPair.second;

			m_aobTimeScalarQue.pop_front();
		}
	}
}

////////////////////////////////////////////////
//
//	FUNCTION		CTutorialManager::AddScreenMessage
//
//	DESCRIPTION		Create a message on the current screen
//
////////////////////////////////////////////////
bool CTutorialManager::AddScreenMessage(const char* pcTextID, const char* pcFont, float fDuration)
{
	UNUSED ( pcFont );

	if ( !CHud::Exists() )
		return false;

	ntstd::Vector<ntstd::String> obStringList;
	obStringList.push_back( ntstd::String( pcTextID ) );
	
	CHud::Get().CreateMessageBox( obStringList, m_obTextPosition.X(), m_obTextPosition.Y(),
										0.1f, 0.1f, 0.5f, 0.5f );

	if (fDuration > 0.0f )
	{
		CHud::Get().RemoveMessageBox( fDuration );
	}

	return true;
}

////////////////////////////////////////////////
//
//	FUNCTION		CTutorialManager::ScreenMessagePosition
//
//	DESCRIPTION		Set position for subsiquent screen messages
//
////////////////////////////////////////////////
void CTutorialManager::ScreenMessagePosition(float fX, float fY)
{
	m_obTextPosition.X() = fX;
	m_obTextPosition.Y() = fY;
	m_obTextPosition.Z() = 0.0f;
}

////////////////////////////////////////////////
//
//	FUNCTION		CTutorialManager::RemoveScreenMessage
//
//	DESCRIPTION		Create a message on the current screen
//
////////////////////////////////////////////////
void CTutorialManager::RemoveScreenMessage()
{
	if ( !CHud::Exists() )
		return;

	CHud::Get().RemoveMessageBox( );
}

//////////////////////////////////////////////
//
//	CTutorialManager::ClearWatches
//
//	Clear out any remaining watches
//
//////////////////////////////////////////////
void	CTutorialManager::ClearWatches( void )
{
	for( CTLIt obIt = m_aobCombatTutorialList.begin(); obIt != m_aobCombatTutorialList.end(); ++obIt)
	{
		NT_DELETE ( (obIt->second) );
	}

	m_aobCombatTutorialList.clear();

	for( ArcherWatchList::iterator obIt = m_obArcherWatchList.begin(); obIt != m_obArcherWatchList.end(); ++obIt)
		NT_DELETE ( (*obIt) );

	m_obArcherWatchList.clear();

	for( EntityWatchList::iterator obIt = m_obEntityWatchList.begin(); obIt != m_obEntityWatchList.end(); )
			NT_DELETE ( (*obIt) );

	m_obEntityWatchList.clear();
}

//////////////////////////////////////////////
//
//	CTutorialManager::AddWatch
//
//	Add a new tutorial event to be watched.
//
//////////////////////////////////////////////
void CTutorialManager::AddWatch( const CHashedString obEvent, const CEntity* pobReplyEnt, const CHashedString obMsg, void* pParam, CEntity* pobWatchEnt )
{
	CCombatTutorialBaseWatch* pobWatch = 0;
	CArcherTutorialBaseWatch* pobArcherWatch = 0;

	CEntity* pobTargEnt = pobWatchEnt;

	if ( !pobTargEnt )
	{
		pobTargEnt = CEntityManager::Get().GetPlayer();
	}

	switch ( obEvent.GetHash() )
	{
	case 0xa4e4903f: //SPEED_MEDIUM
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_SPEED_MEDIUM, false );
		break;
	case 0x577716ef: //SPEED_FAST
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_SPEED_FAST, false );
		break;
	case 0x4a8514b4: //RANGE_MEDIUM
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_RANGE_MEDIUM, false );
		break;
	case 0x2a7f539: //RANGE_FAST
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_RANGE_FAST, false );
		break;
	case 0x77fb1bbb: //POWER_MEDIUM
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_POWER_MEDIUM, false );
		break;
	case 0x2428f79c: //POWER_FAST
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_POWER_FAST, false );
		break;

	case 0xb6fac4b0: //EVADE_STRIKE
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialEvadeAttackWatch( pobReplyEnt, obMsg, false );
		break;

	case 0x92588cfa: //GROUND_ATTACK
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialAttackTargetWatch( pobReplyEnt, obMsg, (int)AT_TYPE_DOWN, false );
		break;

	case 0x1ea8a333: //SPEED_MEDIUM_HIT
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_SPEED_MEDIUM, true );
		break;
	case 0xa39c07e7: //SPEED_FAST_HIT
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_SPEED_FAST, true );
		break;
	case 0xf998b4cc: //RANGE_MEDIUM_HIT
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_RANGE_MEDIUM, true );
		break;
	case 0x913eef3f: //RANGE_FAST_HIT
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_RANGE_FAST, true );
		break;
	case 0x4373fbce: //POWER_MEDIUM_HIT
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_POWER_MEDIUM, true );
		break;
	case 0x8619d31f: //POWER_FAST_HIT
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleStrikeWatch( pobReplyEnt, obMsg, (int)AC_POWER_FAST, true );
		break;

	case 0xe7dfcda5: //EVADE_STRIKE_HIT
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialEvadeAttackWatch( pobReplyEnt, obMsg, true );
		break;

	case 0xcdcd933e: //GROUND_ATTACK_HIT
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialAttackTargetWatch( pobReplyEnt, obMsg, (int)AT_TYPE_DOWN, true );
		break;



	case 0xc6f0e441: //GRAB
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleEventWatch( pobReplyEnt, obMsg, (int)CE_SUCCESSFUL_GRAB );
		break;
	case 0xa453574c: //EVADE
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleEventWatch( pobReplyEnt, obMsg, (int)CE_EVADED_INCOMING_ATTACK );
		break;
	case 0x198959d: //COUNTER
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleEventWatch( pobReplyEnt, obMsg, (int)CE_COUNTER_ATTACK );
		break;
	case 0xce434422: //KILL
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleEventWatch( pobReplyEnt, obMsg, (int)CE_CAUSED_KILL );
		break;
	case 0x4bc827cf: //KO
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleEventWatch( pobReplyEnt, obMsg, (int)CE_CAUSED_KO );
		break;
	case 0x926d5235: //AERIAL
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleEventWatch( pobReplyEnt, obMsg, (int)CE_STARTED_AERIAL );
		break;

	case 0xc6b9b078: //SPEED_BLOCK
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialDeflectWatch( pobReplyEnt, obMsg, (int)ST_SPEED );
		break;
	case 0xa95c177a: //RANGE_BLOCK
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialDeflectWatch( pobReplyEnt, obMsg, (int)ST_RANGE );
		break;
	case 0xfc6cf1f: //POWER_BLOCK
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialDeflectWatch( pobReplyEnt, obMsg, (int)ST_POWER );
		break;

	case 0xe06a96b4: //SPEED_STANCE
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialStanceWatch( pobReplyEnt, obMsg, (int)ST_SPEED );
		break;
	case 0xe0b123f: //RANGE_STANCE
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialStanceWatch( pobReplyEnt, obMsg, (int)ST_RANGE );
		break;
	case 0x33751d30: //POWER_STANCE
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialStanceWatch( pobReplyEnt, obMsg, (int)ST_POWER );
		break;


	case 0x787e9ef9: //SUPERSTYLE_ZERO
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSuperstyleWatch( pobReplyEnt, obMsg, (int)HL_ZERO );
		break;
	case 0x39d6690: //SUPERSTYLE_ONE
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSuperstyleWatch( pobReplyEnt, obMsg, (int)HL_ONE );
		break;
	case 0x683b6a07: //SUPERSTYLE_TWO
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSuperstyleWatch( pobReplyEnt, obMsg, (int)HL_TWO );
		break;
	case 0x751a0f0f: //SUPERSTYLE_THREE
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSuperstyleWatch( pobReplyEnt, obMsg, (int)HL_THREE );
		break;
	case 0x43610916: //SUPERSTYLE_FOUR
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSuperstyleWatch( pobReplyEnt, obMsg, (int)HL_FOUR );
		break;
	case 0x55bc54e9: //SUPERSTYLE_SPECIAL
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSuperstyleWatch( pobReplyEnt, obMsg, (int)HL_SPECIAL );
		break;

	case 0xf79b746e: //VAULT
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_VAULT );
		break;
	case 0x8f71cb61: //CROUCH
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_CROUCH );
		break;
	case 0xadbff62c: //RELOAD
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_RELOAD );
		break;
	case 0x6a07f3be: //FIRSTPERSON_FIRE
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_FIRSTPERSON_FIRE );
		break;
	case 0x4a7f0053: //THIRDPERSON_FIRE
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_THIRDPERSON_FIRE );
		break;
	case 0xed5b56f7: //FIRSTPERSON
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_FIRSTPERSON );
		break;
	case 0x71e2003f: //THIRDPERSON
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_THIRDPERSON );
		break;
	case 0xa3373e43: //HEADSHOT
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_HEADSHOT );
		break;
	case 0x815c97db: //AFTERTOUCH
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_AFTERTOUCH );
		break;
	case 0xaa128a5c: //MOUNT_TURRET
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_MOUNT_TURRET );
		break;
	case 0x1d3c02c4: //DISMOUNT_TURRET
		pobArcherWatch = (CArcherTutorialBaseWatch*)NT_NEW CArcherSimpleEventWatch( pobReplyEnt, obMsg, (int)AE_DISMOUNT_TURRET );
		break;

	case 0x57e0612: //SPECIFIC_STRIKE
		if ( pParam )
		{
			pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSpecificStrikeWatch( pobReplyEnt, obMsg, (CHashedString*)pParam, false );
		}
		break;

	case 0x9f11e17c: //SPECIFIC_STRIKE_HIT
		if ( pParam )
		{
			pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSpecificStrikeWatch( pobReplyEnt, obMsg, (CHashedString*)pParam, true );
		}
		break;

	case 0xe8c502bc: //SELECTED_ATTACK
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleEventWatch( pobReplyEnt, obMsg, (int)CE_SELECTED_ATTACK );
		break;

	case 0x5c466391: //MISSED_COUNTER
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleEventWatch( pobReplyEnt, obMsg, (int)CE_MISSED_COUNTER );
		break;

	case 0xd50cd116: //MISSED_KILL_COUNTER
		pobWatch = (CCombatTutorialBaseWatch*)NT_NEW CTutorialSimpleEventWatch( pobReplyEnt, obMsg, (int)CE_MISSED_KILL_COUNTER );
		break;

	default:
#ifndef _RELEASE
		ntPrintf("Unknown tutorial event type %s\n", ntStr::GetString( obEvent ) );
#endif
		break;
	} // switch (iEventType)

	// Combat Tutorial watch
	if ( pobWatch)
	{
		ntError_p ( pobTargEnt, ("Tutorial: No target entity\n") );
		ntError_p ( pobTargEnt->GetAttackComponent(), ("Tutorial: Target entity does not have attack component\n") );

		CTLIt obIt = m_aobCombatTutorialList.find ( pobTargEnt );

		// Not currently in map
		if ( obIt == m_aobCombatTutorialList.end() )
		{
			CombatTutorialContainer* pobCTC = NT_NEW CombatTutorialContainer( pobTargEnt );
			m_aobCombatTutorialList[ pobTargEnt ] = pobCTC;

			// Add the new watch
			pobCTC->m_obCombatWatchList.push_back( pobWatch );
		}
		else
		{
			CombatTutorialContainer* pobCTC = obIt->second;

			// Add the new watch
			pobCTC->m_obCombatWatchList.push_back( pobWatch );
		}
	}

	// Archer tutorial watch
	if ( pobArcherWatch)
		m_obArcherWatchList.push_back( pobArcherWatch );
}

//////////////////////////////////////////////
//
//	CTutorialManager::AddAttackWindowWatch
//
//	Add a attck window event to be watched.
//
//////////////////////////////////////////////
void CTutorialManager::AddAttackWindowWatch( CEntity* pobWatchEnt, const CHashedString obWindow, const CEntity* pobReplyEnt, const CHashedString obEnterMsg, const CHashedString obExitMsg )
{
	CEntityTutorialBaseWatch* pobWatch = 0;
	
	CEntity* pobTargEnt = pobWatchEnt;

	if ( !pobTargEnt )
	{
		pobTargEnt = CEntityManager::Get().GetPlayer();
	}

	switch ( obWindow.GetHash() )
	{
	case 0xa63e4184: //INVUNERABLE
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_INVUNERABLE );
		break;

	case 0x2c01fb9: //STRIKE
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_STRIKE );
		break;

	case 0x7a689ab7: //STRIKE2
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_STRIKE2 );
		break;
		
	case 0x109cdd27: //ATTACK_POP_OUT
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_ATTACK_POP_OUT );
		break;
		
	case 0x5e653de6: //BLOCK_POP_OUT
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_BLOCK_POP_OUT );
		break;
		
	case 0xa2b467a0: //NO_LOCK
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_NO_LOCK );
		break;
		
	case 0x59daa8fa: //MOVEMENT_POP_OUT
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_MOVEMENT_POP_OUT );
		break;
		
	case 0xbd357078: //UNINTERRUPTABLE
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_UNINTERRUPTABLE );
		break;
		
	case 0x3bef81e7: //NO_COLLIDE
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_NO_COLLIDE );
		break;
		
	case 0x65584cd6: //NEXTMOVE
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_NEXTMOVE );
		break;
		
	case 0x93a2cb22: //RANGE_INTERCEPT
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_RANGE_INTERCEPT );
		break;
		
	case 0xb4413443: //INTERCEPT
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_INTERCEPT );
		break;
		
	case 0x5a99ee9f: //EVADE_AND_GRAB_DENY
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_EVADE_AND_GRAB_DENY );
		break;
		
	case 0x7d2cbdab: //VICTIM_RAGDOLLABLE
		pobWatch = (CEntityTutorialBaseWatch*)NT_NEW CEntityAttackWindowWatch( pobTargEnt, pobReplyEnt, obEnterMsg, obExitMsg, CEntityAttackWindowWatch::AW_VICTIM_RAGDOLLABLE );
		break;

	default:
#ifndef _RELEASE
		ntPrintf("Unknown attack window type %s\n", ntStr::GetString( obWindow ) );
#endif
		break;
	} // switch (iEventType)

	// Event Tutorial watch
	if ( pobWatch)
		m_obEntityWatchList.push_back( pobWatch );
}

//////////////////////////////////////////////
//
//	CTutorialManager::AddTimeScalar
//
//	Add a time scaling blend to the que
//
//////////////////////////////////////////////
void	CTutorialManager::AddTimeScalar ( float fTimeScalar, float fBlendTime )
{
	m_aobTimeScalarQue.push_back ( ntstd::pair < float, float > ( fTimeScalar, fBlendTime ) );
}

//////////////////////////////////////////////
//
//	CTutorialManager::ClearTimeScalar
//
//	Return to normal scaling. Clears the time scalar que.
//
//////////////////////////////////////////////
void	CTutorialManager::ClearTimeScalar ( float fBlendTime )
{
	m_aobTimeScalarQue.clear();
	if ( m_fCurrBlendTime > 0.0f )
	{
		m_fLastTimeScalar = m_fCurrTimeScalar;
	}
	m_aobTimeScalarQue.push_back ( ntstd::pair < float, float > ( 1.0f, fBlendTime ) );
}


//////////////////////////////////////////////
//
//	CTutorialManager::TimeScalarCallback
//
//	Add a callback when the time scalar que is finished
//
//////////////////////////////////////////////
void	CTutorialManager::TimeScalarCallback ( CEntity* pobReplyEnt, CHashedString obMsg )
{
	ntAssert ( pobReplyEnt );

	m_pobScalarCallbackEnt = pobReplyEnt;
	m_obScalarCallbackMsg = obMsg;
}

