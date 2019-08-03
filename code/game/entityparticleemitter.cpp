//--------------------------------------------------
//!
//!	\file game/entityparticleemitter.cpp
//!	Definition of the Particle Emitter object
//!
//--------------------------------------------------


#include "game/entityparticleemitter.h"

#include "objectdatabase/dataobject.h"
#include "game/message.h"
#include "effect/fxhelper.h"


void ForceLinkFunctionParticleEmitter()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionParticleEmitter() !ATTN!\n");
}


START_STD_INTERFACE(Object_ParticleEmitter)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	OVERRIDE_DEFAULT(CastShadows, "false")
	OVERRIDE_DEFAULT(RecieveShadows, "false")

	PUBLISH_VAR_AS(m_Description, Description)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bActive, true, Active);
	PUBLISH_VAR_AS(m_ParticleFX, ParticleFX);

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )	
END_STD_INTERFACE


//--------------------------------------------------
//!
//! Object Fire State Machine
//!
//--------------------------------------------------
STATEMACHINE(OBJECT_PARTICLEEMITTER_FSM, Object_ParticleEmitter)
	OBJECT_PARTICLEEMITTER_FSM(bool bActive)
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
				ME->m_uFXID = FXHelper::Pfx_CreateStaticMatrix( ME->m_ParticleFX, ME->m_PosMatrix );
			}
			END_EVENT(true)

			EVENT(Deactivate)
			{
				// Kill the particle effect
				FXHelper::Pfx_Destroy( ME->m_uFXID, true );

				SET_STATE(INACTIVE);
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // ACTIVE

	STATE(INACTIVE)
		BEGIN_EVENTS
			ON_ENTER
			{

			}
			END_EVENT(true)

			EVENT(Activate)
				SET_STATE(ACTIVE);
			END_EVENT(true)
		END_EVENTS
	END_STATE // INACTIVE
END_STATEMACHINE


//--------------------------------------------------
//!
//!	Object_ParticleEmitter::Object_ParticleEmitter()
//!	Default constructor
//!
//--------------------------------------------------
Object_ParticleEmitter::Object_ParticleEmitter()
{
	m_eType = EntType_Object;
	m_uFXID	= 0;
}

//--------------------------------------------------
//!
//!	Object_ParticleEmitter::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_ParticleEmitter::OnPostConstruct()
{
	CEntity::OnPostConstruct();
	
	// Create components
	InstallMessageHandler();

	// Setup the position matrix
	m_PosMatrix.SetIdentity();
	m_PosMatrix.SetTranslation( GetPosition() );

	// Create and attach the statemachine
	OBJECT_PARTICLEEMITTER_FSM* pFSM = NT_NEW OBJECT_PARTICLEEMITTER_FSM(m_bActive);
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	Object_ParticleEmitter::~Object_ParticleEmitter()
//!	Default destructor
//!
//--------------------------------------------------
Object_ParticleEmitter::~Object_ParticleEmitter()
{

}

