//----------------------------------------------------------------------------------------------------
//!
//!	\file combathelper.cpp
//!	Combat Helper functions
//!
//----------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "combathelper.h"

#include "game/entity.h"

#include "objectdatabase/dataobject.h"
#include "game/messagehandler.h"

#include "game/attacks.h"
#include "game/strike.h"
#include "game/syncdcombat.h"


//-------------------------------------------------------------------------------------------------
// FUNC: CombatHelper::Combat_GenerateStrike
// DESCRIPTION: Generates a combat strike
//-------------------------------------------------------------------------------------------------
bool CombatHelper::Combat_GenerateStrike( CEntity* pobObject, CEntity* pobAttacker, CEntity* pobTarget, const CHashedString& pcAttackDef, int iHitArea  )
{
	ntAssert( pobObject );
	if ( !pobObject ) return false;

	// Make sure that we have a person who has been hit
	ntAssert( pobTarget );
	if ( !pobTarget ) return false;

	// Make sure that someone is not trying to hit themself
	if ( pobAttacker == pobTarget )
		return false;

	// If the target has an attack component we can issue them with a strike
	if ( pobTarget->GetAttackComponent() )
	{
		// Find the attack data that wwe are going to hit the opponent with
		CAttackData* pobAttackData = ObjectDatabase::Get().GetPointerFromName< CAttackData* >( pcAttackDef );
		if ( pobAttackData )
		{
			// Sort out an originating position - use an attacker if we have it
			CPoint obOriginatorPos = ( pobAttacker ) ? pobAttacker->GetPosition() : pobObject->GetPosition();

			// Issue a strike - using the thrown entity as the originator - but the reaction position 
			// relative to the character that issued the attack
			CStrike* pobStrike = 0;
			if (pobObject->IsProjectile())
			{
				pobStrike = NT_NEW CStrike(	0, 
											pobTarget, 
											pobAttackData, 
											1.0f, 
											1.0f, 
											false, 
											false, 
											false,
											false,
											false,
											false,
											pobObject,
											obOriginatorPos,
											iHitArea);
			}
			else
			{
				pobStrike = NT_NEW CStrike(	0, 
											pobTarget, 
											pobAttackData, 
											1.0f, 
											1.0f, 
											false, 
											false, 
											false,
											false,
											false,
											false,
											0,
											obOriginatorPos,
											iHitArea);
			}

			ntError( pobStrike );

			// Post the strike
			SyncdCombat::Get().PostStrike( pobStrike );

			// We were successful
			return true;
		}

		// We couldn't find any atta
		user_warn_p( 0, ( "WARNING: Unable to find CAttackData %s\n", ntStr::GetString(pcAttackDef) ) );
	}

	// Otherwise we can still send the entity a message that it may be able to handle
	else if ( pobTarget->GetMessageHandler() )
	{
		// Send the message
		CMessageSender::SendEmptyMessage( "msg_object_strike", pobTarget->GetMessageHandler() );

		// We didn't manage to send a strike
		return false;
	}

	// We didn't manage to send a strike
	return false;
}
