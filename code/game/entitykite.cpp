//--------------------------------------------------
//!
//!	\file game/entitykite.cpp
//!	Definition of the kite entity object
//!
//--------------------------------------------------
#include "Physics/config.h"
#include "objectdatabase/dataobject.h"
#include "messagehandler.h"
#include "movement.h"
#include "interactioncomponent.h"
#include "physics/collisionbitfield.h"
#include "Physics/system.h"
#include "Physics/singlerigidlg.h"
#include "Physics/animatedlg.h"
#include "anim/animator.h"

#include "camera/camutils.h"

// Debug includes 
#include "core/OSDDisplay.h"
#include "core/Timer.h"

/*#include "game/luaattrtable.h"
#include "Physics/system.h"
#include "anim/animator.h"
#include "core/exportstruct_anim.h"*/


#include "game/entitykite.h"

void ForceLinkFunctionKite()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionKite() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Object_Kite, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	OVERRIDE_DEFAULT(Clump, "Resources\\Objects\\ninjakite\\ninjakite.clump")
	OVERRIDE_DEFAULT(DefaultDynamics, "Animated")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_AnimationContainer, "KiteAnimContainer", AnimationContainer)

	PUBLISH_VAR_AS(m_obAnimationKiteFlight,	  KiteFlightAnim)
	//PUBLISH_VAR_AS(m_obAnimationKiteRelease,  KiteReleaseAnim)
	PUBLISH_VAR_AS(m_obAnimationKiteLeave,	  KiteLeaveAnim)
	//PUBLISH_VAR_AS(m_obAnimationKiteCrash,	  KiteCrashAnim)
	
	PUBLISH_VAR_AS(m_obAnimationNinjaFlight,  FlightAnim)
	//PUBLISH_VAR_AS(m_obAnimationNinjaCrash,   CrashAnim)
	PUBLISH_VAR_AS(m_obAnimationNinjaRelease, ReleaseAnim)
	//PUBLISH_VAR_AS(m_obAnimationNinjaFall,	  FallAnim)
	//PUBLISH_VAR_AS(m_obAnimationNinjaLand,	  LandAnim)

	//PUBLISH_VAR_AS(m_obUserPosition, UserPosition)
	//PUBLISH_VAR_AS(m_obUserRotation, UserRotation)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obUserTransform, "ROOT", UserTransformName)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinTurnRadius, 5.0f, TurnRadius)
	
	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Kite State Machine
