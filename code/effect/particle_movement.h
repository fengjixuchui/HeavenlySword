//--------------------------------------------------
//!
//!	\file particle_movement.h
//!	movement helper objects used by the particle 
//! handlers in CPU based particles
//!
//--------------------------------------------------

#ifndef _PARTICLE_MOVMENT_H
#define _PARTICLE_MOVMENT_H

#include "texture_function.h"
#include "effect/effect_resourceman.h"
#include "effect_util.h"
#include "particle_Structs.h"
#include "particle_spawning.h"
#include "physics/world.h"

//--------------------------------------------------
//!
//!	Particle_AdvancedMovement function ID
//!
//--------------------------------------------------
enum ADV_MOVE_FUNC_ID
{
	VELOCITY_SCALING		= 0,

	ROCKET_ACCELERATION_MIN	= 1,
	ROCKET_ACCELERATION_MAX = 2,
	
	WORLD_ACCELERATION_X	= 3,
	WORLD_ACCELERATION_Y	= 4,
	WORLD_ACCELERATION_Z	= 5,

	ROCKET_YAW				= 6,
	ROCKET_PITCH			= 7,
};

//--------------------------------------------------
//!
//!	ParticleAdvMoveResources
//!
//--------------------------------------------------
class ParticleAdvMoveResources : public EffectResource
{
public:
	ParticleAdvMoveResources();
	virtual ~ParticleAdvMoveResources();
	
	virtual void GenerateResources();
	virtual bool ResourcesOutOfDate() const;

	// these are all exposed to welder via the parent Particle_AdvancedMovement object
	int						m_iNumSamples;
	FunctionCurve_User*		m_pVelScale;

	FunctionCurve_User*		m_pRocketAccMin;
	FunctionCurve_User*		m_pRocketAccMax;

	FunctionCurve_User*		m_pAccelerationX;
	FunctionCurve_User*		m_pAccelerationY;
	FunctionCurve_User*		m_pAccelerationZ;

	FunctionCurve_User*		m_pYaw;
	FunctionCurve_User*		m_pPitch;

	const FunctionObject&	GetFunctions() const	{ return m_functions; }

private:
	FunctionObject			m_functions;		// auto gen'd size funcs
};

//--------------------------------------------------
//!
//!	Particle_AdvancedMovement
//! Definintion Structure for advanced CPU particle movement
//!
//--------------------------------------------------
class Particle_AdvancedMovement
{
public:
	virtual ~Particle_AdvancedMovement() {};
	virtual void PostConstruct();
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter );
	virtual void EditorChangeParent() { PostConstruct(); }

	ntstd::List<void*>	m_obObjects;	// storage for auto created thingumys

	bool	m_bUseVelocityScaling;
	bool	m_bUseRocketAcc;
	bool	m_bUseWorldAcc;
	bool	m_bUseSteering;
	ParticleAdvMoveResources m_resources;
};

//--------------------------------------------------
//!
//!	Iterative_ParticleMover
//!
//--------------------------------------------------
class Iterative_ParticleMover
{
public:
	Iterative_ParticleMover(	bool bUseRayCast,
								PlaneDef* pBouncePlane,
								float fElasticity,
								ParticleSpawner* pSpawner = NULL) :
		m_bUseBounce( false ),
		m_bUseRayCast( bUseRayCast ),
		m_fRestitution( fElasticity ),
		m_bouncePlane( CONSTRUCT_CLEAR ),
		m_pSpawner( pSpawner )
	{
		if (m_bUseRayCast)
		{
			m_bUseBounce = true;
		}
		else if (pBouncePlane)
		{
			m_bUseBounce = true;
			m_bouncePlane = pBouncePlane->GetPlane();
		}
	}

	inline void PreUpdate(	float fTimeDelta,
							float particleLerp,
							bool& bUp,
							const Particle_AdvancedMovement& movement,
							pos_vel_PD& particle );
	
	inline void Update( float fTimeDelta,
						pos_vel_PD& particle );

	CDirection	m_currAcc;

private:
	bool		m_bUseBounce;
	bool		m_bUseRayCast;
	float		m_fRestitution;
	CVector		m_bouncePlane;
	ParticleSpawner* m_pSpawner;
};

