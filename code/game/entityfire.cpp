//--------------------------------------------------
//!
//!	\file game/entityfire.cpp
//!	Definition of the Fire object
//!
//--------------------------------------------------


#include "objectdatabase/dataobject.h"
#include "game/projectilemanager.h"
#include "game/entityfire.h"
#include "game/message.h"

void ForceLinkFunctionEntityFire()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionEntityFire() !ATTN!\n");
}

START_CHUNKED_INTERFACE(Object_Fire, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	OVERRIDE_DEFAULT(CastShadows, "false")
	OVERRIDE_DEFAULT(RecieveShadows, "false")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bActive, true, Active);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obPosition, CPoint(0.0f, 0.0f, 0.0f), Position);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRadius, 1.0f, Radius);
	PUBLISH_VAR_AS(m_FireParticleFX, FireParticleFX);

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Object Fire State Machine
//!
//--------------------------------------------------
STATEMACHINE(OBJECT_FIRE_FSM, Object_Fire)
	OBJECT_FIRE_FSM(bool bActive)
	{
		if(bActive)
			SET_INITIAL_STATE(ACTIVE);
		else
			SET_INITIAL_STATE(INACTIVE);
	}

	STATE(ACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Enable particle effect
			}
			END_EVENT(true)

			ON_UPDATE
			{
				// Collision checks against arrow projectiles to ignite them
				ProjectileManager::Get().SendMessageToProjectilesWithinSphere(msg_ignite, ME->m_obPosition, ME->m_fRadius);
			}
			END_EVENT(true)

			EVENT(msg_deactivate)
			{
				SET_STATE(INACTIVE);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // ACTIVE

	STATE(INACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Disable particle effect

			}
			END_EVENT(true)

			EVENT(msg_activate)
				SET_STATE(ACTIVE);
			END_EVENT(true)
		END_EVENTS
	END_STATE // INACTIVE
END_STATEMACHINE


//--------------------------------------------------
//!
//!	Object_Fire::Object_Fire()
//!	Default constructor
//!
//--------------------------------------------------
Object_Fire::Object_Fire() :
	m_obParticleFXPosition( CONSTRUCT_CLEAR ),
		m_obPosition( CONSTRUCT_CLEAR ),
		m_fRadius( 0.4f )
{
	m_eType = EntType_Object;
}

//--------------------------------------------------
//!
//!	Object_Fire::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_Fire::OnPostConstruct()
{
	CEntity::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();
	// Don't need to install dynamics here if you're gonna use specific shared attributes through Lua below
	// All InstallDynamics does is look for a PS.XML or clump header info, this is also done in the Lua ones

	// Calculate where the particle system should be, based on the position and size of the sphere
	CPoint obDownVector(0.0f, -1.0f * m_fRadius, 0.0f);
	m_obParticleFXPosition = m_obPosition + obDownVector;

	// Create and attach the statemachine
	OBJECT_FIRE_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) OBJECT_FIRE_FSM(m_bActive);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_Fire::~Object_Fire()
//!	Default destructor
//!
//--------------------------------------------------
Object_Fire::~Object_Fire()
{

}
