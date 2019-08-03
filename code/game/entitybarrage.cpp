//--------------------------------------------------
//!
//!	\file game/entitybarrage.cpp
//!	Definition of the Barrage entity object
//!
//--------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "game/randmanager.h"
#include "messagehandler.h"
#include "game/entityprojectile.h"
#include "game/entitycatapultrock.h"

#include "game/entitybarrage.h"


void ForceLinkFunctionBarrage()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionBarrage() !ATTN!\n");
}


//------------------------------------------------------------------------------------------
// Object_Barrage XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(Object_Barrage, Mem::MC_ENTITY)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	PUBLISH_VAR_AS( m_Description, Description )

	PUBLISH_VAR_AS( m_ProjectileDef, ProjectileDef )
	PUBLISH_VAR_AS( m_EntityNameToFire, EntityNameToFire )
	PUBLISH_PTR_AS( m_pTargetEntity, TargetEntityToTrack )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obTargetPosition, CPoint(0.0f, 0.0f, 0.0f), TargetPosition )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fTargetRadius, 1.0f, TargetRadius )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_nNumShotsToFire, 1, NumShotsToFire )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fMinShootInterval, 0.5f, MinShootInterval )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fMaxShootInterval, 1.0f, MaxShootInterval )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bCanBeReShot, false, CanBeReShot )

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Object_Barrage Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START_INHERITED(Object_Barrage, CEntity)
	LUA_EXPOSED_METHOD(SetTarget, SetTarget, "Set the Target for the barrage", "entity Target", "")
LUA_EXPOSED_END(Object_Barrage)


//--------------------------------------------------
//!
//! Barrage Object State Machine
//!
//--------------------------------------------------
STATEMACHINE(OBJECT_BARRAGE_FSM, Object_Barrage)
	OBJECT_BARRAGE_FSM()
	{
		SET_INITIAL_STATE(WAIT_FOR_ACTIVATION);
	}

	STATE(WAIT_FOR_ACTIVATION)
		BEGIN_EVENTS
			ON_ENTER
			{
				ME->m_bCanFire = false;
			}
			END_EVENT(true)

			EVENT(Activate)
				SET_STATE(FIRE);
			END_EVENT(true)

			EVENT(msg_barrage_setradius)
			{
				float fNewRadius = msg.GetFloat("Radius");
				ntPrintf("msg_barrage_setradius recieved on entity %s, setting new radius to %f\n", ME->GetName().c_str(), fNewRadius);
				if(fNewRadius < 0.0f)
				{
					fNewRadius = -fNewRadius;
				}

				ME->m_fTargetRadius = fNewRadius;
			}
			END_EVENT(true)
		END_EVENTS
	END_STATE // WAIT_FOR_ACTIVATION

	STATE(FIRE)
		BEGIN_EVENTS
			ON_ENTER
			{
				// Reset fire count for this round of firing
				ME->m_nCurrentFiredNum	= 0;

				// We are allowed to fire projectiles
				ME->m_bCanFire = true;

				// Fire first shot
				ME->FireBarrageShot();

				// Process Logic
				ME->ProcessLogic();
			}
			END_EVENT(true)

			EVENT(msg_fire_next_shot)
			{
				// Fire next shot
				ME->FireBarrageShot();

				// Process Logic
				ME->ProcessLogic();
			}
			END_EVENT(true)

			EVENT(msg_allow_reuse)
			{
				// Allow the barrage to be reused by putting it into its 'wait for activation' state
				SET_STATE(WAIT_FOR_ACTIVATION);
			}
			END_EVENT(true)

			EVENT(Deactivate)
			{
				// Stop firing
				ME->m_bCanFire = false;

				// Should only be called by infinite firing barrages
				if ( ME->m_bCanBeReShot )
				{
					// Allow more firing, but only after the minimum interval
					Message ReUseMessage(msg_allow_reuse);
					ME->GetMessageHandler()->QueueMessageDelayed(ReUseMessage, ME->m_fMinShootInterval);
				}
				else
				{
					SET_STATE(USED);
				}
			}
			END_EVENT(true)

			EVENT(msg_barrage_setradius)
			{
				float fNewRadius = msg.GetFloat("Radius");
				ntPrintf("msg_barrage_setradius recieved on entity %s, setting new radius to %f\n", ME->GetName().c_str(), fNewRadius);
				if(fNewRadius < 0.0f)
				{
					fNewRadius = -fNewRadius;
				}

				ME->m_fTargetRadius = fNewRadius;
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE // FIRE

	STATE(USED)
		BEGIN_EVENTS
			ON_ENTER
				// Do nothing.  This barrage can't be used again.
			END_EVENT(true)
		END_EVENTS
	END_STATE // USED

END_STATEMACHINE // OBJECT_BARRAGE_FSM


//--------------------------------------------------
//!
//!	Object_Barrage::Object_Barrage()
//!	Default constructor
//!
//--------------------------------------------------
Object_Barrage::Object_Barrage()
{
	m_eType = EntType_Object;
	m_eBarrageType = BARRAGE_TYPE_UNKNOWN;

	// Set Defaults
	m_nCurrentFiredNum	= 0;
	m_pTargetEntity		= 0;
	m_fTargetRadius		= 1.0f;
	m_nNumShotsToFire	= 1;
	m_fMinShootInterval	= 0.5f;
	m_fMaxShootInterval	= 1.0f;
	m_bCanBeReShot		= false;
}

//--------------------------------------------------
//!
//!	Object_Barrage::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void Object_Barrage::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	InstallMessageHandler();

	// Check XML data to determine if we fire projectiles or entities
	if ( !ntStr::IsNull( m_ProjectileDef ) )
	{
		// This barrage fires projectiles
		m_eBarrageType = BARRAGE_TYPE_PROJECTILE;

		// Cache pointer to the projectile attributes
		m_pProjAttrs = ObjectDatabase::Get().GetPointerFromName<Projectile_Attributes*>(m_ProjectileDef);
		ntAssert(m_pProjAttrs);
	}
	else if ( !ntStr::IsNull( m_EntityNameToFire ) )
	{
		// This barrage fires entities
		m_eBarrageType = BARRAGE_TYPE_ENTITY;
	}
	else
	{
		// Abort!  Abort!
		ntError( false ); 
		m_eBarrageType = BARRAGE_TYPE_UNKNOWN;
	}

	// Create and attach the statemachine
	OBJECT_BARRAGE_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) OBJECT_BARRAGE_FSM();
	ATTACH_FSM(pFSM);
	ATTACH_LUA_INTERFACE(Object_Barrage);
}

