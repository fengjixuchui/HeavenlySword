/***************************************************************************************************
*
*	DESCRIPTION		Static (walls etc.) Entity
*
*	NOTES
*
***************************************************************************************************/
#include "Physics/config.h"

#include "objectdatabase/dataobject.h"
#include "game/luaglobal.h"
#include "game/luaattrtable.h"
#include "game/entityinvisiblewall.h"
#include "game/entitymanager.h"
#include "gfx/clump.h"
#include "game/interactioncomponent.h"
#include "game/messagehandler.h"
#include "game/messages.h"
#include "gfx/levelofdetail.h"
#include "area/areasystem.h"
#include "physics/system.h"
#include "physics/staticlg.h"
#include "physics/collisionbitfield.h"
#include "physics/physicsTools.h"

#include "game/attacks.h"
#include "fsm.h"

#include <hkDynamics/phantom/hkPhantom.h>


START_CHUNKED_INTERFACE(InvisibleWall, Mem::MC_ENTITY)
	PUBLISH_ACCESSOR_WITH_DEFAULT( CPoint, Position, GetPosition, SetPosition, CPoint(0.0f, 0.0f, 0.0f) )
	PUBLISH_ACCESSOR_WITH_DEFAULT( CQuat, Orientation, GetRotation, SetRotation, CQuat(0.0f, 0.0f, 0.0f, 1.0f) )
	PUBLISH_ACCESSOR( ntstd::String, Clump, GetClumpString, SetClumpString )
	PUBLISH_ACCESSOR_WITH_DEFAULT( int, CollideWith, GetCollideWith, SetCollideWith, 38 )
    PUBLISH_ACCESSOR_WITH_DEFAULT( bool, StopAfterTouchCamera, QStopAfterTouchCamera, StopAfterTouchCamera, true )
	PUBLISH_ACCESSOR_WITH_DEFAULT( bool, KillPassingKO, QKillPassingKO, KillPassingKO, true )
	PUBLISH_ACCESSOR_WITH_DEFAULT( bool, LetDeadRagdollPass, QLetDeadRagdollPass, LetDeadRagdollPass, true )
	PUBLISH_ACCESSOR_WITH_DEFAULT( bool, LetBigKOOfUnimportantPass, QLetSmallKOOfUnimportantPass, LetSmallKOOfUnimportantPass, true )
	PUBLISH_ACCESSOR_WITH_DEFAULT( bool, LetSmallKOOfUnimportantPass, QLetSmallKOOfUnimportantPass, LetSmallKOOfUnimportantPass, true )
	PUBLISH_ACCESSOR_WITH_DEFAULT( bool, LetBigKOOfUnimportantPass, QLetSmallKOOfUnimportantPass, LetSmallKOOfUnimportantPass, true )
	PUBLISH_ACCESSOR_WITH_DEFAULT( bool, LetSmallKOOfImportantPass, QLetSmallKOOfImportantPass, LetSmallKOOfImportantPass, false )
	PUBLISH_ACCESSOR_WITH_DEFAULT( bool, LetBigKOOfImportantPass, QLetSmallKOOfImportantPass, LetBigKOOfImportantPass, false )
	PUBLISH_ACCESSOR_WITH_DEFAULT( bool, OneSide, QOneSide, OneSide, false )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_iMappedAreaInfo, 0, SectorBits )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_replacesEnt, false, ReplacesEnt )
	PUBLISH_VAR_AS( m_entToReplace, SectorLODEnt )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bInitiallyActive, true, InitiallyActive )

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