//!
//--------------------------------------------------
STATEMACHINE(INTERACTABLE_KITE_FSM, Object_Kite)
	INTERACTABLE_KITE_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
				ME->m_pOther = 0;
			END_EVENT(true)

			EVENT(msg_running_action)
			EVENT(msg_action)
			{
				ME->m_pOther = (Character*)msg.GetEnt("Other");
				ntError(!ME->m_pOther || ME->m_pOther->IsCharacter());

				// Player won't activate a kite
				if ( ME->m_pOther->IsPlayer() )
				{
					Message msgExitState(msg_exitstate);
					msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
					SET_STATE(DEFAULT);

					END_EVENT(true)
				}
				SET_STATE(ATTACH);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(ATTACH)
		BEGIN_EVENTS
			ON_ENTER
			{
				CEntity* pobUser = ME->m_pOther;
				
				ME->GetInteractionComponent()->ExcludeCollisionWith( pobUser );

				//pobUser->GetMovement()->Lua_AltStartLinkedMovement(ME->m_obAnimationNinjaFlight, ME, ME->m_obUserTransform, ME->m_obUserPosition, ME->m_obUserRotation);
				SET_STATE(FLIGHT);
			}
			END_EVENT(true)

			EVENT(Trigger)
				SET_STATE(FLIGHT);
			END_EVENT(true)

			EVENT(msg_interrupt)
			//EVENT(msg_projcol)
				SET_STATE(CRASH);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(FLIGHT)
		BEGIN_EVENTS
			ON_ENTER
				// Start the flying animation
				if ( !ntStr::IsNull(ME->m_obAnimationKiteFlight) )
				{
					CEntity* pobUser = ME->m_pOther;

					// Reparent the user to the kite
					//pobUser->Lua_Reparent(ME, ME->m_obUserTransform);
					//pobUser->Lua_SetLocalTransform(0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);

					pobUser->GetMovement()->Lua_AltStartFullyLinkedMovement(ME->m_obAnimationNinjaFlight, ME, ME->m_obUserTransform, ME->m_obUserPosition, ME->m_obUserRotation);
					pobUser->GetPhysicsSystem()->Lua_Character_SetCollidable( false );

					ME->GetMovement()->Lua_AltStartSimpleMovement( ME->m_obAnimationKiteFlight, false, false, false );
					ME->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_animdone", ME );

					ME->m_obPrevPos = ME->GetPosition();
					ME->m_dPrevTime = CTimer::Get().GetGameTime();
				}	
			END_EVENT(true)

			ON_UPDATE
			{
				// FIX ME - Localy storing velocity as Velocity of animated LG incorrect from havok.  Remove this when we have Havok 4
				CDirection obDisp(ME->GetPosition() - ME->m_obPrevPos);
				float dTimeStep = (float)(CTimer::Get().GetGameTime() - ME->m_dPrevTime);

				ME->m_obPrevPos = ME->GetPosition();
				ME->m_dPrevTime = CTimer::Get().GetGameTime();

				CDirection obLinVel;
				if ( dTimeStep > EPSILON )
					obLinVel = obDisp/dTimeStep;
			}
			END_EVENT(true)

			EVENT(msg_animdone)
				SET_STATE(RELEASE);
			END_EVENT(true)

			EVENT(msg_interrupt)
			//EVENT(msg_projcol)
				SET_STATE(CRASH);
			END_EVENT(true)

			EVENT(msg_activate)
				SET_STATE(DEFAULT);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(CRASH)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Don't fly anymore
				ME->Lua_AnimStopAll( );

				// Crash anim
				/*if ( !ntStr::IsNull(ME->m_obAnimationKiteCrash) )
				{
					//ME->GetMovement()->ClearControllers();
					ME->GetMovement()->Lua_AltStartSimpleMovement( ME->m_obAnimationKiteCrash, false, false, false );
					ME->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_animdone", ME );

					CEntity* pobUser = ME->m_pOther;
					pobUser->GetMovement()->Lua_AltStartLinkedMovement(ME->m_obAnimationNinjaCrash, ME, ME->m_obUserTransform, ME->m_obUserPosition, ME->m_obUserRotation);

				}*/

				Physics::AnimatedLG* lg = (Physics::AnimatedLG*)ME->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
				ntAssert_p(lg, ("Kite must have \"Animated\" Default Dynamics"));

				// FIX ME - Velocity of animated LG incorrect from havok.  Look again when we have Havok 4
				//CDirection obLinVel( lg->GetLinearVelocity() );
				//ntPrintf("Initial Lin Vel %f,%f,%f\n", obLinVel.X(), obLinVel.Y(), obLinVel.Z());

				CDirection obDisp(ME->GetPosition() - ME->m_obPrevPos);
				float dTimeStep = (float)(CTimer::Get().GetGameTime() - ME->m_dPrevTime);

				ME->m_obPrevPos = ME->GetPosition();
				ME->m_dPrevTime = CTimer::Get().GetGameTime();

				CDirection obLinVel;
				if ( dTimeStep > EPSILON )
					obLinVel = obDisp/dTimeStep;

				// Go under dynamic motion
				if(lg)
					lg->MakeDynamic();

				// Spiral in plane, so store vertical vel for later
				ME->m_obVerticalVel = obLinVel.Y();
				obLinVel.Y() = 0.0f; 

				// Angular velocity to achive m_fMinTurnRadius
				CDirection obRadius( obLinVel );
				obRadius = obRadius.Cross( CDirection( 0.0f, 1.0f, 0.0f) );
				obRadius.Normalise();
				obRadius *= ME->m_fMinTurnRadius;
				
				// W = (V x R) / |R|^2
				ME->m_obAngularVel = obLinVel.Cross( obRadius );  
				ME->m_obAngularVel /= ME->m_fMinTurnRadius * ME->m_fMinTurnRadius;	

				// Spiral centre
				ME->m_obSpiralCentre = ME->GetPosition() + obRadius;

				// Linear velocity inherted from animation
				obLinVel.Y() = ME->m_obVerticalVel;
				lg->SetLinearVelocity(obLinVel);

				// Set angular velocity
				lg->SetAngularVelocity(ME->m_obAngularVel);

				// Set a collision msg callback
				ME->GetPhysicsSystem()->GetCollisionCallbackHandler()->SetCollisionCallback(5.0f);

				// Kill the operator
				if ( ME->m_pOther )
				{
					ME->m_pOther->GetPhysicsSystem()->Lua_ActivateState("Ragdoll");
					ME->m_pOther->GetPhysicsSystem()->Lua_Ragdoll_SetMotionType(4);

					// Reparent the ragdoll to the kite
					ME->m_pOther->Lua_Reparent(ME, ME->m_obUserTransform);
					ME->m_pOther->Lua_SetLocalTransform(0.0f,-0.6f,0.0f,0.0f,0.0f,0.0f);
				}
			}
			END_EVENT(true)

			ON_UPDATE
			{
				Physics::AnimatedLG* lg = (Physics::AnimatedLG*)ME->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
			
				CPoint obCurPos = ME->GetPosition();
				
				CDirection obLinVel( lg->GetLinearVelocity() );
				//ME->m_obVerticalVel = obLinVel.Y();

				////////////////////////////////////////////////////////
				// Debug current lin vel (Red)
				//CPoint obStart, obEnd;
				//obStart = obCurPos;
				//obEnd = obCurPos + obLinVel;
				//g_VisualDebug->RenderLine(obStart,obEnd,0xffff0000);
				////////////////////////////////////////////////////////

				////////////////////////////////////////////////////////
				// Debug spiral centre (Green)
				//obStart = ME->m_obSpiralCentre;
				//obEnd = obStart;
				//obStart.Y() += 200.0f;
				//obEnd.Y() -= 200.0f;
				//g_VisualDebug->RenderLine(obStart,obEnd,0xff00ff00);
				////////////////////////////////////////////////////////

				CDirection obSpCr = (CDirection)ME->m_obSpiralCentre;
				CDirection obRadius( obCurPos - obSpCr ); 
				obRadius.Y() = 0.0f; // In plane for time being
				obLinVel = ME->m_obAngularVel.Cross( obRadius );  // V = W x R
	
				ntAssert(!(obRadius.Y()>EPSILON));

				obLinVel.Y() = ME->m_obVerticalVel;	// Put our vertical speed back on
				lg->SetLinearVelocity( obLinVel );	// Set velocites
				lg->SetAngularVelocity(ME->m_obAngularVel);	

				///////////////////////////////////////////////////////
				// Debug radius (Blue)
				//obStart = ME->m_obSpiralCentre;
				//obStart.Y() = obCurPos.Y();
				//obEnd = obStart;
				//obEnd += obRadius;
				//g_VisualDebug->RenderLine(obStart,obEnd,0xff0000ff);
				//////////////////////////////////////////////////////

				///////////////////////////////////////////////////////
				// Debug new lin vel (Red)
				//obStart = obCurPos;
				//obEnd = obCurPos + obLinVel;
				//g_VisualDebug->RenderLine(obStart,obEnd,0xffff0000);
				////////////////////////////////////////////////////////
			}
			END_EVENT(true)

			EVENT(msg_animdone)
				SET_STATE(DEFAULT);
			END_EVENT(true)

			EVENT(msg_collision)
				SET_STATE(ONGROUND);
			END_EVENT(true)

			EVENT(msg_activate)
			
				SET_STATE(DEFAULT);
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(ONGROUND)
		BEGIN_EVENTS
			ON_ENTER
				// FIX ME add crash particle/sound effects
				
			END_EVENT(true)
		END_EVENTS
	END_STATE // ONGROUND

	STATE(RELEASE)
		BEGIN_EVENTS
			ON_ENTER
			{
				if ( !ntStr::IsNull(ME->m_obAnimationKiteLeave) )
				{
					ME->GetMovement()->Lua_AltStartSimpleMovement( ME->m_obAnimationKiteLeave, false, false, false );
					ME->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_animdone", ME );
				}	

				//ME->m_pOther->Lua_ReparentToWorld();
				//ME->m_pOther->GetMovement()->ClearControllers();


				Transform* pTransform = ME->GetHierarchy()->GetTransform( ME->m_obUserTransform );

				const CMatrix& obTargetWorldMatrix=pTransform->GetWorldMatrix();

				// Calculate the translation delta
				//CDirection obTranslationOffset=CDirection(ME->m_obUserPosition) * obTargetWorldMatrix;

				CPoint obNewPosition(obTargetWorldMatrix.GetTranslation());
				//obNewPosition+=obTranslationOffset;

				// Calculate the rotation delta

				pTransform = ME->GetHierarchy()->GetRootTransform();
	
				const CMatrix& obTargetRotMatrix=pTransform->GetWorldMatrix();

				CQuat obTargetRotation(obTargetRotMatrix);

				//CQuat obNewRotation=obTargetRotation * CCamUtil::QuatFromEuler_XYZ( ME->m_obUserRotation.X(), ME->m_obUserRotation.Y(), ME->m_obUserRotation.Z() );

				// One frame delay to allow movement controller to be killed off
				Message msgNotifyDetach(msg_detach);
				ME->GetMessageHandler()->QueueMessage(msgNotifyDetach);

				// Have to pop our position back to the way the land animation expects
				ME->m_pOther->GetMovement()->Lua_AltStartCorrectiveMovement(obNewPosition, obTargetRotation);
			}
			END_EVENT(true)

			EVENT(msg_detach)

				if ( !ntStr::IsNull(ME->m_obAnimationNinjaRelease) )
				{

					ME->m_pOther->GetPhysicsSystem()->Lua_Character_SetCollidable( true );
					ME->m_pOther->GetMovement()->Lua_AltStartSimpleMovement( ME->m_obAnimationNinjaRelease, false, false, true );
					
					/* // Nice arial-like landing system removed :(
					// Build the definition for dropping
					ContinuationTransitionDef obDropDefinitionDef;
					obDropDefinitionDef.SetDebugNames( "Ninja Drop Loop", "ContinuationTransitionDef" );
					obDropDefinitionDef.m_obAnimationName = ME->m_obAnimationNinjaFall;
					obDropDefinitionDef.m_bHorizontalVelocity = true;
					obDropDefinitionDef.m_bVerticalVelocity = true;
					obDropDefinitionDef.m_bVerticalAcceleration = false;
					obDropDefinitionDef.m_bLooping = true;
					obDropDefinitionDef.m_bEndOnGround = true;
					obDropDefinitionDef.m_bApplyGravity = true;

					// Push the controller on to the movement component
					ME->m_pOther->GetMovement()->AddChainedController( obDropDefinitionDef );

					// Build the definition for landing
					StandardTargetedTransitionDef obLandDefinitionDef;
					obLandDefinitionDef.SetDebugNames( "Ninja Land", "StandardTargetedTransitionDef" );
					obLandDefinitionDef.m_bApplyGravity = true;
					obLandDefinitionDef.m_bLooping = false;
					obLandDefinitionDef.m_bReversed = false;
					obLandDefinitionDef.m_bTrackTarget = false;
					obLandDefinitionDef.m_obAnimationName = ME->m_obAnimationNinjaLand;

					// Push the controller on to the movement component
					ME->m_pOther->GetMovement()->AddChainedController( obLandDefinitionDef );*/

					ME->m_pOther->GetMovement()->Lua_AltSetMovementCompleteMessage( "msg_movementdone", ME->m_pOther );
				}	
			END_EVENT(true)

			EVENT(msg_movementdone)
				if ( ME->m_pOther )
				{
					Message msgNotifyLand(msg_landed);
					ME->m_pOther->GetMessageHandler()->QueueMessage(msgNotifyLand);

					Message msgExitState(msg_exitstate);
					msgExitState.SetEnt( CHashedString(HASH_STRING_SENDER), ME);
					ME->m_pOther->GetMessageHandler()->QueueMessage(msgExitState);
				}
				SET_STATE(DEFAULT);
			END_EVENT(true)
		END_EVENTS
	END_STATE	// RELEASE