//--------------------------------------------------
//!
//!	Object_Barrage::~Object_Barrage()
//!	Default destructor
//!
//--------------------------------------------------
Object_Barrage::~Object_Barrage()
{
}


//--------------------------------------------------
//!
//!	Object_Barrage::FireBarrageShot()
//!	Fires a barrage shot
//!
//--------------------------------------------------
void Object_Barrage::FireBarrageShot( void )
{
	// Only fire a shot if we haven't exceeded the limit or we have infinite shots
	if ( ( m_nCurrentFiredNum >= m_nNumShotsToFire && m_nNumShotsToFire >= 1 ) || !m_bCanFire)
	{
		return;
	}

	CPoint obWorldOriginPos = GetPosition();
	CPoint obWorldTargetPos;

	// TARGET TRACKED
	if ( m_pTargetEntity )
	{
		//Spawn the projectile, but calculate a world-space offset for the projectile target within a semi-circle
		//(oriented between the player's location and the location of the barrage object).
		CDirection Line2Enemy = CDirection(GetPosition() - m_pTargetEntity->GetPosition());
		Line2Enemy.Y() = 0;
		Line2Enemy.Normalise();
		float fRandAngle = grandf(1.0f) > 0.5f ? grandf(HALF_PI) : -grandf(HALF_PI);
		float fSin, fCos;
		CMaths::SinCos(fRandAngle, fSin, fCos);
		CDirection Rotation = CDirection(Line2Enemy.X()*fCos - Line2Enemy.Z()*fSin, Line2Enemy.Y(), Line2Enemy.X()*fSin + Line2Enemy.Z()*fCos);
		CDirection obWorldTargetOffset = CDirection(m_fTargetRadius * Rotation) + CDirection(0.0f, 1.5f, 0.0f);
		obWorldTargetPos = m_pTargetEntity->GetPosition() + CPoint(obWorldTargetOffset);
	}
	// TARGET SPHERE
	else
	{
		// Generate a random target position based on the radius from the target position
		obWorldTargetPos = m_obTargetPosition;

		// Only go through the random position generation if we need to
		if ( m_fTargetRadius > 0.0f )
		{
			CDirection obOffsetVec(CONSTRUCT_CLEAR);
			float fRanX;
			float fRanY;
			float fRanZ;
			
			do
			{
				fRanX = grandf(2.0f) - 1.0f;
				fRanY = grandf(2.0f) - 1.0f;
				fRanZ = grandf(2.0f) - 1.0f;

				obOffsetVec.X() = fRanX;
				obOffsetVec.Y() = fRanY;
				obOffsetVec.Z() = fRanZ;
			}
			while( obOffsetVec.LengthSquared() < 0.01f );

			obOffsetVec.Normalise();
			obOffsetVec *= m_fTargetRadius;

			obWorldTargetPos += obOffsetVec;
		}
	}

	if ( m_eBarrageType == BARRAGE_TYPE_PROJECTILE )
	{
		// Spawn the projectile
		Object_Projectile* pProjectile = Object_Projectile::SpawnAimedProjectile( m_pProjAttrs, GetPosition(), obWorldTargetPos, true);
		UNUSED(pProjectile);	
	}
	else if ( m_eBarrageType == BARRAGE_TYPE_ENTITY )
	{
		if ( m_EntityNameToFire == CHashedString("CatapultRock") )
		{
			CEntity* pNewRock = Object_Catapult_Rock::ConstructBarrageCatapultRockObject( GetMappedAreaInfo(), obWorldOriginPos, obWorldTargetPos );
			UNUSED( pNewRock );
		}
	}

	// Increment number of shots fired
	m_nCurrentFiredNum++;
}


//--------------------------------------------------
//!
//!	Object_Barrage::FireBarrageShot()
//!	Decide what to do next, saves duplicating code
//!
//--------------------------------------------------
void Object_Barrage::ProcessLogic( void )
{
	// Do we finish now or fire another shot?
	if (m_nCurrentFiredNum < m_nNumShotsToFire || m_nNumShotsToFire < 1)
	{
		// Fire another shot
		float fIntervalDiff = m_fMaxShootInterval -  m_fMinShootInterval;
		float fInterval = m_fMinShootInterval + grandf(fIntervalDiff);
		
		Message FireNextMessage(msg_fire_next_shot);
		m_pobMessageHandler->QueueMessageDelayed(FireNextMessage, fInterval);
	}
	else
	{
		m_bCanFire = false;

		// Fire no more shots, can this barrage be reshot?
		if ( m_bCanBeReShot )
		{
			// Allow more firing, but only after the minimum interval
			Message ReUseMessage(msg_allow_reuse);
			m_pobMessageHandler->QueueMessageDelayed(ReUseMessage, m_fMinShootInterval);
		}
		else
		{
			// No more firing
			EXTERNALLY_SET_STATE(OBJECT_BARRAGE_FSM, USED);
		}
	}
}
