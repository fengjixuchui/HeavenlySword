//------------------------------------------------------------------------------------------
//!
//!	\file syncdcombat.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "game/syncdcombat.h"
#include "game/strike.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/attacks.h"

//------------------------------------------------------------------------------------------
//!
//!	SyncdCombat::SyncdCombat
//!	Construction
//!
//------------------------------------------------------------------------------------------
SyncdCombat::SyncdCombat( void )
:	m_obStrikesToSend()
{
}


//------------------------------------------------------------------------------------------
//!
//!	SyncdCombat::~SyncdCombat
//!	Destruction
//!
//------------------------------------------------------------------------------------------
SyncdCombat::~SyncdCombat( void )
{
	// Loop through all the strikes we have and destroy them
	ntstd::List< CStrike* >::iterator obEnd = m_obStrikesToSend.end();
	for ( ntstd::List< CStrike*  >::iterator obIt = m_obStrikesToSend.begin(); obIt != obEnd; ++obIt )
	{
		NT_DELETE( ( *obIt ) );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	SyncdCombat::Update
//!	Send off all the strikes in our list
//!
//------------------------------------------------------------------------------------------
void SyncdCombat::Update( void )
{
	// Loop through all the strikes we have and send them to their target
	ntstd::List< CStrike* >::iterator obEnd = m_obStrikesToSend.end();
	for ( ntstd::List< CStrike*  >::iterator obIt = m_obStrikesToSend.begin(); obIt != obEnd; ++obIt )
	{
		// Make sure we have a target
		ntAssert( ( *obIt )->GetTargetP() );

		// Now send it what we point to
		( *obIt )->GetTargetP()->GetAttackComponent()->ReceiveStrike( *obIt );
	}

	// Clear the list
	m_obStrikesToSend.clear();
}


//------------------------------------------------------------------------------------------
//!
//!	SyncdCombat::PostStrike
//!	Add a strike to be sent to the target at the beginning of next frame
//!
//------------------------------------------------------------------------------------------
void SyncdCombat::PostStrike( CStrike* pobStrike )
{
	// Store the pointer to the strike
	m_obStrikesToSend.push_back( pobStrike );
}