//--------------------------------------------------
//!
//!	Iterative_ParticleMover::PreUpdate
//! NB, could do with working out how to reverse these
//! Factors to get correct initial conditions
//!
//--------------------------------------------------
inline void Iterative_ParticleMover::PreUpdate(	float fTimeDelta,
												float particleLerp,
												bool& bUp,
												const Particle_AdvancedMovement& movement,
												pos_vel_PD& particle )
{
	const FunctionObject& moveFuncs	= movement.m_resources.GetFunctions();

	// hacky velocity scaling
	if (movement.m_bUseVelocityScaling)
	{
		float fScaleFactor = moveFuncs.Sample_linearFilter( particle.ageN, VELOCITY_SCALING );
		particle.vel += particle.vel * (fTimeDelta * (fScaleFactor - 1.0f));
	}

	// more accurate acceleration in direction we're facing
	if (movement.m_bUseRocketAcc)
	{
		float acc1 = moveFuncs.Sample_linearFilter( particle.ageN, ROCKET_ACCELERATION_MIN );
		float acc2 = moveFuncs.Sample_linearFilter( particle.ageN, ROCKET_ACCELERATION_MAX );
		float accSum = (acc1 * (1.0f - particleLerp)) + (acc2 * particleLerp); 

		CDirection newAcc( particle.vel );
		newAcc.Normalise();

		m_currAcc += newAcc * accSum;
	}

	// world based acceleration
	if (movement.m_bUseWorldAcc)
	{
		CDirection newAcc( 	moveFuncs.Sample_linearFilter( particle.ageN, WORLD_ACCELERATION_X ),
							moveFuncs.Sample_linearFilter( particle.ageN, WORLD_ACCELERATION_Y ),
							moveFuncs.Sample_linearFilter( particle.ageN, WORLD_ACCELERATION_Z ) );

		m_currAcc += newAcc;
	}

	// steer in yaw and pitch
	if (movement.m_bUseSteering)
	{
		float fVelLength = particle.vel.Length();
		particle.vel.Normalise();

		float fPitch = -atan2f( particle.vel.Y(), sqrt( particle.vel.X()*particle.vel.X() + particle.vel.Z()*particle.vel.Z() ) );
		float fYaw = atan2f( particle.vel.X(),particle.vel.Z() );

		float fPitchOld = fPitch;
		float fYawOffset = moveFuncs.Sample_linearFilter( particle.ageN, ROCKET_YAW ) * DEG_TO_RAD_VALUE * fTimeDelta;
		float fPitchOffset = moveFuncs.Sample_linearFilter( particle.ageN, ROCKET_PITCH ) * DEG_TO_RAD_VALUE * fTimeDelta;

		if (bUp)
		{
			fPitch += fPitchOffset;
			fYaw += fYawOffset;
		}
		else
		{
			fPitch -= fPitchOffset;
			fYaw -= fYawOffset;
		}

		if	(
			((fPitchOld < HALF_PI) && (fPitch >= HALF_PI)) ||
			((fPitchOld > -HALF_PI) && (fPitch <= -HALF_PI))
			)
			bUp = !bUp;

		float SX, CX, SY, CY;
		CMaths::SinCos( fPitch + HALF_PI, SX, CX );
		CMaths::SinCos( fYaw, SY, CY );

		particle.vel = CDirection( SX*SY, CX, SX*CY ) * fVelLength;
	}
}

//--------------------------------------------------
//!
//!	Iterative_ParticleMover::Update
//!
//--------------------------------------------------
inline void Iterative_ParticleMover::Update(	float fTimeDelta,
												pos_vel_PD& particle )
{
	// AHEM! the order of update is important here, pos must be updated
	// before vel for 'correct' (re:smooth) bounce continuity under acceleration
	if (m_bUseBounce)
	{
		CPoint nextPos( particle.pos + (particle.vel * fTimeDelta) );

		// check for bounce
		float fHitFraction;
		CDirection normal;
		bool bCollided = false;

		union Physics::RaycastCollisionFlag obFlag;
		obFlag.base = 0;

		// [Mus] - What settings for this cast ?
		obFlag.flags.i_am = Physics::COLLISION_ENVIRONMENT_BIT;
		obFlag.flags.i_collide_with = (	Physics::LARGE_INTERACTABLE_BIT	);

		if	(
			(m_bUseRayCast) && // do we want full on havok collision?
			(Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( particle.pos, nextPos, fHitFraction, normal, obFlag ))
			)
		{
			bCollided = true;
		}
		else 
		if ( PlaneDef::Intersects( m_bouncePlane, particle.pos, nextPos, fHitFraction ) )
		{
			normal = CDirection( m_bouncePlane );
			bCollided = true;
		}

		if (bCollided)
		{
			CPoint intersect;
			PlaneDef::BallisticReflect( particle.pos, intersect, particle.vel, m_currAcc, normal, fTimeDelta, fHitFraction, m_fRestitution );

			if (m_pSpawner)
				m_pSpawner->BounceNotification( &particle, intersect, normal );	
		}
		else
		{
			particle.pos = nextPos;
			particle.vel += m_currAcc * fTimeDelta;
		}
	}
	else
	{
		particle.pos += particle.vel * fTimeDelta;
		particle.vel += m_currAcc * fTimeDelta;
	}
};

#endif // _PARTICLE_MOVMENT_H