// FSM to allow it to receive messages
STATEMACHINE(INVISIBLE_WALL_FSM, InvisibleWall)
	INVISIBLE_WALL_FSM( bool bActive )
	{
		if ( bActive )
			SET_INITIAL_STATE(ACTIVE);
		else
			SET_INITIAL_STATE(INACTIVE);
	}

	STATE(ACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->Activate();
			}
			END_EVENT(true)

			EVENT(Deactivate)
			{
				SET_STATE( INACTIVE );
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

	STATE(INACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->Deactivate();
			}
			END_EVENT(true)

			EVENT(Activate)
			{
				SET_STATE( ACTIVE );
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE

END_STATEMACHINE // INVISIBLE_WALL_FSM

InvisibleWall::InvisibleWall() :
	m_listener(0)
{
	m_DefaultDynamics = "Static";
	m_eType = EntType_Collision;
	m_bDisableRender = true;
	m_bInitiallyActive = true;

	// set up default collision flags
	m_collisionFlags.base = 0;
	m_collisionFlags.flags.i_am = Physics::AI_WALL_BIT;
	m_collisionFlags.flags.i_collide_with = Physics::CHARACTER_CONTROLLER_PLAYER_BIT | 
		Physics::CHARACTER_CONTROLLER_ENEMY_BIT | Physics::RAGDOLL_BIT;	

	m_collisionFlags.flags.kill_passing_KO = 1;

	m_collisionFlags.flags.not_collide_with_KO_states_important = 0;
	m_collisionFlags.flags.not_collide_with_KO_states_unimportant = Physics::BIG_KO_BIT;

	m_collisionFlags.flags.raycast_material = Physics::COLLISION_ENVIRONMENT_BIT;
}

InvisibleWall::~InvisibleWall()
{
	//NT_DELETE( m_obAttributeTable );
	Physics::StaticLG * lg = (Physics::StaticLG *) m_pobPhysicsSystem->GetFirstGroupByType(Physics::LogicGroup::STATIC_LG);
	if (lg)
		lg->RemoveCollisionListener(m_listener);

	NT_DELETE( m_listener);
}


void InvisibleWall::Activate( void )
{
	ntAssert( m_pobPhysicsSystem );
	m_pobPhysicsSystem->Activate();
}


void InvisibleWall::Deactivate( void )
{
	ntAssert( m_pobPhysicsSystem );
	m_pobPhysicsSystem->Deactivate();
}


void InvisibleWall::OnPostConstruct()
{
	CEntity::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();

	// Create rigid body physics
	Lua_CreatePhysicsSystem();

	//Physics::StaticLG* lg =  Physics::ClumpTools::ConstructStaticLGFromClump( this );
	Physics::StaticLG* lg = NT_NEW Physics::StaticLG(GetName(), this);
	lg->Load();

	if( lg )
	{		
		m_pobPhysicsSystem->AddGroup( (Physics::LogicGroup*)lg );
		lg->Activate();

		// add collision listener for killing and one side 
		m_listener = NT_NEW Physics::InvisibleWallsListener(this);
		lg->AddCollisionListener(m_listener);
	}	
	else	
		ntPrintf("%s(%d): ### PHYSICS ERROR - %s logic group not created for entity %s with clump %s\n", __FILE__, __LINE__, "Static", GetName().c_str(), GetClumpString().c_str());	


	m_pobPhysicsSystem->SetCollisionFilterInfo(m_collisionFlags.base);
	
	// invisible..
	m_bCastShadows = false;
	m_bRecieveShadows = false;
	m_bDisableRender = true;

	SetPosition( m_InitialPosition );
	SetRotation( m_InitialRotation );	

	// Create and attach the statemachine
	INVISIBLE_WALL_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) INVISIBLE_WALL_FSM( m_bInitiallyActive );
	ATTACH_FSM(pFSM);

	SetPostConstructed(true);
}

void InvisibleWall::SetCollisionFilterInfo()
{
	if (m_pobPhysicsSystem)
	{
		m_pobPhysicsSystem->SetCollisionFilterInfo(m_collisionFlags.base);
		m_pobPhysicsSystem->UpdateCollisionFilter();
	}
}

int InvisibleWall::GetCollideWith() const
{
	return m_collisionFlags.flags.i_collide_with;
}
void InvisibleWall::SetCollideWith(const int& collideWith)
{
	m_collisionFlags.flags.i_collide_with = collideWith;
	SetCollisionFilterInfo();
}

bool InvisibleWall::QStopAfterTouchCamera() const
{
	return m_collisionFlags.flags.raycast_material & Physics::COOLCAM_AFTERTOUCH_BIT;
}

void InvisibleWall::StopAfterTouchCamera(const bool& stop)
{
	if (stop)
		m_collisionFlags.flags.raycast_material |= Physics::COOLCAM_AFTERTOUCH_BIT;
	else
		m_collisionFlags.flags.raycast_material &= ~Physics::COOLCAM_AFTERTOUCH_BIT;

	SetCollisionFilterInfo();
}

bool InvisibleWall::QKillPassingKO() const
{
	return m_collisionFlags.flags.kill_passing_KO;
}

void InvisibleWall::KillPassingKO(const bool& pass)
{
	if (pass)
		m_collisionFlags.flags.kill_passing_KO = 1;
	else	
		m_collisionFlags.flags.kill_passing_KO = 0;
	
	SetCollisionFilterInfo();
}

bool InvisibleWall::QLetDeadRagdollPass() const
{
	return m_collisionFlags.flags.not_collide_with_dead_ragdolls;
}

void InvisibleWall::LetDeadRagdollPass(const bool& pass)
{
	if (pass)
		m_collisionFlags.flags.not_collide_with_dead_ragdolls = 1;
	else
		m_collisionFlags.flags.not_collide_with_dead_ragdolls = 0;

	SetCollisionFilterInfo();
}

bool InvisibleWall::QLetSmallKOOfUnimportantPass() const
{
	return m_collisionFlags.flags.not_collide_with_KO_states_unimportant & Physics::SMALL_KO_BIT;
}

void InvisibleWall::LetSmallKOOfUnimportantPass(const bool& pass)
{
	if (pass)
		m_collisionFlags.flags.not_collide_with_KO_states_unimportant |= Physics::SMALL_KO_BIT;
	else
		m_collisionFlags.flags.not_collide_with_KO_states_unimportant &= ~Physics::SMALL_KO_BIT;

	SetCollisionFilterInfo();
}

bool InvisibleWall::QLetBigKOOfUnimportantPass() const
{
	return m_collisionFlags.flags.not_collide_with_KO_states_unimportant & Physics::BIG_KO_BIT;
}

void InvisibleWall::LetBigKOOfUnimportantPass(const bool& pass)
{
	if (pass)
		m_collisionFlags.flags.not_collide_with_KO_states_unimportant |= Physics::BIG_KO_BIT;
	else
		m_collisionFlags.flags.not_collide_with_KO_states_unimportant &= ~Physics::BIG_KO_BIT;

	SetCollisionFilterInfo();
}

bool InvisibleWall::QLetSmallKOOfImportantPass() const
{
	return m_collisionFlags.flags.not_collide_with_KO_states_important & Physics::SMALL_KO_BIT;
}

void InvisibleWall::LetSmallKOOfImportantPass(const bool& pass)
{
	if (pass)
		m_collisionFlags.flags.not_collide_with_KO_states_important |= Physics::SMALL_KO_BIT;
	else
		m_collisionFlags.flags.not_collide_with_KO_states_important &= ~Physics::SMALL_KO_BIT;

	SetCollisionFilterInfo();
}

bool InvisibleWall::QLetBigKOOfImportantPass() const
{
	return m_collisionFlags.flags.not_collide_with_KO_states_important & Physics::BIG_KO_BIT;
}

void InvisibleWall::LetBigKOOfImportantPass(const bool& pass)
{
	if (pass)
		m_collisionFlags.flags.not_collide_with_KO_states_important |= Physics::BIG_KO_BIT;
	else
		m_collisionFlags.flags.not_collide_with_KO_states_important &= ~Physics::BIG_KO_BIT;

	SetCollisionFilterInfo();
}

bool InvisibleWall::QOneSide() const
{
	return m_collisionFlags.flags.one_sided;
}

void InvisibleWall::OneSide(const bool& side)
{
	if (side)
		m_collisionFlags.flags.one_sided = 1;
	else
		m_collisionFlags.flags.one_sided = 0;

	SetCollisionFilterInfo();
}

namespace Physics 
{
	//! If character is in KOs that will pass, reject points and kill character.
	void InvisibleWallsListener::contactPointAddedCallback (hkContactPointAddedEvent &event)
	{
		if (m_wall->QOneSide())
		{
			// wall is one sided check if we are on the side where we should collide... 
			if (event.m_bodyA.getShape()->getType() == HK_SHAPE_TRIANGLE)
			{
				const hkTriangleShape * triangle = static_cast<const hkTriangleShape*>(event.m_bodyA.getShape());

				if (Tools::CalcTriangleNormal(triangle->getVertices()).dot3(event.m_contactPoint->getNormal()) < 0)
				{
					event.m_status = HK_CONTACT_POINT_REJECT;
					return;
				}
			}

			if (event.m_bodyB.getShape()->getType() == HK_SHAPE_TRIANGLE)
			{
				const hkTriangleShape * triangle = static_cast<const hkTriangleShape*>(event.m_bodyB.getShape());

				if (Tools::CalcTriangleNormal(triangle->getVertices()).dot3(event.m_contactPoint->getNormal()) > 0)
				{
					event.m_status = HK_CONTACT_POINT_REJECT;
					return;
				}
			}
		}

		if (!m_wall->QKillPassingKO())
			return; // nothing to do.... 

		EntityCollisionFlag infoA; 
		infoA.base = event.m_bodyA.getRootCollidable()->getCollisionFilterInfo();

		EntityCollisionFlag infoB; 
		infoB.base = event.m_bodyB.getRootCollidable()->getCollisionFilterInfo();

		AIWallCollisionFlag infoWall;
		ChatacterControllerCollisionFlag infoCharacter;

		const hkCollidable * character; 

		if (infoA.flags.i_am & Physics::AI_WALL_BIT)
		{
			if (infoB.flags.i_am & (Physics::RAGDOLL_BIT | Physics::CHARACTER_CONTROLLER_PLAYER_BIT |
				Physics::CHARACTER_CONTROLLER_ENEMY_BIT))
			{
				infoWall.base = infoA.base;
				infoCharacter.base = infoB.base;
				character = event.m_bodyB.getRootCollidable();
			}
			else
				return;
		}
		else
		{
			if (infoA.flags.i_am & (Physics::RAGDOLL_BIT | Physics::CHARACTER_CONTROLLER_PLAYER_BIT |
				Physics::CHARACTER_CONTROLLER_ENEMY_BIT))
			{
				infoWall.base = infoB.base;
				infoCharacter.base = infoA.base;
				character = event.m_bodyA.getRootCollidable();
			}
			else
				return;
		}

		bool kill = false;

		if (infoCharacter.flags.i_am_important)
		{
			if (infoCharacter.flags.i_am_in_KO_state & infoWall.flags.not_collide_with_KO_states_important)
				kill = true;
		}
		else 
		{
			if (infoCharacter.flags.i_am_in_KO_state & infoWall.flags.not_collide_with_KO_states_unimportant)
				kill = true;
		}

		if (!kill)
			return;

		// let it pass but kill it
		event.m_status = HK_CONTACT_POINT_REJECT;

		// get entity and send it kill message
		hkRigidBody * obRB = hkGetRigidBody(character);
		CEntity * pobEntity = 0; 
		if( obRB)
		{
			if (obRB->hasProperty(PROPERTY_ENTITY_PTR) )
				pobEntity = (CEntity*) obRB->getProperty(PROPERTY_ENTITY_PTR).getPtr();
		}
		else
		{
			hkPhantom * oPhantom = hkGetPhantom(character);
			if( oPhantom && oPhantom->hasProperty(PROPERTY_ENTITY_PTR) )
				pobEntity = (CEntity*) oPhantom->getProperty(PROPERTY_ENTITY_PTR).getPtr();
		}

		// Tell character that they need to die as soon as they can 	
		if (pobEntity && pobEntity->IsCharacter() && pobEntity->ToCharacter()->GetAttackComponent())
			pobEntity->ToCharacter()->GetAttackComponent()->NotifyDieOutOfCurrentMovement();		
	}
}

