/***************************************************************************************************
*
*	DESCRIPTION		Processes controller input and converts it into direct attack requests
*
*	NOTES
*
***************************************************************************************************/

// Necessary includes
#include "attacktracking.h"
#include "game/attacks.h"
#include "game/inputcomponent.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "game/hitcounter.h"
#include "game/awareness.h"
#include "objectdatabase/dataobject.h"
#include "game/randmanager.h"

// How does this tracker function?
#define TAKES_MULTIPLE_REQUESTS ( true )

// Disable Evades when Pad Left is held, but not in release build
#ifndef _RELEASE
#include "camera/camman.h"
#include "camera/camview.h"
#define CHECK_EVADES																		\
	if(!(pobInput->GetHeld() & PAD_LEFT) || !CamMan::GetPrimaryView()->IsDebugCamActive())	\
	{																						\
		if(false);
#define CHECK_EVADES_END }
#else
#define CHECK_EVADES
#define CHECK_EVADES_END
#endif

/***************************************************************************************************
*
*	FUNCTION		CAttackTrackingData::CanLedgeRecover
*
*	DESCRIPTION		Have I get a ledge recover attack?
*
***************************************************************************************************/
bool CAttackTracker::CanLedgeRecover()
{ 
	return m_pobClusterStructure->m_pobLedgeRecoverAttack != 0; 
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTrackingData::CanLedgeRecover
*
*	DESCRIPTION		Have I get a ledge recover attack?
*
***************************************************************************************************/
void CAttackTracker::SelectLedgeRecoverAttack()
{ 
	if (CanLedgeRecover())
	{
		// Reset our data
		Reset();

		m_obRequestedAttack.pobAttackLink = m_pobClusterStructure->m_pobLedgeRecoverAttack;		
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();
		m_obRequestedAttack.eRequestType = AM_ACTION; // Arbitrary choice of type here	
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTrackingData::CAttackTrackingData
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
CAttackTracker::CAttackTrackingData::CAttackTrackingData() 
{ 
	Reset(); 
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTrackingData::operator=
*
*	DESCRIPTION		Assignment
*
***************************************************************************************************/
CAttackTracker::CAttackTrackingData& CAttackTracker::CAttackTrackingData::operator=( const CAttackTrackingData& obOther )
{
	// Copy the data over
	pobAttackLink	= obOther.pobAttackLink; 
	pobAttackData	= obOther.pobAttackData;
	eRequestType	= obOther.eRequestType;

	return *this;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTrackingData::Reset
*
*	DESCRIPTION		Clear out the existing data
*
***************************************************************************************************/
void CAttackTracker::CAttackTrackingData::Reset() 
{	
	pobAttackLink = 0; 
	pobAttackData = 0;
	eRequestType = AM_NONE;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTrackingData::IsComplete
*
*	DESCRIPTION		Make sure an item contains the necessary data
*
***************************************************************************************************/
bool CAttackTracker::CAttackTrackingData::IsComplete( void ) const 
{	
	return ( pobAttackData != 0 && pobAttackLink != 0 ); 
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::CAttackTracker
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
CAttackTracker::CAttackTracker()
:	m_pobClusterStructure( 0 ),
	m_bDirectMode( false ),
	m_bWaiting( false )
{
	// Make sure that our tracking data is clear
	m_obCurrentAttack.Reset();
	m_obRequestedAttack.Reset();
	
	m_iLastRandomStyleIndex = -1;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::~CAttackTracker
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/
CAttackTracker::~CAttackTracker()
{
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::InitialiseAttackStructure
*
*	DESCRIPTION		Set the attack data structure that this tracker should track
*
***************************************************************************************************/
void CAttackTracker::InitialiseAttackStructure( const CClusterStructure* pobClusterStructure )
{
	// Make sure we are not in direct mode
	ntAssert( !m_bDirectMode );

	// Check the argument and assign the value
	ntAssert( pobClusterStructure );
	m_pobClusterStructure = pobClusterStructure;

	// Move to the beginning of the attack structure
	Reset();
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectSpecificAttackFromVulnerabilityZone
*
*	DESCRIPTION		Little bit hacky as it could be used to completely override current attack.
*					But should only be used when swapping out attack data with new attack data
*					got after checking for specific attack vulnerability zones on a target.
*
***************************************************************************************************/
bool CAttackTracker::SelectSpecificAttackFromVulnerabilityZone(const CAttackLink* pobNewLink)
{
	// Get what our attack type was
	ATTACK_MOVE_TYPE eAttackType = m_obRequestedAttack.eRequestType;
	// See if we can swap it out with something from this specific vulnerability link
	const CAttackLink* pobNewRequested = pobNewLink->GetNextAttackNode(eAttackType);

	// If we can, do
	if (pobNewRequested)
	{
		// Reset our data
		Reset();
	
		m_obRequestedAttack.pobAttackLink = pobNewRequested;		
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();
		m_obRequestedAttack.eRequestType = eAttackType;		

		if (m_obRequestedAttack.pobAttackLink->m_pobButtonHeldAttack)
		{
			m_obRequestedButtonHeldAttack.pobAttackLink = m_obRequestedAttack.pobAttackLink->m_pobButtonHeldAttack;
			m_obRequestedButtonHeldAttack.pobAttackData = m_obRequestedButtonHeldAttack.pobAttackLink->GetAttackDataP();
			m_obRequestedButtonHeldAttack.eRequestType = eAttackType;	
		}

		return true;
	}

	// If we can't, nothing's changed
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::ForceRequestedAttackToBe
*
*	DESCRIPTION		This can be used to completely override current attack. ONLY FOR WELDER!
*
***************************************************************************************************/
bool CAttackTracker::ForceRequestedAttackToBe(const CAttackLink* pobNewLink)
{
	ntError( pobNewLink );

	ntPrintf("*** FORCING AN ATTACK LINK ONTO A TRACKER ***\n*** If you see this and you're NOT using the anim event editor or Welder, someone needs pain. Ask Duncan to inflict it. ***\n");

	// Reset our data
	Reset();

	m_obRequestedAttack.pobAttackLink = pobNewLink;		
	m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();
	m_obRequestedAttack.eRequestType = m_obRequestedAttack.eRequestType;		

	if (m_obRequestedAttack.pobAttackLink->m_pobButtonHeldAttack)
	{
		m_obRequestedButtonHeldAttack.pobAttackLink = m_obRequestedAttack.pobAttackLink->m_pobButtonHeldAttack;
		m_obRequestedButtonHeldAttack.pobAttackData = m_obRequestedButtonHeldAttack.pobAttackLink->GetAttackDataP();
		m_obRequestedButtonHeldAttack.eRequestType = m_obRequestedAttack.eRequestType;	
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::MoveToRequested
*
*	DESCRIPTION		This checks then moves the requested attack data into the current attack data
*					position, making it accessable for use.
*
***************************************************************************************************/
void CAttackTracker::MoveToRequested( void )
{
	// Make sure we have the data we need
	ntAssert( m_obRequestedAttack.IsComplete() );

	//ntPrintf("AttackTracker: Moving from %s, to %s\n",ObjectDatabase::Get().GetNameFromPointer(m_obCurrentAttack.pobAttackData),ObjectDatabase::Get().GetNameFromPointer(m_obRequestedAttack.pobAttackData) );
	
	ntAssert(m_obRequestedAttack.pobAttackLink);
	ntAssert(m_obRequestedAttack.pobAttackData);

	// Move the data
	m_obCurrentAttack = m_obRequestedAttack;

	// Clear out the request information
	ClearRequested();
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SwitchToRequestedButtonHeldAttack
*
*	DESCRIPTION		Switch over the the button held attack link
*
***************************************************************************************************/
bool CAttackTracker::SwitchToRequestedButtonHeldAttack()
{
	if (m_obRequestedButtonHeldAttack.IsComplete())
	{
		m_obRequestedAttack = m_obRequestedButtonHeldAttack;
		m_obRequestedButtonHeldAttack.Reset();
		//ntPrintf("Switching to %s.\n", ObjectDatabase::Get().GetNameFromPointer(m_obRequestedAttack.pobAttackData));

		return true;
	}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::WillTakeAttackRequests
*
*	DESCRIPTION		Can another request for an attack be made - depends on whether we are set 
*					to recieve multiple attack requests
*
***************************************************************************************************/
bool CAttackTracker::WillTakeAttackRequests( void ) const
{
	// Either we don't have an existing request or we allow multiple
	return ( ( !RequestIsReady() )
		     ||  
			 ( ( TAKES_MULTIPLE_REQUESTS ) && ( !m_bDirectMode ) ) );
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::CurrentAttackStillRequested
*
*	DESCRIPTION		Check to see if the current attack is still being requested.  We ignore the
*					stance 'modifier' here, we only need to check the actual button press.
*
***************************************************************************************************/
bool CAttackTracker::CurrentAttackStillRequested( CInputComponent& obInput, float fAttackTime ) const
{
	// This will never be the case in direct mode
	if ( m_bDirectMode )
		return false;

	// If theres a new requested attack then the current attack isn't requested anymore! JML
	if( m_obRequestedAttack.pobAttackData )
		return false;

	// Switch on the current attack type
	switch ( m_obCurrentAttack.eRequestType )
	{
	case AM_POWER_FAST:
	case AM_SPEED_FAST:
	case AM_RANGE_FAST:		
		if ( obInput.GetVHeldTime( AB_ATTACK_FAST ) >= fAttackTime )	
			return true; 
		break;

	case AM_RANGE_MEDIUM:
	case AM_POWER_MEDIUM:
	case AM_SPEED_MEDIUM:	
		if ( obInput.GetVHeldTime( AB_ATTACK_MEDIUM ) >= fAttackTime )	
			return true; 
		break;

	case AM_RANGE_GRAB:
	case AM_POWER_GRAB:
	case AM_SPEED_GRAB:		
		if ( obInput.GetVHeldTime( AB_GRAB ) >= fAttackTime )			
			return true; 
		break;

	case AM_ACTION:			
		if ( obInput.GetVHeldTime( AB_ACTION ) >= fAttackTime )			
			return true; 
		break;

	case AM_DODGE_LEFT:		
		if ( obInput.GetVHeldTime( AB_DODGE_LEFT ) >= fAttackTime )		
			return true; 
		break;

	case AM_DODGE_RIGHT:	
		if ( obInput.GetVHeldTime( AB_DODGE_RIGHT ) >= fAttackTime )	
			return true; 
		break;

	case AM_DODGE_FORWARD:	
		if ( obInput.GetVHeldTime( AB_DODGE_FORWARD ) >= fAttackTime )	
			return true; 
		break;

	case AM_DODGE_BACK:		
		if ( obInput.GetVHeldTime( AB_DODGE_BACK ) >= fAttackTime )		
			return true; 
		break;

	case AM_NONE:
		break;

	default:			
		break;
	}

	// Just in case
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::ButtonHeldAttackStillRequested
*
*	DESCRIPTION		Check to see if the button held attack is still being requested
*
***************************************************************************************************/
bool CAttackTracker::ButtonHeldAttackStillRequested( CInputComponent& obInput ) const
{
	// This will never be the case in direct mode
	if ( m_bDirectMode )
		return false;

	if (!m_obRequestedButtonHeldAttack.IsComplete())
		return false;

	// Switch on the current attack type
	switch ( m_obRequestedButtonHeldAttack.eRequestType )
	{
	case AM_POWER_FAST:
	case AM_SPEED_FAST:
	case AM_RANGE_FAST:		
		if ( obInput.GetVHeld() & ( 1 << AB_ATTACK_FAST ) )	
			return true; 
		break;

	case AM_RANGE_MEDIUM:
	case AM_POWER_MEDIUM:
	case AM_SPEED_MEDIUM:	
		if ( obInput.GetVHeld() & ( 1 << AB_ATTACK_MEDIUM ) )	
			return true; 
		break;

	case AM_RANGE_GRAB:
	case AM_POWER_GRAB:
	case AM_SPEED_GRAB:		
		if ( obInput.GetVHeld() & ( 1 << AB_GRAB ) )			
			return true; 
		break;

	case AM_ACTION:			
		if ( obInput.GetVHeld() & ( 1 << AB_ACTION ) )			
			return true; 
		break;

	case AM_DODGE_LEFT:		
		if ( obInput.GetVHeld() & ( 1 << AB_DODGE_LEFT ) )		
			return true; 
		break;

	case AM_DODGE_RIGHT:	
		if ( obInput.GetVHeld() & ( 1 << AB_DODGE_RIGHT ) )	
			return true; 
		break;

	case AM_DODGE_FORWARD:	
		if ( obInput.GetVHeld() & ( 1 << AB_DODGE_FORWARD ) )	
			return true; 
		break;

	case AM_DODGE_BACK:		
		if ( obInput.GetVHeld() & ( 1 << AB_DODGE_BACK ) )		
			return true; 
		break;

	case AM_NONE:
		break;

	default:			
		break;
	}

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	CAttackTracker::GetAttackRequestTime
//!	Get the length of time that this attack has been requested for.
//!
//------------------------------------------------------------------------------------------
float CAttackTracker::GetAttackRequestTime(CInputComponent& input) const
{
	// This will never be the case in direct mode
	if(m_bDirectMode)
		return 0.f;

	// If theres a new requested attack then the current attack isn't requested anymore! JML
	if(m_obRequestedAttack.pobAttackData)
		return 0.f;

	// Switch on the current attack type
	switch(m_obCurrentAttack.eRequestType)
	{
	case AM_POWER_FAST:
	case AM_SPEED_FAST:
	case AM_RANGE_FAST:		
		return input.GetVHeldTime(AB_ATTACK_FAST);

	case AM_RANGE_MEDIUM:
	case AM_POWER_MEDIUM:
	case AM_SPEED_MEDIUM:	
		return input.GetVHeldTime(AB_ATTACK_MEDIUM);

	case AM_RANGE_GRAB:
	case AM_POWER_GRAB:
	case AM_SPEED_GRAB:		
		return input.GetVHeldTime(AB_GRAB);

	case AM_ACTION:			
		return input.GetVHeldTime(AB_ACTION);

	case AM_DODGE_LEFT:		
		return input.GetVHeldTime(AB_DODGE_LEFT);

	case AM_DODGE_RIGHT:	
		return input.GetVHeldTime(AB_DODGE_RIGHT);

	case AM_DODGE_FORWARD:	
		return input.GetVHeldTime(AB_DODGE_FORWARD);

	case AM_DODGE_BACK:		
		return input.GetVHeldTime(AB_DODGE_BACK);

	case AM_NONE:
	default:			
		return 0.f;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::Reset
*
*	DESCRIPTION		Return to the beginning of the attack structure
*
***************************************************************************************************/
void CAttackTracker::Reset()
{
	//ntPrintf("AttackTracker Reset()\n");

	// Clear our information
	m_obCurrentAttack.Reset();
	m_obRequestedAttack.Reset();

	// If we are tracking user input...
	if ( !m_bDirectMode )
	{
		// Set the current attack to point at the starting point
		m_obCurrentAttack.pobAttackLink = m_pobClusterStructure->m_pobLeadCluster;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::GetRequestedAttackClass
*
*	DESCRIPTION		Allows the combat component access to what has been requested - in some cases
*					behaviour may depend on the type of attack that the player wishes to do next.
*
***************************************************************************************************/
ATTACK_CLASS CAttackTracker::GetRequestedAttackClass( void )
{
	// Make sure there is a requested attack - user should switch on this first
	ntAssert( m_obRequestedAttack.IsComplete() );

	// Returnt the attack class of the default attack belonging to the header
	return m_obRequestedAttack.pobAttackData->m_eAttackClass;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::GetRequestedAttackType
*
*	DESCRIPTION		Get the attack type requested based on the current pad state
*
*					This implementation is hardly optimal but it makes it clear which distinct
*					attacks are available in each stance.  Nice and readable.
*
***************************************************************************************************/
ATTACK_MOVE_TYPE CAttackTracker::GetRequestedAttackType( const CInputComponent* pobInput, STANCE_TYPE eStance ) const
{
	// If we are here in direct mode or have no input we have no request types
	if ( ( m_bDirectMode ) || ( !pobInput ) )
		return AM_NONE;

	// Get the current state of the input component - the stances are held not pressed
	u_int uiPressed = pobInput->GetVPressed();

	// Create a return value
	ATTACK_MOVE_TYPE eAttackType = AM_NONE;

	#define MASH_PER_SECOND_THRESHOLD 4

	// Power stance attacks
	if ( eStance == ST_POWER )
	{
		if	( uiPressed & ( 1 << AB_ATTACK_FAST ) )	
		{
			if (pobInput->GetVButtonMashesInTheLastSecond(AB_ATTACK_FAST) > MASH_PER_SECOND_THRESHOLD)
				eAttackType = AM_MASH_POWER_FAST;
			else
				eAttackType = AM_POWER_FAST;
		}
		else if	( uiPressed & ( 1 << AB_ATTACK_MEDIUM ) )	
		{
			if (pobInput->GetVButtonMashesInTheLastSecond(AB_ATTACK_MEDIUM) > MASH_PER_SECOND_THRESHOLD)
				eAttackType = AM_MASH_POWER_MEDIUM;
			else
				eAttackType = AM_POWER_MEDIUM;
		}
		else if	( uiPressed & ( 1 << AB_GRAB ) )			
		{
			if (pobInput->GetVButtonMashesInTheLastSecond(AB_GRAB) > MASH_PER_SECOND_THRESHOLD)
				eAttackType = AM_MASH_POWER_GRAB;
			else
				eAttackType = AM_POWER_GRAB;
		}
	}

	// Range stance attacks
	else if ( eStance == ST_RANGE )
	{
		if	( uiPressed & ( 1 << AB_ATTACK_FAST ) )	
		{
			if (pobInput->GetVButtonMashesInTheLastSecond(AB_ATTACK_FAST) > MASH_PER_SECOND_THRESHOLD)
				eAttackType = AM_MASH_RANGE_FAST;
			else
				eAttackType = AM_RANGE_FAST;
		}
		else if	( uiPressed & ( 1 << AB_ATTACK_MEDIUM ) )	
		{
			if (pobInput->GetVButtonMashesInTheLastSecond(AB_ATTACK_MEDIUM) > MASH_PER_SECOND_THRESHOLD)
				eAttackType = AM_MASH_RANGE_MEDIUM;
			else
				eAttackType = AM_RANGE_MEDIUM;
		}
		else if	( uiPressed & ( 1 << AB_GRAB ) )			
		{
			if (pobInput->GetVButtonMashesInTheLastSecond(AB_GRAB) > MASH_PER_SECOND_THRESHOLD)
				eAttackType = AM_MASH_RANGE_GRAB;
			else
				eAttackType = AM_RANGE_GRAB;
		}
	}
	
	// Technique stance attacks
	else if ( eStance == ST_SPEED )
	{
		if	( uiPressed & ( 1 << AB_ATTACK_FAST ) )	
		{
			if (pobInput->GetVButtonMashesInTheLastSecond(AB_ATTACK_FAST) > MASH_PER_SECOND_THRESHOLD)
				eAttackType = AM_MASH_SPEED_FAST;
			else
				eAttackType = AM_SPEED_FAST;
		}
		else if	( uiPressed & ( 1 << AB_ATTACK_MEDIUM ) )	
		{
			if (pobInput->GetVButtonMashesInTheLastSecond(AB_ATTACK_MEDIUM) > MASH_PER_SECOND_THRESHOLD)
				eAttackType = AM_MASH_SPEED_MEDIUM;
			else
				eAttackType = AM_SPEED_MEDIUM;
		}
		else if	( uiPressed & ( 1 << AB_GRAB ) )			
		{
			if (pobInput->GetVButtonMashesInTheLastSecond(AB_GRAB) > MASH_PER_SECOND_THRESHOLD)
				eAttackType = AM_MASH_SPEED_GRAB;
			else
				eAttackType = AM_SPEED_GRAB;
		}		
	}

	// Check non stance specifics
	if	( uiPressed & ( 1 << AB_ACTION ) )	
		eAttackType = AM_ACTION;
	CHECK_EVADES
	else if	( uiPressed & ( 1 << AB_DODGE_LEFT ) )		
		eAttackType = AM_DODGE_LEFT;
	else if	( uiPressed & ( 1 << AB_DODGE_RIGHT ) )		
		eAttackType = AM_DODGE_RIGHT;
	else if	( uiPressed & ( 1 << AB_DODGE_FORWARD ) )	
		eAttackType = AM_DODGE_FORWARD;
	else if	( uiPressed & ( 1 << AB_DODGE_BACK ) )		
		eAttackType = AM_DODGE_BACK;
	CHECK_EVADES_END

	// Give it to them
	return eAttackType;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::GetStyleLevelLink
*
*	DESCRIPTION		Get attack from style level cluster
*
***************************************************************************************************/
const CAttackLink* CAttackTracker::GetStyleLevelLink( ATTACK_MOVE_TYPE eAttackType, HIT_LEVEL eHitLevel, bool bOnTheSpotEquivalent )
{
	ntAssert(m_pobClusterStructure); // We have got to have one of these
	ntAssert(m_pobClusterStructure->m_pobLeadCluster); // We must have something to fall back onto

	switch ( eHitLevel )
	{
	case HL_SPECIAL:	
		{
			if (m_pobClusterStructure->m_pobStyleLevelSpecialCluster)
				return m_pobClusterStructure->m_pobStyleLevelSpecialCluster->GetNextAttackNode( eAttackType );
			else
				return m_pobClusterStructure->m_pobLeadCluster->GetNextAttackNode( eAttackType );
		break;
		}

	case HL_FOUR:	
		{
			// Should have started special behaviour here so no need to do anything
			return 0;
		break;
		}
	case HL_THREE:	
		{
			int iNumAttacks = -1;
			CClusterStructure::SuperStyleLevelLinkList::const_iterator obIt, obEnd;
			
			if (bOnTheSpotEquivalent)
			{
				obIt = m_pobClusterStructure->m_aobOnTheSpotStyleLevelLinks[2].begin();
				obEnd = m_pobClusterStructure->m_aobOnTheSpotStyleLevelLinks[2].end();
				iNumAttacks = m_pobClusterStructure->m_aobOnTheSpotStyleLevelLinks[2].size();
			}
			else
			{
				obIt = m_pobClusterStructure->m_aobSuperStyleLevelLinks[2].begin();
				obEnd = m_pobClusterStructure->m_aobSuperStyleLevelLinks[2].end();
				iNumAttacks = m_pobClusterStructure->m_aobSuperStyleLevelLinks[2].size();
			}

			if (iNumAttacks > 0)
			{
				int iIndex = 0;
				int iRandomIndex = rand()%iNumAttacks;

				if (iNumAttacks>1)
				{
				while (iRandomIndex == m_iLastRandomStyleIndex)
					iRandomIndex = rand()%iNumAttacks;
				}

				m_iLastRandomStyleIndex = iRandomIndex;
				while (obIt != obEnd)
				{
					if (iIndex == iRandomIndex)
					{
						return (*obIt)->GetNextAttackNode( eAttackType );
					}

					++iIndex;
					obIt++;
				}
			}
			else
				return 0;
		break;
		}
	case HL_TWO:	
		{
			int iNumAttacks = -1;
			CClusterStructure::SuperStyleLevelLinkList::const_iterator obIt, obEnd;
			if (bOnTheSpotEquivalent)
			{
				obIt = m_pobClusterStructure->m_aobOnTheSpotStyleLevelLinks[1].begin();
				obEnd = m_pobClusterStructure->m_aobOnTheSpotStyleLevelLinks[1].end();
				iNumAttacks = m_pobClusterStructure->m_aobOnTheSpotStyleLevelLinks[1].size();
			}
			else
			{
				obIt = m_pobClusterStructure->m_aobSuperStyleLevelLinks[1].begin();
				obEnd = m_pobClusterStructure->m_aobSuperStyleLevelLinks[1].end();
				iNumAttacks = m_pobClusterStructure->m_aobSuperStyleLevelLinks[1].size();
			}

			if (iNumAttacks > 0)
			{
				int iIndex = 0;
				int iRandomIndex = rand()%iNumAttacks;

				if (iNumAttacks>1)
				{
				while (iRandomIndex == m_iLastRandomStyleIndex)
					iRandomIndex = rand()%iNumAttacks;
				}

				m_iLastRandomStyleIndex = iRandomIndex;
				while (obIt != obEnd)
				{
					if (iIndex == iRandomIndex)
					{
						return (*obIt)->GetNextAttackNode( eAttackType );
					}

					++iIndex;
					obIt++;
				}
			}
			else
				return 0;
		break;
		}
	case HL_ONE:	
		{
			int iNumAttacks = -1;
			CClusterStructure::SuperStyleLevelLinkList::const_iterator obIt, obEnd;
			if (bOnTheSpotEquivalent)
			{
				obIt = m_pobClusterStructure->m_aobOnTheSpotStyleLevelLinks[0].begin();
				obEnd = m_pobClusterStructure->m_aobOnTheSpotStyleLevelLinks[0].end();
				iNumAttacks = m_pobClusterStructure->m_aobOnTheSpotStyleLevelLinks[0].size();
			}
			else
			{
				obIt = m_pobClusterStructure->m_aobSuperStyleLevelLinks[0].begin();
				obEnd = m_pobClusterStructure->m_aobSuperStyleLevelLinks[0].end();
				iNumAttacks = m_pobClusterStructure->m_aobSuperStyleLevelLinks[0].size();
			}

			if (iNumAttacks > 0)
			{
				int iIndex = 0;
				int iRandomIndex = rand()%iNumAttacks;

				if (iNumAttacks>1)
				{
				while (iRandomIndex == m_iLastRandomStyleIndex)
					iRandomIndex = rand()%iNumAttacks;
				}

				m_iLastRandomStyleIndex = iRandomIndex;
				while (obIt != obEnd)
				{
					if (iIndex == iRandomIndex)
					{
						return (*obIt)->GetNextAttackNode( eAttackType );
					}

					++iIndex;
					obIt++;
				}
			}
			else
				return 0;
		break;
		}
	case HL_ZERO:	
		return m_pobClusterStructure->m_pobLeadCluster->GetNextAttackNode( eAttackType );
		break;

	default:		
		ntAssert( 0 );	
		break;
	}

	// If we are here things are a little bonkers
	return 0;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectStartAttack
*
*	DESCRIPTION		Selects the first attack in a string - currently this is doing all the button
*					press monitoring.  Maybe this should be removed outside the combat system.
*
***************************************************************************************************/
bool CAttackTracker::SelectStartAttack( CEntity* pobEntity, STANCE_TYPE eStance, HIT_LEVEL eHitLevel, bool bOnTheSpotEquivalent,
									   ATTACK_EVADE_TYPE ePossibleEvadeType, ATTACK_EVADE_SECTOR ePossibleEvadeSector)
{
	// If in direct mode indicate whether a request is present
	if ( m_bDirectMode )
		return m_obRequestedAttack.IsComplete();

	// Check the input for requests - this needs to change for remote players - JML
	ATTACK_MOVE_TYPE eAttackType;
	if ( pobEntity->GetInputComponent() )
        eAttackType = GetRequestedAttackType( pobEntity->GetInputComponent(), eStance );
	else
		eAttackType = AM_NONE;

	// Check that we actually have an input
	if ( eAttackType != AM_NONE )
	{
		// Reset our data
		Reset();
	
		// Get something from out lead style clusters
		m_obRequestedAttack.pobAttackLink = GetStyleLevelLink( eAttackType, eHitLevel, bOnTheSpotEquivalent );

		// If that fails, see if we can get an evade
		if (!m_obRequestedAttack.pobAttackLink)
			m_obRequestedAttack.pobAttackLink = GetEvade( eAttackType, ePossibleEvadeType, ePossibleEvadeSector );

		// If that didn't return anything then get what we were initially intended to get
		if ( !m_obRequestedAttack.pobAttackLink )
			m_obRequestedAttack.pobAttackLink = m_obCurrentAttack.pobAttackLink->GetNextAttackNode( eAttackType );

		if ( m_obRequestedAttack.pobAttackLink && m_obRequestedAttack.pobAttackLink->GetAttackDataP() )
		{
			// Get the data
			m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();
			m_obRequestedAttack.eRequestType = eAttackType;		

			if (m_obRequestedAttack.pobAttackLink->m_pobButtonHeldAttack)
			{
				m_obRequestedButtonHeldAttack.pobAttackLink = m_obRequestedAttack.pobAttackLink->m_pobButtonHeldAttack;
				m_obRequestedButtonHeldAttack.pobAttackData = m_obRequestedButtonHeldAttack.pobAttackLink->GetAttackDataP();
				m_obRequestedButtonHeldAttack.eRequestType = eAttackType;	
			}

			//ntPrintf("AttackTracker SelectStartAttack() %s\n", ObjectDatabase::Get().GetNameFromPointer(m_obRequestedAttack.pobAttackLink) );

			return true;
		}
	}

	// We failed, check if it's a MASH type, then see if the equivalent non-mash type had a move
	if ( eAttackType == AM_MASH_SPEED_FAST )
		eAttackType = AM_SPEED_FAST;
	else if ( eAttackType == AM_MASH_SPEED_MEDIUM )
		eAttackType = AM_SPEED_MEDIUM;
	else if ( eAttackType == AM_MASH_RANGE_FAST ) 
		eAttackType = AM_RANGE_FAST;
	else if ( eAttackType == AM_MASH_RANGE_MEDIUM )
		eAttackType = AM_RANGE_MEDIUM;
	else if ( eAttackType == AM_MASH_POWER_FAST )
		eAttackType = AM_POWER_FAST;
	else if ( eAttackType == AM_MASH_POWER_MEDIUM )
		eAttackType = AM_POWER_MEDIUM;
	else if ( eAttackType == AM_MASH_POWER_MEDIUM )
		eAttackType = AM_POWER_MEDIUM;
	else if ( eAttackType == AM_MASH_ACTION )
		eAttackType = AM_ACTION;
	else 
		eAttackType = AM_NONE;

	// Try again!
    if ( eAttackType != AM_NONE )
	{
		// Reset our data
		Reset();
	
		// Get something from out lead style clusters
		m_obRequestedAttack.pobAttackLink = GetStyleLevelLink( eAttackType, eHitLevel, bOnTheSpotEquivalent );

		// If that fails, see if we can get an evade
		if (!m_obRequestedAttack.pobAttackLink)
			m_obRequestedAttack.pobAttackLink = GetEvade( eAttackType );			

		// If that didn't return anything then get what we were initially intended to get
		if ( !m_obRequestedAttack.pobAttackLink )
			m_obRequestedAttack.pobAttackLink = m_obCurrentAttack.pobAttackLink->GetNextAttackNode( eAttackType );

		if ( m_obRequestedAttack.pobAttackLink && m_obRequestedAttack.pobAttackLink->GetAttackDataP() )
		{
			// Get the data
			m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();
			m_obRequestedAttack.eRequestType = eAttackType;		

			if (m_obRequestedAttack.pobAttackLink->m_pobButtonHeldAttack)
			{
				m_obRequestedButtonHeldAttack.pobAttackLink = m_obRequestedAttack.pobAttackLink->m_pobButtonHeldAttack;
				m_obRequestedButtonHeldAttack.pobAttackData = m_obRequestedButtonHeldAttack.pobAttackLink->GetAttackDataP();
				m_obRequestedButtonHeldAttack.eRequestType = eAttackType;	
			}

			//ntPrintf("AttackTracker SelectStartAttack() %s\n", ObjectDatabase::Get().GetNameFromPointer(m_obRequestedAttack.pobAttackLink) );

			return true;
		}
	}

	//ntPrintf("AttackTracker SelectStartAttack() FAIL\n");

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::GetEvade
*
*	DESCRIPTION		Select an evade, based on whether it is targeted or not
*
***************************************************************************************************/
const CAttackLink* CAttackTracker::GetEvade( ATTACK_MOVE_TYPE eAttackType, ATTACK_EVADE_TYPE eEvadeType, ATTACK_EVADE_SECTOR eEvadeSector)
{
	//Make sure we have evade data available, otherwise return 0.
	if(eEvadeType == AET_STANDARD && (!m_pobClusterStructure->m_pobOpenFreeEvades || !m_pobClusterStructure->m_pobComboFreeEvades ))
	{
		return 0;
	}
	else if(eEvadeType == AET_SKILLEVADE && !m_pobClusterStructure->m_pobOpenSkillEvades)
	{
		return 0;
	}
	else if(eEvadeType == AET_SKILLEVADE_CLOSE && !m_pobClusterStructure->m_pobCloseSkillEvades)
	{
		return 0;
	}

	if(eEvadeType == AET_STANDARD)
	{
		if ( m_obCurrentAttack.IsComplete() &&
			m_obCurrentAttack.pobAttackData->m_eAttackClass != AC_GRAB_GOTO &&
			m_obCurrentAttack.pobAttackData->m_eAttackClass != AC_GRAB_HOLD &&
			m_obCurrentAttack.pobAttackData->m_eAttackClass != AC_GRAB_STRIKE &&
			m_obCurrentAttack.pobAttackData->m_eAttackClass != AC_EVADE )
		{
			return m_pobClusterStructure->m_pobComboFreeEvades->GetNextAttackNode( eAttackType );
		}
		else
		{
			return m_pobClusterStructure->m_pobOpenFreeEvades->GetNextAttackNode( eAttackType );
		}
	}
	else if(eEvadeType == AET_SKILLEVADE)
	{
		return m_pobClusterStructure->m_pobOpenSkillEvades->GetRandomSkillEvade( eEvadeSector );
	}
	else if(eEvadeType == AET_SKILLEVADE_CLOSE)
	{
		return m_pobClusterStructure->m_pobCloseSkillEvades->GetRandomSkillEvade( eEvadeSector );
	}
	else if(eEvadeType == AET_SUPERSKILLEVADE)
	{
		return m_pobClusterStructure->m_pobOpenSuperSkillEvades->GetRandomSkillEvade( eEvadeSector );
	}
	else if(eEvadeType == AET_SUPERSKILLEVADE_CLOSE)
	{
		return m_pobClusterStructure->m_pobCloseSuperSkillEvades->GetRandomSkillEvade( eEvadeSector );
	}
	else
	{
		ntError_p(false, ("Invalid ATTACK_EVADE_TYPE passed-in to eAttackType"));
		return 0;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectBlockedGrab
*
*	DESCRIPTION		If it's available, select the blocked grab attack
*
***************************************************************************************************/
bool CAttackTracker::SelectBlockedGrab()
{
	// This should only be called if we're already trying a grab
	ntAssert( m_obCurrentAttack.pobAttackData->m_eAttackClass == AC_GRAB_GOTO ||
			m_obCurrentAttack.pobAttackData->m_eAttackClass == AC_GRAB_HOLD ||
			m_obCurrentAttack.pobAttackData->m_eAttackClass == AC_GRAB_STRIKE );

	if (m_pobClusterStructure->m_pobBlockedGrab)
	{
		m_obRequestedAttack.pobAttackLink = m_pobClusterStructure->m_pobBlockedGrab;
		m_obRequestedAttack.pobAttackData = m_pobClusterStructure->m_pobBlockedGrab->GetAttackDataP();
		// Inherit move type from the grab that's failing		
		m_obRequestedAttack.eRequestType = m_obCurrentAttack.eRequestType;

		//ntPrintf("AttackTracker SelectBlockedGrab() %s\n", ObjectDatabase::Get().GetNameFromPointer(m_obRequestedAttack.pobAttackData));

		return true;
	}

	//ntPrintf("AttackTracker SelectBlockedGrab() FAIL\n");

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectGroundAttack
*
*	DESCRIPTION		If any are available, try to select a ground attack
*
***************************************************************************************************/
bool CAttackTracker::SelectGroundAttack( CEntity* pobEntity, STANCE_TYPE eStance, HIT_LEVEL eHitLevel, REACTION_ZONE eZone )
{
	UNUSED( eHitLevel );

	// Check the input for requests - this needs to change for remote players - JML
	ATTACK_MOVE_TYPE eAttackType;
	if ( pobEntity->GetInputComponent() )
        eAttackType = GetRequestedAttackType( pobEntity->GetInputComponent(), eStance );
	else
		eAttackType = AM_NONE;

	// Check that we actually have an input and a ground cluster
	if ( eAttackType != AM_NONE )
	{
		if (eZone == RZ_FRONT && m_pobClusterStructure->m_pobGroundClusterFront)
		{
			// Set up a temporary attack link
			const CAttackLink* pobTempAttackLink = 0;
		
			// Find something in our ground attack cluster
			pobTempAttackLink = m_pobClusterStructure->m_pobGroundClusterFront->GetNextAttackNode(eAttackType);

			if ( pobTempAttackLink && pobTempAttackLink->GetAttackDataP() )
			{
				// Grab the data
				m_obRequestedAttack.pobAttackLink = pobTempAttackLink;
				m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();

				// Store the attack type that this was requested through
				m_obRequestedAttack.eRequestType = eAttackType;

				//ntPrintf("AttackTracker SelectGroundAttack() %s\n", ObjectDatabase::Get().GetNameFromPointer(m_obRequestedAttack.pobAttackData));

				return true;
			}
		} 
		else if (eZone == RZ_BACK && m_pobClusterStructure->m_pobGroundClusterBack)
		{
			// Set up a temporary attack link
			const CAttackLink* pobTempAttackLink = 0;
		
			// Find something in our ground attack cluster
			pobTempAttackLink = m_pobClusterStructure->m_pobGroundClusterBack->GetNextAttackNode(eAttackType);

			if ( pobTempAttackLink && pobTempAttackLink->GetAttackDataP() )
			{
				// Grab the data
				m_obRequestedAttack.pobAttackLink = pobTempAttackLink;
				m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();

				// Store the attack type that this was requested through
				m_obRequestedAttack.eRequestType = eAttackType;

				//ntPrintf("AttackTracker SelectGroundAttack() %s\n", ObjectDatabase::Get().GetNameFromPointer(m_obRequestedAttack.pobAttackData));

				return true;
			}
		}
	}

	//ntPrintf("AttackTracker SelectGroundAttack() FAIL\n");

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectInitialAttackEquivalent
*
*	DESCRIPTION		Leaves the existing requested attack in place if no equivalent is found
*
***************************************************************************************************/
bool CAttackTracker::SelectInitialAttackEquivalent( void )
{
	// Make sure we have enough current data
	ntAssert( m_obRequestedAttack.eRequestType != AM_NONE );

	// Make sure we are safe here
	ATTACK_MOVE_TYPE eMoveType = m_obRequestedAttack.eRequestType;

	// To keep this safe lets make sure we can do this
	if ( ( eMoveType == AM_NONE ) || ( eMoveType == AM_ACTION ) )
	{
		// But give some debug so we know something's not right
		user_warn_p(false,("SelectInitialAttackEquivalent FAILED. Replacing with a range fast. Has an autolink gone wrong?\n"));
		eMoveType = AM_SPEED_FAST;
	}

	// Set up a temporary attack link
	const CAttackLink* pobTempAttackLink = 0;

	// Get an attack from the lead cluster of the same type
	pobTempAttackLink = m_pobClusterStructure->m_pobLeadCluster->GetNextAttackNode( eMoveType );

	// If the temp link is suitable...
	if ( pobTempAttackLink && pobTempAttackLink->GetAttackDataP() )
	{
		// Copy the all the details into our requested attack slot
		m_obRequestedAttack.pobAttackLink = pobTempAttackLink;

		// Grab the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();

		// Store the attack type that this was requested through
		m_obRequestedAttack.eRequestType = eMoveType;
		return true;
	}

	// If we are here then we have failed
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectNextAttackEquivalent
*
*	DESCRIPTION		This should always find an equivalent.  If it doesn't then it is probably being
*					called in the wrong circumstance.  This needs to be called if we have looked for
*					to-lockon equivalents but failed to find a target - then we may need to return
*					to the initial attack
*
***************************************************************************************************/
bool CAttackTracker::SelectNextAttackEquivalent( void )
{
	// Make sure we have enough current data
	ntAssert( m_obRequestedAttack.eRequestType != AM_NONE );

	// Set up a temporary attack link
	const CAttackLink* pobTempAttackLink = 0;

	// Get an attack from the lead cluster of the same type
	pobTempAttackLink = m_obCurrentAttack.pobAttackLink->GetNextAttackNode( m_obRequestedAttack.eRequestType );

	// If the temp link is suitable...
	if ( pobTempAttackLink && pobTempAttackLink->GetAttackDataP() )
	{
		// Copy the all the details into our requested attack slot
		m_obRequestedAttack.pobAttackLink = pobTempAttackLink;

		// Grab the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();

		// Store the attack type that this was requested through
		m_obRequestedAttack.eRequestType = m_obRequestedAttack.eRequestType;
		return true;
	}

	// We shouldn't ever been here
	ntAssert( 0 );

	// If we are here then we have failed
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectMediumRangeEquivalent
*
*	DESCRIPTION		Leaves the existing requested attack in place if no equivalent is found
*
***************************************************************************************************/
bool CAttackTracker::SelectMediumRangeEquivalent( void )
{
	// Make sure we have enough current data
	if( m_obRequestedAttack.eRequestType == AM_NONE )
		return false;
	
	// Try and get an attack of the same current request type from the medium range cluster
	const CAttackLink* pobTempAttackLink = m_pobClusterStructure->m_pobMediumRangeCluster->GetNextAttackNode( m_obRequestedAttack.eRequestType );

	// If the temp link is suitable...
	if ( pobTempAttackLink && pobTempAttackLink->GetAttackDataP() )
	{
		// Copy the all the details into our requested attack slot
		m_obRequestedAttack.pobAttackLink = pobTempAttackLink;

		// Grab the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();

		// Store the attack type that this was requested through
		m_obRequestedAttack.eRequestType = m_obRequestedAttack.eRequestType;
		return true;
	}

	// If we are here then we have failed
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectLongRangeEquivalent
*
*	DESCRIPTION		Leaves the existing requested attack in place if no equivalent is found
*
***************************************************************************************************/
bool CAttackTracker::SelectLongRangeEquivalent( void )
{
	// Make sure we have enough current data
	ntAssert( m_obRequestedAttack.eRequestType != AM_NONE );

	// Try and get an attack of the same current request type from the long range cluster
	const CAttackLink* pobTempAttackLink = m_pobClusterStructure->m_pobLongRangeCluster->GetNextAttackNode( m_obRequestedAttack.eRequestType );
	
	// If the temp link is suitable...
	if ( pobTempAttackLink && pobTempAttackLink->GetAttackDataP() )
	{
		// Copy the all the details into our requested attack slot
		m_obRequestedAttack.pobAttackLink = pobTempAttackLink;

		// Grab the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();

		// Store the attack type that this was requested through
		m_obRequestedAttack.eRequestType = m_obRequestedAttack.eRequestType;
		return true;
	}

	// If we are here then we have failed
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectInterceptEquivalent
*
*	DESCRIPTION		If we end up targeting some body that is already attacking us - and in their
*					'intercept' window before the strike, we may want to switch to a special attack
*					that gives us some more specialised functionality
*
*					Leaves the existing requested attack in place if no equivalent is found
*
***************************************************************************************************/
bool CAttackTracker::SelectInterceptEquivalent( HIT_LEVEL eHitLevel )
{
	UNUSED( eHitLevel );

	// Make sure we have enough current data
	//ntAssert( m_obRequestedAttack.eRequestType != AM_NONE );
	// Can't see any reason that this would be an error
	if (m_obRequestedAttack.eRequestType == AM_NONE)
		return false;

	// Make sure that we have the structure from which to select an alternative attack
	if ( !m_pobClusterStructure->m_pobInterceptCluster )
		return false;

		// Set up a temporary attack link
	const CAttackLink* pobTempAttackLink = 0;

	// If that didn't return anything then get what we were initially intended to get
	if ( !pobTempAttackLink )
		pobTempAttackLink = m_pobClusterStructure->m_pobInterceptCluster->GetNextAttackNode( m_obRequestedAttack.eRequestType );

	// If the temp link is suitable...
	if ( pobTempAttackLink && pobTempAttackLink->GetAttackDataP() )
	{
		// Copy the all the details into our requested attack slot
		m_obRequestedAttack.pobAttackLink = pobTempAttackLink;

		// Grab the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();

		// Store the attack type that this was requested through
		m_obRequestedAttack.eRequestType = m_obRequestedAttack.eRequestType;

		return true;
	}

	// If we are here then we have failed
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::AutoSelect
*
*	DESCRIPTION		This will automatically select an attack from the given link.  Its not very 
*					clever so it needs to be easy.  Hopefully there will only be one attack - if 
*					there is more than one in total then there should only be one for the current
*					stance.  If there is more than one for the current stance then it will just
*					pick the first from the top of the list and be done with it.
*
***************************************************************************************************/
ATTACK_MOVE_TYPE CAttackTracker::AutoSelect( const CAttackLink* pobFromLink, STANCE_TYPE eStanceType )
{
	UNUSED( pobFromLink );
	UNUSED( eStanceType );

	// No more messing around looking for one, if an autolink is set up, it'll be on action
	return AM_ACTION;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectNextAttack
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackTracker::SelectNextAttack( CEntity* pobEntity, STANCE_TYPE eStance, bool bAutomatic, HIT_LEVEL eHitLevel, bool bOnTheSpotEquivalent )
{
	// If in direct mode indicate whether a request is present
	if ( m_bDirectMode && !bAutomatic )
		return m_obRequestedAttack.IsComplete();

	// We need to find the next attack in the string from the current attack
	ATTACK_MOVE_TYPE eMoveType = AM_NONE;
	
	// If this is an automatic thing then we choose from the next attack link ourselves
	if ( bAutomatic )
		eMoveType = AutoSelect( m_obCurrentAttack.pobAttackLink, eStance );
	else
		eMoveType = GetRequestedAttackType( pobEntity->GetInputComponent(), eStance );

	bool bRet = false;
	if ( eMoveType != AM_NONE )
		bRet = SelectNewAttack( m_obCurrentAttack.pobAttackLink, eMoveType, eHitLevel, bOnTheSpotEquivalent );

	if (!bRet)
	{
		// We failed, check if it's a MASH type, then see if the equivalent non-mash type had a move
		if ( eMoveType == AM_MASH_SPEED_FAST )
			eMoveType = AM_SPEED_FAST;
		if ( eMoveType == AM_MASH_SPEED_MEDIUM )
			eMoveType = AM_SPEED_MEDIUM;
		if ( eMoveType == AM_MASH_RANGE_FAST ) 
			eMoveType = AM_RANGE_FAST;
		if ( eMoveType == AM_MASH_RANGE_MEDIUM )
			eMoveType = AM_RANGE_MEDIUM;
		if ( eMoveType == AM_MASH_POWER_FAST )
			eMoveType = AM_POWER_FAST;
		if ( eMoveType == AM_MASH_POWER_MEDIUM )
			eMoveType = AM_POWER_MEDIUM;
		if ( eMoveType == AM_MASH_POWER_MEDIUM )
			eMoveType = AM_POWER_MEDIUM;
		if ( eMoveType == AM_MASH_ACTION )
			eMoveType = AM_ACTION;

		// Try again!
		if ( eMoveType != AM_NONE )
			bRet = SelectNewAttack( m_obCurrentAttack.pobAttackLink, eMoveType, eHitLevel, bOnTheSpotEquivalent );
	}

	if (!bRet)
		ClearRequested();
	return bRet;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectRisingAttack
*
*	DESCRIPTION		Selects from our rising attacks
*
***************************************************************************************************/
bool CAttackTracker::SelectRisingAttack( CEntity* pobEntity, STANCE_TYPE eStance, bool& bRequestMade, HIT_LEVEL eHitLevel )
{
	// If in direct mode indicate whether a request is present
	if ( m_bDirectMode )
		return m_obRequestedAttack.IsComplete();

	// Get the next attack based on popping up to the next cluster level
	ATTACK_MOVE_TYPE eMoveType = GetRequestedAttackType( pobEntity->GetInputComponent(), eStance );
	if ( eMoveType != AM_NONE )
	{
		// Flag that a request was made
		bRequestMade = true;
		return SelectNewAttack( m_pobClusterStructure->m_pobRisingCluster, eMoveType, eHitLevel, false );
	}

	// Otherwise make sure that the requested attack data is clear
	m_obRequestedAttack.Reset();
	
	// If we are here we have no attack
	bRequestMade = false;
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectSpecificCounterAttack
*
*	DESCRIPTION		Tries to find a specific counter attack in the cluster, and if found returns 
*					true and uses it. Otherwise returns false.
*
***************************************************************************************************/
bool CAttackTracker::SelectSpecificCounterAttack( CEntity* pobEntity, STANCE_TYPE eStance, SpecificCounterIndex* pobSpecificCounterIndex, bool bAutoCounter )
{
	//ntPrintf("AttackTracker SelectSpecificCounterAttack() %s\n", *obSpecificCounterName);

	if (!pobSpecificCounterIndex || pobSpecificCounterIndex->GetNumberOfEntries() == 0 || !m_pobClusterStructure->m_pobSpecificCounterCollection )
		return false;

	int iRand = -1;
	SpecificCounterIndexEntry* pobSpecificCounterIndexEntry = 0;
	if (pobSpecificCounterIndex->GetNumberOfEntries() == 1)
	{	
		pobSpecificCounterIndexEntry = pobSpecificCounterIndex->GetEntryAt(0);
		CAttackLink* pobSpecificCounterLink = m_pobClusterStructure->m_pobSpecificCounterCollection->GetSpecificCounter(pobSpecificCounterIndexEntry->GetSpecificCounterAttackName());
		ATTACK_MOVE_TYPE eMoveType = GetRequestedAttackType( pobEntity->GetInputComponent(), eStance );
		if(pobSpecificCounterLink && (eMoveType != AM_NONE))
		{
			return SelectNewAttack( pobSpecificCounterLink, eMoveType, HL_ZERO, false );
		}
	}
	else
	{
		// Find a rand we've not had last time the collection was queried
		iRand = grand() % pobSpecificCounterIndex->GetNumberOfEntries();
		if (iRand == pobSpecificCounterIndex->GetLastUsedIndex())
		{
			iRand = (iRand + 1) % pobSpecificCounterIndex->GetNumberOfEntries();
		}

		pobSpecificCounterIndexEntry = pobSpecificCounterIndex->GetEntryAt(iRand);	

		// If we don't have a specific counter for this index entry, then to be sure we should try ALL other index entries 
		// Just to be sure that we definately have no way to select a counter
		int iNumTried = 1;
		CAttackLink* pobSpecificCounterLink = m_pobClusterStructure->m_pobSpecificCounterCollection->GetSpecificCounter(pobSpecificCounterIndexEntry->GetSpecificCounterAttackName());
		while (!pobSpecificCounterLink
				&&
				iNumTried < pobSpecificCounterIndex->GetNumberOfEntries())
		{
			iRand = (iRand + 1) % pobSpecificCounterIndex->GetNumberOfEntries();
			pobSpecificCounterIndexEntry = pobSpecificCounterIndex->GetEntryAt(iRand);
			pobSpecificCounterLink = m_pobClusterStructure->m_pobSpecificCounterCollection->GetSpecificCounter(pobSpecificCounterIndexEntry->GetSpecificCounterAttackName());
			iNumTried++;
		}

		if (pobSpecificCounterLink)
		{
			pobSpecificCounterIndex->SetLastUsedIndex(iRand);

			ATTACK_MOVE_TYPE eMoveType = GetRequestedAttackType( pobEntity->GetInputComponent(), eStance );
			
			// If we're auto countering, flick through until we find something on the link
			if (bAutoCounter)
			{
				int iMove = 0;
				while (!pobSpecificCounterLink->GetNextAttackNode((ATTACK_MOVE_TYPE)iMove) || iMove > AM_NONE-1)
				{
					iMove++;	
				}
				if ((ATTACK_MOVE_TYPE)iMove != AM_NONE)
					eMoveType = (ATTACK_MOVE_TYPE)iMove;
			} // If we haven't found anything, eMoveType is unchanged and nothing will go mental
			
			if ( eMoveType != AM_NONE ) 
			{			
				// With specific counters we can assume hit level doesn't matter, cos we're either gonna select the counter or get hit
				// Just select next attack from the link
				pobSpecificCounterIndex->SetLastUsedIndex(iRand);
				return SelectNewAttack( pobSpecificCounterLink, eMoveType, HL_ZERO, false );
				
				//ntPrintf("AttackTracker SelectSpecificCounterAttack() SUCCESS %s\n", ObjectDatabase::Get().GetNameFromPointer(m_obRequestedAttack.pobAttackData));
			}
			else
			{
				//ntPrintf("AttackTracker SelectSpecificCounterAttack() FAIL\n");

				m_obRequestedAttack.Reset();
				return false;
			}
		}
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectInstantKORecoverAttack
*
*	DESCRIPTION		Get an attack from the IKORA cluster and request it if there's one there.
*
***************************************************************************************************/
bool CAttackTracker::SelectInstantKORecoverAttack( CEntity* pobEntity, STANCE_TYPE eStance, HIT_LEVEL eHitLevel )
{
	UNUSED( eHitLevel );

	// Check the input for requests - this needs to change for remote players - JML
	ATTACK_MOVE_TYPE eAttackType;
	if ( pobEntity->GetInputComponent() )
        eAttackType = GetRequestedAttackType( pobEntity->GetInputComponent(), eStance );
	else
		eAttackType = AM_NONE;

	// Check that we actually have an input and a IKORA cluster
	if ( eAttackType != AM_NONE && m_pobClusterStructure->m_pobInstantKORecoverAttackCluster)
	{
		// Set up a temporary attack link
		const CAttackLink* pobTempAttackLink = 0;
	
		// Find something in our IKORA attack cluster
		pobTempAttackLink = m_pobClusterStructure->m_pobInstantKORecoverAttackCluster->GetNextAttackNode(eAttackType);

		if ( pobTempAttackLink && pobTempAttackLink->GetAttackDataP() )
		{
			// Grab the data
			m_obRequestedAttack.pobAttackLink = pobTempAttackLink;
			m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();

			// Store the attack type that this was requested through
			m_obRequestedAttack.eRequestType = eAttackType;

			//ntPrintf("AttackTracker SelectInstantKORecoverAttack() %s\n", ObjectDatabase::Get().GetNameFromPointer(m_obRequestedAttack.pobAttackData));

			return true;
		}
	}

	//ntPrintf("AttackTracker SelectInstantKORecoverAttack() FAIL\n");

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::SelectNewAttack
*
*	DESCRIPTION		Returns true if a new attack was successfully selected
*
***************************************************************************************************/
bool CAttackTracker::SelectNewAttack( const CAttackLink* pobFromLink, ATTACK_MOVE_TYPE eMoveType, HIT_LEVEL eHitLevel, bool bOnTheSpotEquivalent )
{
	// Make sure we are not here if in direct mode
	// ntAssert( !m_bDirectMode );

	//ntPrintf("SelectNewAttack:");

	// If we have a valid request type - make a request
	if ( ( eMoveType != AM_NONE ) && ( pobFromLink ) )
	{
		UNUSED(eHitLevel);

		//ntPrintf("...");
	
		// Set up a temporary attack link
		const CAttackLink* pobTempAttackLink = 0;

		// Look in our current attack first
		pobTempAttackLink = pobFromLink->GetNextAttackNode( eMoveType );

		// If that didn't return anything then get stuff from main clusters
		if ( !pobTempAttackLink )
			pobTempAttackLink = GetEvade( eMoveType );

		// If we're trying to do a grab as part of a string, look in the style clusters
		// This was needed because we now have auto blocking for grabs, so you need to stagger the enemy to grab them
		// Extra code needed in attacks.cpp to stop grabs getting repeatedly selected
		if ( !pobTempAttackLink &&
			(eMoveType == AM_RANGE_GRAB || eMoveType == AM_POWER_GRAB || eMoveType == AM_SPEED_GRAB) )
		{
			pobTempAttackLink = GetStyleLevelLink( eMoveType, eHitLevel, bOnTheSpotEquivalent );
		}

		if ( pobTempAttackLink && pobTempAttackLink->GetAttackDataP() )
		{
			// Grab the data
			m_obRequestedAttack.pobAttackLink = pobTempAttackLink;
			m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();

			// Store the attack type that this was requested through
			m_obRequestedAttack.eRequestType = eMoveType;

			if (m_obRequestedAttack.pobAttackLink->m_pobButtonHeldAttack)
			{
				m_obRequestedButtonHeldAttack.pobAttackLink = m_obRequestedAttack.pobAttackLink->m_pobButtonHeldAttack;
				m_obRequestedButtonHeldAttack.pobAttackData = m_obRequestedButtonHeldAttack.pobAttackLink->GetAttackDataP();
				m_obRequestedButtonHeldAttack.eRequestType = eMoveType;	
			}

			//ntPrintf("AttackTracker SelectNewAttack() %s\n", ObjectDatabase::Get().GetNameFromPointer(m_obRequestedAttack.pobAttackData));

			return true;
		} 
		else
		{
			//ntPrintf(" Failed from %s!\n", ObjectDatabase::Get().GetNameFromPointer(pobFromLink->GetAttackDataP()));
			//m_obRequestedAttack.pobAttackLink = pobFromLink->GetNextAttackNode( eMoveType );
		}
	}

	//ntPrintf("AttackTracker SelectNewAttack() FAIL\n");

	// Failed
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::AISelectStartAttack
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackTracker::AISelectStartAttack( ATTACK_MOVE_TYPE eMoveType )
{
	ntAssert(m_bDirectMode);

	if(eMoveType == AM_NONE)
		return false;

	// Make sure that clusters are ok. 
	if( !m_pobClusterStructure || !m_pobClusterStructure->m_pobLeadCluster )
		return false;

	// Reset our data
	Reset();
	m_obCurrentAttack.pobAttackLink = m_pobClusterStructure->m_pobLeadCluster;

	// Get the first attack from the cluster structure -
	m_obRequestedAttack.pobAttackLink = m_obCurrentAttack.pobAttackLink->GetNextAttackNode( eMoveType );

	if ( m_obRequestedAttack.pobAttackLink && m_obRequestedAttack.pobAttackLink->GetAttackDataP() )
	{
		// Get the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();
		m_obRequestedAttack.eRequestType = eMoveType;			

		return true;
	}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::AISelectShortLockAttack
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackTracker::AISelectShortLockAttack( ATTACK_MOVE_TYPE eMoveType )
{
	ntAssert( m_bDirectMode );

	if( eMoveType == AM_NONE )
		return false;

	// Reset our data
	Reset();

	// No attack cluster - weird, return false now. 
	if( !m_pobClusterStructure || !m_pobClusterStructure->m_pobShortRangeCluster ) 
		return false;

	m_obCurrentAttack.pobAttackLink = m_pobClusterStructure->m_pobShortRangeCluster;

	// Get the first attack from the cluster structure -
	m_obRequestedAttack.pobAttackLink = m_obCurrentAttack.pobAttackLink->GetNextAttackNode( eMoveType );

	if ( m_obRequestedAttack.pobAttackLink && m_obRequestedAttack.pobAttackLink->GetAttackDataP() )
	{
		// Get the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();
		m_obRequestedAttack.eRequestType = eMoveType;			

		return true;
	}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::AISelectLockAttack
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CAttackTracker::AISelectLockAttack( ATTACK_MOVE_TYPE eMoveType, float fDist )
{
	ntAssert( m_bDirectMode );

	if( eMoveType == AM_NONE )
		return false;

	// Reset our data
	Reset();

	// Error checks
	ntError(m_pobClusterStructure);

	// Choose the cluster, starting with the long range
	m_obCurrentAttack.pobAttackLink = m_pobClusterStructure->m_pobLongRangeCluster;

	if( !m_obCurrentAttack.pobAttackLink ||
		(m_pobClusterStructure->m_pobMediumRangeCluster && 
		m_pobClusterStructure->m_pobMediumRangeCluster->GetNextAttackNode( eMoveType ) &&
		m_pobClusterStructure->m_pobMediumRangeCluster->GetNextAttackNode( eMoveType )->GetAttackDataP() &&
		m_pobClusterStructure->m_pobMediumRangeCluster->GetNextAttackNode( eMoveType )->GetAttackDataP()->m_fMaxDistance >= fDist) )
	{
		m_obCurrentAttack.pobAttackLink = m_pobClusterStructure->m_pobMediumRangeCluster;
	}

	if( !m_obCurrentAttack.pobAttackLink ||
		(m_pobClusterStructure->m_pobShortRangeCluster && 
		m_pobClusterStructure->m_pobShortRangeCluster->GetNextAttackNode( eMoveType ) &&
		m_pobClusterStructure->m_pobShortRangeCluster->GetNextAttackNode( eMoveType )->GetAttackDataP() &&
		m_pobClusterStructure->m_pobShortRangeCluster->GetNextAttackNode( eMoveType )->GetAttackDataP()->m_fMaxDistance >= fDist) )
	{
		m_obCurrentAttack.pobAttackLink = m_pobClusterStructure->m_pobShortRangeCluster;
	}

	// Get the first attack from the cluster structure -
	m_obRequestedAttack.pobAttackLink = m_obCurrentAttack.pobAttackLink->GetNextAttackNode( eMoveType );

	if ( m_obRequestedAttack.pobAttackLink && m_obRequestedAttack.pobAttackLink->GetAttackDataP() )
	{
		// Get the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();
		m_obRequestedAttack.eRequestType = eMoveType;			

		return true;
	}

	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::AISelectNextAttack
*
*	DESCRIPTION		Returns true if a new attack was successfully selected
*
***************************************************************************************************/
bool CAttackTracker::AISelectNextAttack( ATTACK_MOVE_TYPE eMoveType )
{
	ntAssert(m_bDirectMode);

	if(eMoveType == AM_NONE)
		return false;

	// Make sure that clusters are ok. 
	if( !m_obCurrentAttack.pobAttackLink )
		return false;

	// Set up the request data we can at this stage
	m_obRequestedAttack.pobAttackLink = m_obCurrentAttack.pobAttackLink->GetNextAttackNode( eMoveType );

	if ( m_obRequestedAttack.pobAttackLink && m_obRequestedAttack.pobAttackLink->GetAttackDataP() )
	{
		// Grab the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();

		// Store the attack type that this was requested through
		m_obRequestedAttack.eRequestType = eMoveType;
		return true;
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::AISelectAttack
*
*	DESCRIPTION		Allows an AI to select their attack from an attack link.	
*
***************************************************************************************************/
bool CAttackTracker::AISelectAttack( const CAttackLink* pAttackLink )
{
	ntAssert(m_bDirectMode);

	// Check that the attack link is valid
	if( !pAttackLink )
		return false;

	// Make sure that clusters are ok. 
	if( !m_pobClusterStructure || !m_pobClusterStructure->m_pobLeadCluster )
		return false;

	// Reset our data
	Reset();

	// Set the current attack as the lead cluster, 
	m_obCurrentAttack.pobAttackLink = m_pobClusterStructure->m_pobLeadCluster;

	// Save the attack link
	m_obRequestedAttack.pobAttackLink = pAttackLink;

	if ( m_obRequestedAttack.pobAttackLink && m_obRequestedAttack.pobAttackLink->GetAttackDataP() )
	{
		// Get the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();		

		return true;
	}

	return false;
}

bool CAttackTracker::AISelectNextAttack( const CAttackLink* pobAttackLink )
{
	ntAssert(m_bDirectMode);

	// Make sure that clusters are ok. 
	if( !m_obCurrentAttack.pobAttackLink )
		return false;

	// Set up the request data we can at this stage
	m_obRequestedAttack.pobAttackLink = pobAttackLink;

	if ( m_obRequestedAttack.pobAttackLink && m_obRequestedAttack.pobAttackLink->GetAttackDataP() )
	{
		// Grab the data
		m_obRequestedAttack.pobAttackData = m_obRequestedAttack.pobAttackLink->GetAttackDataP();
		return true;
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::ClearRequested
*
*	DESCRIPTION		We may want to clear out a request that we don't want
*
***************************************************************************************************/
bool CAttackTracker::ClearRequested( void )
{
	// We shouldn't clear anything if we're waiting otherwise we'll get ERRORS
	if (this->m_bWaiting)
		return false;

	// If we have a requested attack clear it and indicate 
	// that it was cleared
	if ( m_obRequestedAttack.IsComplete() )
	{
		m_obRequestedButtonHeldAttack.Reset();
		m_obRequestedAttack.Reset();
		return true;
	}

	// Otherwise if we are here we didn't need to do anything
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAttackTracker::AIQueryAttackLink
*
*	DESCRIPTION		Test function... 
*
***************************************************************************************************/
const CAttackLink* CAttackTracker::AIQueryAttackLink( ATTACK_MOVE_TYPE eMoveType, float fLockOnRange, const GameGUID* pPrevGUID ) const
{
	// Obtian a pointer to the base link
	CAttackLink* pobBaseLink = 0;

	if( pPrevGUID && !pPrevGUID->IsNull() )
	{
		// Obtain the pointer through the objectdatabase
		DataObject* pobDataObject = ObjectDatabase::Get().GetDataObjectFromGUID( *pPrevGUID );
		ntAssert( pobDataObject != NULL );

		// Check that object is of the correct type
		ntAssert( strcmp( pobDataObject->GetClassName(), "CAttackLink" ) == 0 );

		pobBaseLink = (CAttackLink*) pobDataObject->GetBasePtr();

	}
	else if ( fLockOnRange != 0.0f )
	{
		pobBaseLink = m_pobClusterStructure->m_pobLongRangeCluster;

		if( !pobBaseLink ||
		   (m_pobClusterStructure->m_pobMediumRangeCluster && 
			m_pobClusterStructure->m_pobMediumRangeCluster->GetNextAttackNode( eMoveType ) &&
			m_pobClusterStructure->m_pobMediumRangeCluster->GetNextAttackNode( eMoveType )->GetAttackDataP() &&
			m_pobClusterStructure->m_pobMediumRangeCluster->GetNextAttackNode( eMoveType )->GetAttackDataP()->m_fMaxDistance >= fLockOnRange) )
		{
			pobBaseLink = m_pobClusterStructure->m_pobMediumRangeCluster;
		}

		if( !pobBaseLink ||
		   (m_pobClusterStructure->m_pobShortRangeCluster && 
			m_pobClusterStructure->m_pobShortRangeCluster->GetNextAttackNode( eMoveType ) &&
			m_pobClusterStructure->m_pobShortRangeCluster->GetNextAttackNode( eMoveType )->GetAttackDataP() &&
			m_pobClusterStructure->m_pobShortRangeCluster->GetNextAttackNode( eMoveType )->GetAttackDataP()->m_fMaxDistance >= fLockOnRange) )
		{
			pobBaseLink = m_pobClusterStructure->m_pobShortRangeCluster;
		}

		// Ahhhh
		if( !pobBaseLink )
			pobBaseLink = m_pobClusterStructure->m_pobLeadCluster;
	}
	else
	{
		pobBaseLink = m_pobClusterStructure->m_pobLeadCluster;
	}

	// Check that the link is valid. 
	ntAssert( pobBaseLink != NULL );

	// Return the attack node. 
	return (pobBaseLink->GetNextAttackNode( eMoveType ));
}
