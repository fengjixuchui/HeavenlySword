//--------------------------------------------------
//!
//!	\file game/entityinteractableladder.cpp
//!	Definition of the Interactable Pushable entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "fsm.h"

#include "game/entityinteractableladder.h"

// Components
#include "game/interactioncomponent.h"
#include "game/movement.h"

#include "game/interactiontransitions.h"


void ForceLinkFunctionLadder()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionLadder() !ATTN!\n");
}


START_CHUNKED_INTERFACE(Interactable_Ladder, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(Interactable)
	COPY_INTERFACE_FROM(Interactable)

	OVERRIDE_DEFAULT(Clump, "ladder\\ladder.clump")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_LadderParams, "DefaultLadderParameters", LadderParameters)

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE

//--------------------------------------------------
//!
//! Interactable Ladder State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_LADDER_FSM, Interactable_Ladder)
	INTERACTABLE_LADDER_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;

				// Characters can use the ladder
				ME->GetInteractionComponent()->SetInteractionType(USE);
			END_EVENT(true)

			EVENT(msg_running_action)
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(!ME->m_pOther || ME->m_pOther->IsCharacter());
				ME->m_bRunTo = true;

				SET_STATE(MOVETOANDCLIMB);
				//SET_STATE(CLIMB);
			END_EVENT(true)

			EVENT(msg_action)
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(!ME->m_pOther || ME->m_pOther->IsCharacter());
				ME->m_bRunTo = false;

				SET_STATE(MOVETOANDCLIMB);
				//SET_STATE(CLIMB);
			END_EVENT(true)
		
			EVENT(msg_action_specific)
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(!ME->m_pOther || ME->m_pOther->IsCharacter());
				ME->m_bRunTo = false;

				SET_STATE(CLIMB);
				//SET_STATE(CLIMB);
			END_EVENT(true)
			
			EVENT(msg_action_specific_run)
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntAssert(!ME->m_pOther || ME->m_pOther->IsCharacter());
				ME->m_bRunTo = true;
	
				SET_STATE(CLIMB);
				//SET_STATE(CLIMB);
			END_EVENT(true)

		END_EVENTS
	END_STATE // DEFAULT

	STATE(MOVETOANDCLIMB)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				
				LadderParameters* pobLadderParams = ObjectDatabase::Get().GetPointerFromName<LadderParameters*>(ME->m_LadderParams);
				Character* pPlayer = ME->m_pOther;

				// Disable collision between ladder and user
				ME->GetInteractionComponent()->ExcludeCollisionWith( pPlayer );

				CPoint obPosition( pPlayer->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation() );
				CPoint obOffset(CONSTRUCT_CLEAR);
				CQuat obRotOffset(CONSTRUCT_IDENTITY);
				
				ME->m_bFromTop = ME->IsNamedUsePoint( obPosition, CHashedString("UsePoint_Top") );

				/*CQuat obRot = ME->GetRotation();	
				CMatrix obMatrix;
				obMatrix.SetFromQuat(obRot);*/

				if ( ME->m_bFromTop )
				{
					//obOffset = pobLadderParams->m_obMountTopPosition;
					obOffset.Y() += ME->GetInteractionComponent()->GetHeightFromUsePoints();
					obRotOffset = pobLadderParams->m_obMountTopRotation;
				}
				else
				{
					//obOffset = pobLadderParams->m_obMountBottomPosition;
					//obOffset = CPoint(0.0f,0.0f,0.5f);
					obRotOffset = pobLadderParams->m_obMountBottomRotation;
				}
				
				//pPlayer->GetMovement()->Lua_AltStartMoveToTransition(ME->m_bRunTo ? pobLadderParams->m_obAnimRunTo : pobLadderParams->m_obAnimMoveTo, ME, 0.0f, 0.25f, &obOffset );
				pPlayer->GetMovement()->Lua_StartFacingMoveToTransition(ME->m_bFromTop ? pobLadderParams->m_obAnimMountTop : pobLadderParams->m_obAnimMountBottom, ME, 0.0f, 1.0f, &obOffset, &obRotOffset );
				
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME );
				//pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", pPlayer );

				// Bring on ladder movement controller and tell player to exit interacting state once done.

				// Climb controller now chained for Dario's ladder que AI
				pPlayer->GetMovement()->Lua_LadderController(ME->m_LadderParams, ME, true);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", pPlayer);
				pPlayer->SetExitOnMovementDone(true);

				SET_STATE(DEFAULT);
			}
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_movementdone)
				SET_STATE(CLIMB);
			END_EVENT(true)
		END_EVENTS
	END_STATE // MOVETOANDCLIMB

	STATE(CLIMB)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->GetInteractionComponent()->Lua_SetInteractionPriority(NONE);
				
				Character* pPlayer = ME->m_pOther;

				// Bring on ladder movement controller and tell player to exit interacting state once done.
				pPlayer->GetMovement()->Lua_LadderController(ME->m_LadderParams, ME, false);
				pPlayer->GetMovement()->Lua_AltSetMovementCompleteMessage("msg_movementdone", pPlayer);
				pPlayer->SetExitOnMovementDone(true);

				SET_STATE(DEFAULT);
			}	
			END_EVENT(true)

			EVENT(msg_interrupt)
				SET_STATE(DEFAULT);
			END_EVENT(true)
		END_EVENTS
	END_STATE // CLIMB
END_STATEMACHINE // INTERACTABLE_LADDER_FSM


//--------------------------------------------------
//!
//!	Interactable_Ladder::Interactable_Ladder()
//!	Constructor
//!
//--------------------------------------------------
Interactable_Ladder::Interactable_Ladder()
{
	m_eType = EntType_Interactable;
	m_eInteractableType = EntTypeInteractable_Ladder;
}


//--------------------------------------------------
//!
//!	Interactable_Ladder::~Interactable_Ladder()
//!	Destructor
//!
//--------------------------------------------------
Interactable_Ladder::~Interactable_Ladder()
{
	if ( m_pSharedAttributes )
		NT_DELETE ( m_pSharedAttributes );
}


//--------------------------------------------------
//!
//!	Interactable_Ladder::OnPostConstruct()
//!	Post Construction stuff
//!
//--------------------------------------------------
void Interactable_Ladder::OnPostConstruct()
{
	// scee.sbashow - why straight to CEntity::OnPostConstruct(), and not through Interactable::OnPostConstruct() ?
	// 					So have changed it to Interactable::OnPostConstruct();
	Interactable::OnPostConstruct();

	// Install required components
	InstallMessageHandler();
	InstallDynamics();

	m_pSharedAttributes = NT_NEW LuaAttributeTable;
	m_pSharedAttributes->SetDataObject(ObjectDatabase::Get().GetDataObjectFromName(m_LadderParams));

	// Attach Ladder FSM to entity
	INTERACTABLE_LADDER_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_LADDER_FSM;
	ATTACH_FSM(pFSM);
}

bool Interactable_Ladder::IsNamedUsePoint(const CPoint& pobPos, const CHashedString obUseName)
{
	CUsePoint* pobUsePoint = GetInteractionComponent()->GetClosestUsePoint(pobPos);

	return (pobUsePoint->GetName() == obUseName);
}