END_STATEMACHINE


//--------------------------------------------------
//!
//!	Object_Kite::Object_Kite()
//!	Default constructor
//!
//--------------------------------------------------
Object_Kite::Object_Kite()
:	m_pOther ( 0 )
,	m_pobAnim ( 0 )
,	m_obUserPosition ( CONSTRUCT_CLEAR )
,	m_obUserRotation ( CONSTRUCT_CLEAR )
,	m_obAngularVel ( CONSTRUCT_CLEAR )
,	m_obSpiralCentre ( CONSTRUCT_CLEAR )
,	m_obVerticalVel ( 0.0f )
,	m_obPrevPos ( CONSTRUCT_CLEAR )
,	m_dPrevTime ( 0.0f )
{
	// You don't interact directly with kites
	m_eType = EntType_Object;
}

//--------------------------------------------------
//!
//!	Object_Kite::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_Kite::OnPostConstruct()
{
	CEntity::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();
	InstallDynamics();
	InstallAudioChannel();
	
	// requires a hierarchy
	if( !GetHierarchy() )
		InstallHierarchy();
	if(! m_AnimationContainer.IsNull())
	{
		m_pobAnimator = CAnimator::Create( static_cast< EntityAnimSet * >( this ), m_pobHierarchy, this );

		// Pass on the animator to the EntityAnimSet.
		EntityAnimSet::InstallAnimator( m_pobAnimator, m_AnimationContainer );
	}

	if(!GetMovement()) // Create a movement system if one doesn't already exist
	{
		CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(this, GetAnimator(), GetPhysicsSystem());
		SetMovement(pobMovement);
	}

	// Create and attach the statemachine
	INTERACTABLE_KITE_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INTERACTABLE_KITE_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_Kite::~Object_Kite()
//!	Default destructor
//!
//--------------------------------------------------
Object_Kite::~Object_Kite()
{
}
