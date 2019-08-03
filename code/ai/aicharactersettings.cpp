/***************************************************************************************************
*
*	DESCRIPTION		aicharactersettings.cpp - parameters used by AI state machines
*
*	NOTES
*
***************************************************************************************************/

#include "aicharactersettings.h"

/***************************************************************************************************
*
*	FUNCTION		CAIMeleeSettings constructor
*
*	DESCRIPTION		Default values for a melee combatant
*
***************************************************************************************************/
CAIMeleeSettings::CAIMeleeSettings( void )
{
	m_fPatrolLookTimeMax		= 0.0f;
	m_fInvHesitationTime		= 0.0f;
	m_fInvSoundTimeMin			= 0.0f;
	m_fInvSoundTimeMax			= 0.0f;
	m_fInvSoundTimeExtension	= 0.0f;
	m_fInvSightTimeMin			= 0.0f;
	m_fInvSightTimeMax			= 0.0f;
	m_fInvSightTimeExtension	= 0.0f;

}


CAIMeleeSettings::~CAIMeleeSettings( void )
{
}

